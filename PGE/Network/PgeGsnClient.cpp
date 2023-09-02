/*
    ###################################################################################
    PgeGsnClient.cpp
    This file is part of PGE.
    Client Wrapper for GameNetworkingSockets library
    PR00F's Game Engine networking subsystem
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PureBaseIncludes.h"  // PCH

#include <cassert>

#include "PgeGsnClient.h"
#include "../PGEincludes.h"
#include "../PGEpragmas.h"


/*
   PgeGsnClient
   ###########################################################################
*/


// ############################### PUBLIC ################################


/**
    Creates and gets the singleton instance.
*/
PgeGsnClient& PgeGsnClient::createAndGet(PGEcfgProfiles& cfgProfiles)
{
    static PgeGsnClient inst(cfgProfiles);
    return inst;
} // createAndGet()

PgeGsnClient::~PgeGsnClient()
{
    destroy();
} // ~PgeGsnClient()

bool PgeGsnClient::destroy()
{
    if (!isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s() Already disconnected.", __func__);
        return true;
    }
    CConsole::getConsoleInstance("PgeGsnClient").OLn("Detailed Connection Status:");
    CConsole::getConsoleInstance("PgeGsnClient").OLn("%s", getDetailedStatus().c_str());
    
    return disconnectClient() && PgeGsnWrapper::destroy();
} // destroy()

bool PgeGsnClient::isInitialized() const
{
    return m_hConnection != k_HSteamNetConnection_Invalid;
} // isInitialized()

bool PgeGsnClient::connectToServer(const std::string& sServerAddress)
{
    if (isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s ERROR: already connected to %s!", __func__, m_szAddr);
        return false;
    }

    m_addrServer.Clear();
    if (!m_addrServer.ParseString(sServerAddress.c_str()))
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s() Failed to parse address: %s!", __func__, sServerAddress.c_str());
        return false;
    }

    if (m_addrServer.m_port == 0)
    {
        m_addrServer.m_port = DEFAULT_SERVER_PORT;
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s() Using default port: %d", __func__, m_addrServer.m_port);
    }
    else
    {
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s() Using custom port: %d", __func__, m_addrServer.m_port);
    }

    // Parse back to string just to make sure we are logging and connecting to proper address
    m_addrServer.ToString(m_szAddr, sizeof(m_szAddr), true);
    CConsole::getConsoleInstance("PgeGsnClient").OLn("%s Connecting to server at %s", __func__, m_szAddr);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)steamNetConnectionStatusChangedCallback);

    m_hConnection = m_pInterface->ConnectByIPAddress(m_addrServer, 1, &opt);
    if (m_hConnection == k_HSteamNetConnection_Invalid)
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("Client Failed to create connection");
        return false;
    }

    //SteamNetConnectionInfo_t connInfo;
    //m_pInterface->GetConnectionInfo(m_hConnection, &connInfo);

    return true;
}

bool PgeGsnClient::disconnectClient()
{
    if (!isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s not connected.", __func__);
        return true;
    }

    if (m_hConnection != k_HSteamNetConnection_Invalid)
    {
        m_pInterface->CloseConnection(m_hConnection, k_ESteamNetConnectionEnd_App_Generic, "PgeGsnServer Client Graceful shutdown", true);
        m_hConnection = k_HSteamNetConnection_Invalid;
    }
    m_hConnectionServerSide = k_HSteamNetConnection_Invalid;
    m_addrServer.Clear();
    memset(m_szAddr, sizeof(m_szAddr), 0);
    memset(&m_connRtStatus, 0, sizeof(m_connRtStatus));

    return true;
}

const HSteamNetConnection& PgeGsnClient::getConnectionHandle() const
{
    return m_hConnection;
}

const HSteamNetConnection& PgeGsnClient::getConnectionHandleServerSide() const
{
    return m_hConnectionServerSide;
}

const char* PgeGsnClient::getServerAddress() const
{
    return m_szAddr;
}

void PgeGsnClient::sendToServer(const pge_network::PgePacket& pkt)
{
    if (!isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s not connected!", __func__);
        return;
    }

    uint32_t nActualPktSize;
    if (pge_network::PgePacket::getPacketId(pkt) == pge_network::PgePktId::APP)
    {
        // We need the real used memory size so we can truncate the sent pkt to that.
        // We need to consider member padding and the actual message sizes to have a correct value.
        // 'cData' is our point of view since from there we need to calculate.
        const pge_network::MsgApp* pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
        const uint8_t* pMsgAppInByteSteps = pge_network::PgePacket::getMessageAppArea(pkt).cData;  // we can step this ptr in Bytes
        // Real offset in Bytes in memory of actual app data relative to beginning of MsgApp struct
        // const size_t nByteDistanceOfMsgDataInMsgApp = (uint8_t*)(&(pMsgApp->cMsgData)) - (uint8_t*)(pMsgApp);
        const size_t nByteDistanceOfMsgDataInMsgApp = offsetof(pge_network::MsgApp, cMsgData);
        for (uint8_t i = 0; i < pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount; i++)
        {
            pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pMsgAppInByteSteps);
            // moving pMsgAppInByteSteps by the _actual_ size of the current MsgApp struct (considering the actual app message size there)
            pMsgAppInByteSteps += (nByteDistanceOfMsgDataInMsgApp + pMsgApp->nMsgSize);
        }
        // now pMsgAppInByteSteps points to RIGHT AFTER the 1st byte of the last application message
        const uint8_t* pPkt = (const uint8_t*)(&pkt);
        nActualPktSize = pMsgAppInByteSteps - pPkt;
    }
    else
    {
        nActualPktSize = static_cast<uint32_t>(sizeof(pkt));
    }

    m_pInterface->SendMessageToConnection(m_hConnection, &pkt, nActualPktSize, k_nSteamNetworkingSend_Reliable, nullptr);
    if (m_nTxPktCount == 1)
    {
        m_time1stTxPkt = std::chrono::steady_clock::now();
    }
    m_nTxPktCount++;
    if (pge_network::PgePacket::getPacketId(pkt) == pge_network::PgePktId::APP)
    {
        const pge_network::MsgApp* pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
        for (uint8_t i = 0; i < pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount; i++)
        {
            // TODO: nooo, I need to properly iterate to next appmsg as I iterate in above loop!
            ++m_nTxMsgCount[pMsgApp->msgId];
        }
    }
    m_nTxByteCount += nActualPktSize;
}

