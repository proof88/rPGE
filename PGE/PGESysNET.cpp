/*
    ###################################################################################
    PGESysNET.cpp
    This file is part of PGE.
    PR00F's Game Engine networking subsystem
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PRREbaseIncludes.h"  // PCH

#include <cassert>

#include "PGESysNET.h"
#include "PGEincludes.h"
#include "PGEpragmas.h"


/*
   PGESysNET
   ###########################################################################
*/


// ############################### PUBLIC ################################

const uint16 PGESysNET::DEFAULT_SERVER_PORT;

const uint32_t PgePktUserConnected::id;
const uint32_t PgePktUserCmdMove::id;

bool PGESysNET::isServer()
{
    return bServer;
}

PGESysNET::PGESysNET() :
    nPort(DEFAULT_SERVER_PORT),
    m_pInterface(nullptr),
    m_hListenSock(k_HSteamListenSocket_Invalid),
    m_hPollGroup(k_HSteamNetPollGroup_Invalid),
    m_hConnection(k_HSteamNetConnection_Invalid)
{
    addrServer.Clear();
} // PGESysNET()


PGESysNET::~PGESysNET()
{

} // ~PGESysNET()


// TODO: move this down to private area later ...
static void NetworkDbg(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        CConsole::getConsoleInstance("PGESysNET").EOLn("%s: %s", __func__, pszMsg);
    }
    else
    {
        CConsole::getConsoleInstance("PGESysNET").OLn("%s: %s", __func__, pszMsg);
    }
}


bool PGESysNET::initSysNET(void)
{
    SteamDatagramErrMsg errMsg;

    if ( GameNetworkingSockets_Init(nullptr, errMsg) )
    {
        CConsole::getConsoleInstance("PGESysNET").SOLn("Initialized GameNetworkingSockets %s!", GAMENETWORKINGSOCKETS_VER_STR);
    }
    else
    {
        CConsole::getConsoleInstance("PGESysNET").EOLn("Failed to initialize GameNetworkingSockets %s: %s!", GAMENETWORKINGSOCKETS_VER_STR, errMsg);
        return false;
    }

    addrServer.Clear();
    if (MessageBox(0, "Szerver?", ":)", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES)
    {
        bServer = true;
    }
    else
    {
        bServer = false;
        const std::string sServerAddr = "127.0.0.1";
        if ( !addrServer.ParseString(sServerAddr.c_str()) )
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("Failed to parse addrServer: %s!", sServerAddr.c_str());
            destroySysNET();
            return false;
        }
        if (addrServer.m_port == 0)
        {
            addrServer.m_port = DEFAULT_SERVER_PORT;
            CConsole::getConsoleInstance("PGESysNET").OLn("Using default port: %d", addrServer.m_port);
        }
        else
        {
            CConsole::getConsoleInstance("PGESysNET").OLn("Using custom port: %d", addrServer.m_port);
        }
    }

    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, NetworkDbg);

    // Select instance to use. For now we'll always use the default.
    // But we could use SteamGameServerNetworkingSockets() on Steam.
    m_pInterface = SteamNetworkingSockets();

    if (isServer())
    {
        // Start listening
        SteamNetworkingIPAddr serverLocalAddr;
        serverLocalAddr.Clear();
        serverLocalAddr.m_port = nPort;
        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
        m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
        if (m_hListenSock == k_HSteamListenSocket_Invalid)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("Failed to listen on port %d", nPort);
            destroySysNET();
            return false;
        }

        m_hPollGroup = m_pInterface->CreatePollGroup();
        if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("Failed to listen on port %d", nPort);
            destroySysNET();
            return false;
        }
        CConsole::getConsoleInstance("PGESysNET").OLn("Server listening on port %d", nPort);
    }
    else
    {
        // ConnectClient() can be invoked later, now temporarily by public PGE::ConnectClient()    
    }

    // now main engine loop can invoke PollIncomingMessages() and PollConnectionStateChanges()

    return true;
} // initSysNET()