const SteamNetConnectionRealTimeStatus_t& PgeGsnClient::getRealTimeStatus(bool bForceUpdate)
{
    if (!isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s not connected!", __func__);
        assert(false);
        return m_connRtStatus;
    }

    if (bForceUpdate)
    {
        m_pInterface->GetConnectionRealTimeStatus(m_hConnection, &m_connRtStatus, 0, NULL);
    }

    return m_connRtStatus;
}

std::string PgeGsnClient::getDetailedStatus() const
{
    if (!isInitialized())
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s not connected!", __func__);
        assert(false);
        return "";
    }

    char szDetailedStatus[4096];
    const int nRes = m_pInterface->GetDetailedConnectionStatus(m_hConnection, szDetailedStatus, sizeof(szDetailedStatus));
    if (nRes == 0)
    {
        // only in this case szDetailedStatus is null-terminated and perfect!
        return std::string(szDetailedStatus);
    }
    else
    {
        CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s GetDetailedConnectionStatus() returned %d!", __func__, nRes);
        return "";
    }
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

PgeGsnClient::PgeGsnClient(PGEcfgProfiles& cfgProfiles) :
    PgeGsnWrapper(cfgProfiles),
    m_hConnection(k_HSteamNetConnection_Invalid),
    m_hConnectionServerSide(k_HSteamNetConnection_Invalid)
{
    m_addrServer.Clear();
    memset(&m_connRtStatus, 0, sizeof(m_connRtStatus));
} // PgeGsnClient()

PgeGsnClient::PgeGsnClient(const PgeGsnClient& other) :
    PgeGsnWrapper(other.m_cfgProfiles)
{

}

PgeGsnClient& PgeGsnClient::operator=(const PgeGsnClient&)
{
    return *this;
}

int PgeGsnClient::receiveMessages(ISteamNetworkingMessage** pIncomingMsg, int nIncomingMsgArraySize) const
{
    // ReceiveMessagesOnConnection() basically copies the pointers to messages from GNS's internal linked list,
    // and unlinks these from the linked list, so it is cheap copy to our array
    return m_pInterface->ReceiveMessagesOnConnection(m_hConnection, pIncomingMsg, nIncomingMsgArraySize);
}

bool PgeGsnClient::validateSteamNetworkingMessage(const HSteamNetConnection&) const
{
    return true;
}

void PgeGsnClient::updateIncomingPgePacket(pge_network::PgePacket&, const HSteamNetConnection&) const
{
    // here we are client, we don't set pkt.connHandle because we expect it to be already properly filled in by sender (server)!
}

void PgeGsnClient::onSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    // This function is also invoked on main thread when I call PgeGsnClient.pollConnectionStateChanges() from PGE::runGame()
    // so no need to utilize mutexes around here.
    // And the other function pollIncomingMessages() is also invoked by PGE::runGame().
    // So it is safe to do operations on m_queuePackets.

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
            CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s: CLIENT problem 1 (rejected I guess) (%s)",
                __func__, pInfo->m_info.m_szEndDebug);
        }
        else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
        {
            CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s: CLIENT problem 2 (guess our machine has lost network connection) (%s)",
                __func__, pInfo->m_info.m_szEndDebug);
        }
        else
        {
            // NOTE: We could check the reason code for a normal disconnection
            CConsole::getConsoleInstance("PgeGsnClient").EOLn("%s: CLIENT problem 3 (closed by peer) (%s)",
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
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s: CLIENT Connecting ...", __func__);
        break;

    case k_ESteamNetworkingConnectionState_Connected:
    {
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s: CLIENT Connected to server OK", __func__);

        // KEKEKEKEKE
        // you jumped here from PgePacket.h MsgUserConnected.
        char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
        pInfo->m_info.m_addrRemote.ToString(szAddr, sizeof(szAddr), true);
        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s: CLIENT Remote address: %s", __func__, szAddr);

        CConsole::getConsoleInstance("PgeGsnClient").OLn("%s: CLIENT conn. description: %s", __func__, pInfo->m_info.m_szConnectionDescription);

        break;
    }
    default:
        // Silences -Wswitch
        break;
    }
}