bool PGESysNET::destroySysNET(void)
{
    if (isServer())
    {
        // Close all the connections
        CConsole::getConsoleInstance("PGESysNET").OLn("Server closing connections...");
        for (auto it : m_mapClients)
        {
            // Send them one more goodbye message.  Note that we also have the
            // connection close reason as a place to send final data.  However,
            // that's usually best left for more diagnostic/debug text not actual
            // protocol strings.
            //SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

            // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
            // to flush this out and close gracefully.
            m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
        }
        m_mapClients.clear();

        m_pInterface->CloseListenSocket(m_hListenSock);
        m_hListenSock = k_HSteamListenSocket_Invalid;

        m_pInterface->DestroyPollGroup(m_hPollGroup);
        m_hPollGroup = k_HSteamNetPollGroup_Invalid;
    }
    m_hConnection = k_HSteamNetConnection_Invalid;
    m_pInterface = nullptr;
    addrServer.Clear();
    GameNetworkingSockets_Kill();
    return true;
} // destroySysNET()

bool PGESysNET::PollIncomingMessages(PgePacket& pkt)
{
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    if (isServer())
    {
        int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
        if (numMsgs == 0)
        {
            return false;
        }

        if (numMsgs < 0)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER Error checking for messages!", __func__);
            return false;
        }

        if (numMsgs != 1)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER numMsgs == %d!", __func__, numMsgs);
            return false;
        }

        if (!pIncomingMsg)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER pIncomingMsg null!", __func__);
            return false;
        }
        
        auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
        if (itClient == m_mapClients.end())
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER failed to find in m_mapClients!", __func__);
            return false;
        }

        // '\0'-terminate it to make it easier to parse
        //std::string sCmd;
        //sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
        //const char* cmd = sCmd.c_str();

        // We don't need this anymore.
        pIncomingMsg->Release();

        // TODO: process message: cmd

        return false; // TODO: change this to true when we are finally copying stuff to pkt!
    }
    else
    {   // client
        if (m_hConnection == k_HSteamNetConnection_Invalid)
        {
            return false;
        }

        const int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
        
        // ### from here same as server code above
        if (numMsgs == 0)
        {
            return false;
        }

        if (numMsgs < 0)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT Error checking for messages!", __func__);
            return false;
        }

        if (numMsgs != 1)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT numMsgs == %d!", __func__, numMsgs);
            return false;
        }

        if (!pIncomingMsg)
        {
            CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT pIncomingMsg null!", __func__);
            return false;
        }
        // ### until here

        assert(pIncomingMsg->m_cbSize == sizeof(pkt));
        memcpy(&pkt, pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);

        // Just echo anything we get from the server
        //fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
        //fputc('\n', stdout);

        // We don't need this anymore.
        pIncomingMsg->Release();

        return true;
    }
}

void PGESysNET::PollConnectionStateChanges()
{
    s_pCallbackInstance = this;
    m_pInterface->RunCallbacks();
}

// ### from here server only
void PGESysNET::SendStringToClient(HSteamNetConnection conn, const char* str)
{
    m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void PGESysNET::SendPacketToClient(HSteamNetConnection conn, const PgePacket& pkt)
{
    m_pInterface->SendMessageToConnection(conn, &pkt, (uint32)sizeof(pkt), k_nSteamNetworkingSend_Reliable, nullptr);
}

void PGESysNET::SendStringToAllClients(const char* str, HSteamNetConnection except)
{
    for (auto& c : m_mapClients)
    {
        if (c.first != except)
            SendStringToClient(c.first, str);
    }
}

void PGESysNET::SendPacketToAllClients(const PgePacket& pkt, HSteamNetConnection except)
{
    for (auto& c : m_mapClients)
    {
        if (c.first != except)
            SendPacketToClient(c.first, pkt);
    }
}
// ### server only until here


// ### from here client only
bool PGESysNET::ConnectClient()
{
    // Start connecting
    char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
    addrServer.ToString(szAddr, sizeof(szAddr), true);
    CConsole::getConsoleInstance("PGESysNET").OLn("Connecting to server at %s", szAddr);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

    m_hConnection = m_pInterface->ConnectByIPAddress(addrServer, 1, &opt);
    if (m_hConnection == k_HSteamNetConnection_Invalid)
    {
        CConsole::getConsoleInstance("PGESysNET").EOLn("Client Failed to create connection");
        destroySysNET();
        return false;
    }

    return true;
}
// ### client only until here


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


PGESysNET* PGESysNET::s_pCallbackInstance = nullptr;
bool PGESysNET::bServer = false;

void PGESysNET::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    // This function is also invoked on main thread when I call PGESysNET.PollConnectionStateChanges() from PGE::runGame()
    // so no need to utilize mutexes around here.
    // And the other function PollIncomingMessages() is also invoked by PGE::runGame().
    // So it is safe to do operations on queuePackets.
    // The problem related to CConsole not being threadsafe was not due to the logs within this function
    // but to the callback function I've set with SteamNetworkingUtils()->SetDebugOutputFunction(), as it can be called
    // in parallel by SteamNetworking thread.

    if (isServer())
    {
        char temp[1024];

        // What's the state of the connection?
        switch (pInfo->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Ignore if they were not previously connected.  (If they disconnected
            // before we accepted the connection.)
            if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {

                // Locate the client.  Note that it should have been found, because this
                // is the only codepath where we remove clients (except on shutdown),
                // and connection change callbacks are dispatched in queue order.
                auto itClient = m_mapClients.find(pInfo->m_hConn);
                assert(itClient != m_mapClients.end());

                // Select appropriate log messages
                const char* pszDebugLogAction;
                if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
                {
                    pszDebugLogAction = "problem detected locally";
                    sprintf(temp, "%s disappeared (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
                }
                else
                {
                    // Note that here we could check the reason code to see if
                    // it was a "usual" connection or an "unusual" one.
                    pszDebugLogAction = "closed by peer";
                    sprintf(temp, "%s closed connection", itClient->second.m_sNick.c_str());
                }

                // Spew something to our own log.  Note that because we put their nick
                // as the connection description, it will show up, along with their
                // transport-specific data (e.g. their IP address)
                CConsole::getConsoleInstance("PGESysNET").OLn("%s: SERVER Connection %s %s, reason %d: %s\n",
                    __func__,
                    pInfo->m_info.m_szConnectionDescription,
                    pszDebugLogAction,
                    pInfo->m_info.m_eEndReason,
                    pInfo->m_info.m_szEndDebug
                );

                m_mapClients.erase(itClient);

                // Send a message so everybody else knows what happened
                //SendStringToAllClients(temp);
            }
            else
            {
                assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
            }

            // Clean up the connection.  This is important!
            // The connection is "closed" in the network sense, but
            // it has not been destroyed.  We must close it on our end, too
            // to finish up.  The reason information do not matter in this case,
            // and we cannot linger because it's already closed on the other end,
            // so we just pass 0's.
            m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            // This must be a new connection
            assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

            CConsole::getConsoleInstance("PGESysNET").OLn("%s: SERVER Connection request from %s", __func__, pInfo->m_info.m_szConnectionDescription);

            // A client is attempting to connect
            // Try to accept the connection.
            if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
            {
                // This could fail.  If the remote host tried to connect, but then
                // disconnected, the connection may already be half closed.  Just
                // destroy whatever we have on our side.
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER Can't accept connection. (It was already closed?)", __func__);
                break;
            }

            // Assign the poll group
            if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
            {
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                CConsole::getConsoleInstance("PGESysNET").EOLn("%s: SERVER Failed to set poll group?", __func__);
                break;
            }

            // Generate a random nick.  A random temporary nick
            // is really dumb and not how you would write a real chat server.
            // You would want them to have some sort of signon message,
            // and you would keep their client in a state of limbo (connected,
            // but not logged on) until them.  I'm trying to keep this example
            // code really simple.
            PgePacket pkt;
            memset(&pkt, 0, sizeof(pkt));
            pkt.pktId = PgePktUserConnected::id;
            pkt.msg.userConnected.bCurrentClient = true;
            
            // generate unique user name
            bool found = false;
            do
            {
                sprintf(pkt.msg.userConnected.sUserName, "User%d", 10000 + (rand() % 100000));
                for (const auto& client : m_mapClients)
                {
                    found = (client.second.m_sNick == pkt.msg.userConnected.sUserName);
                    if (found)
                    {
                        break;
                    }
                }
            } while ( found );


            // Send them a welcome message
            //sprintf(temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick);
            //SendStringToClient(pInfo->m_hConn, temp);

            

            // Also send them a list of everybody who is already connected
            //if (m_mapClients.empty())
            //{
            //    SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
            //}
            //else
            //{
            //    sprintf(temp, "%d companions greet you:", (int)m_mapClients.size());
            //    for (auto& c : m_mapClients)
            //        SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
            //}

            // Let everybody else know who they are for now
            //sprintf(temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick);
            //SendStringToAllClients(temp, pInfo->m_hConn);
            

            // Add them to the client list, using std::map wacky syntax
            m_mapClients[pInfo->m_hConn];
            SetClientNick(pInfo->m_hConn, pkt.msg.userConnected.sUserName);

            // we push this packet to our pkt queue, this is how we "send" message to ourselves so server game loop can process it
            queuePackets.push_back(pkt);

            SendPacketToAllClients(pkt);

            CConsole::getConsoleInstance("PGESysNET").OLn("%s: SERVER A client with name %s is connecting ...", __func__, pkt.msg.userConnected.sUserName);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            // We will get a callback immediately after accepting the connection.
            // Since we are the server, we can ignore this, it's not news to us.
            CConsole::getConsoleInstance("PGESysNET").OLn("%s: SERVER A client has connected!", __func__);
            break;

        default:
            // Silences -Wswitch
            break;
        }
    }
    else
    {
        assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

        // What's the state of the connection?
        switch (pInfo->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Print an appropriate message
            if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
            {
                // Note: we could distinguish between a timeout, a rejected connection,
                // or some other transport problem.
                CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT problem 1 (rejected I guess) (%s)",
                    __func__, pInfo->m_info.m_szEndDebug);
            }
            else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT problem 2 (guess our machine has lost network connection) (%s)",
                    __func__, pInfo->m_info.m_szEndDebug);
            }
            else
            {
                // NOTE: We could check the reason code for a normal disconnection
                CConsole::getConsoleInstance("PGESysNET").EOLn("%s: CLIENT problem 3 (closed by peer) (%s)",
                    __func__, pInfo->m_info.m_szEndDebug);
            }

            // Clean up the connection.  This is important!
            // The connection is "closed" in the network sense, but
            // it has not been destroyed.  We must close it on our end, too
            // to finish up.  The reason information do not matter in this case,
            // and we cannot linger because it's already closed on the other end,
            // so we just pass 0's.
            m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            m_hConnection = k_HSteamNetConnection_Invalid;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            // We will get this callback when we start connecting.
            // We can ignore this.
            CConsole::getConsoleInstance("PGESysNET").OLn("%s: CLIENT Connecting ...", __func__);
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            CConsole::getConsoleInstance("PGESysNET").OLn("%s: CLIENT Connected to server OK", __func__);
            break;

        default:
            // Silences -Wswitch
            break;
        }
    }
}

void PGESysNET::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

PGESysNET::PGESysNET(const PGESysNET&)
{

}

PGESysNET& PGESysNET::operator=(const PGESysNET&)
{
    return *this;
}

void PGESysNET::SetClientNick(HSteamNetConnection hConn, const char* nick)
{
    // Remember their nick
    m_mapClients[hConn].m_sNick = nick;

    // Set the connection name, too, which is useful for debugging
    m_pInterface->SetConnectionName(hConn, nick);
}