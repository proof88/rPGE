#pragma once

/*
    ###################################################################################
    PGESysNET.h
    This file is part of PGE.
    Internal header.
    PR00F's Game Engine networking subsystem
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../PGEallHeaders.h"

#include <cstdint>
#include <deque>
#include <map>
#include <string>

#include "PgePacket.h"

// this idea of building include paths is coming from:
// https://stackoverflow.com/questions/32066204/construct-path-for-include-directive-with-macro
#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define PATH2(x,y) STR(IDENT(x)IDENT(y))
#define PATH3(x,y,z) STR(IDENT(x)IDENT(y)IDENT(z))

#define GAMENETWORKINGSOCKETS_VER 1.4.0
#define GAMENETWORKINGSOCKETS_VER_STR STR(GAMENETWORKINGSOCKETS_VER)

// TODO: add "Network/GameNetworkingSockets-1.4.0/include" to project include dirs, do not use that path here!
// TODO: to log lib version, extract version from %AdditionalIncludeDirectories
#include PATH3(GameNetworkingSockets-,GAMENETWORKINGSOCKETS_VER,/include/steam/steamnetworkingsockets.h)
#include PATH3(GameNetworkingSockets-,GAMENETWORKINGSOCKETS_VER,/include/steam/isteamnetworkingutils.h)

struct TClient
{
    std::string sCustomName;  /**< App level can set a custom name for client which is useful for debugging. */
};

// TODO: this whole PGESysNET should be also separated into client and server code, similar to how PgeNetwork separates
// it into PgeClient and PgeServer, but currently no time for this ...

/**
    PR00F's Game Engine networking subsystem.
*/
class PGESysNET
{
#ifdef PGE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PGESysNET is included")   
#endif

public:
    static const uint16 DEFAULT_SERVER_PORT = 27020;

    static PGESysNET& createAndGet();           /**< Creates and gets the singleton implementation instance. */

    static bool isServer();

    // ---------------------------------------------------------------------------

    virtual ~PGESysNET();

    bool initSysNET(void);
    bool destroySysNET(void);
    bool isInitialized() const;

    const HSteamNetConnection& getConnectionHandle() const;
    const char* getServerAddress() const;
    
    bool PollIncomingMessages();
    void PollConnectionStateChanges();

    void SendPacketToClient(HSteamNetConnection conn, const pge_network::PgePacket& pkt);
    void SendPacketToAllClients(const pge_network::PgePacket& pkt, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
    void SendToServer(const pge_network::PgePacket& pkt);

    std::deque<pge_network::PgePacket>& getPacketQueue();  // TODO: TEMPORAL: obviously we should not allow this kind of access
    std::set<pge_network::PgePktId>& getBlackListedPgeMessages();
    std::set<pge_network::TPgeMsgAppMsgId>& getBlackListedAppMessages();

    const SteamNetConnectionRealTimeStatus_t& getRealTimeStatus(bool bForceUpdate);

    bool connectToServer(const std::string& sServerAddress); /* temporal, now I dont have better idea in this time */
    bool DisconnectClient();
    bool startListening();
    bool stopListening();

    void WriteServerClientList();

private:

    static PGESysNET* s_pCallbackInstance;
    static bool s_bServer;

    uint16 m_nPort;
    SteamNetworkingIPAddr m_addrServer;  // used by client only
    char m_szAddr[SteamNetworkingIPAddr::k_cchMaxString];  // used by client only
    HSteamNetConnection m_hConnection; // used by client only
    ISteamNetworkingSockets* m_pInterface;
    HSteamListenSocket m_hListenSock;  // used by server only
    HSteamNetPollGroup m_hPollGroup;   // used by server only
    SteamNetConnectionRealTimeStatus_t m_connRtStatus;  // used by client only

    std::map< HSteamNetConnection, TClient > m_mapClients;  // used by server only
    // The connection handle is used to identify clients in the map.
    // This handle for a client is NOT the same as the handle on the client side, obviously.
    // Note that the first connection is this map is an invalid connection (k_HSteamNetConnection_Invalid), which
    // is the server itself, this is how this layer stores nickname for the server (self).
    

    std::deque<pge_network::PgePacket> m_queuePackets;  // used by both client and server
    std::set<pge_network::PgePktId> m_blackListedPgeMessages;  // used by both client and server
    std::set<pge_network::TPgeMsgAppMsgId> m_blackListedAppMessages;  // used by both client and server

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

    // ---------------------------------------------------------------------------

    PGESysNET();
    PGESysNET(const PGESysNET&); 
    PGESysNET& operator=(const PGESysNET&);

    void SetClientNick(HSteamNetConnection hConn, const char* nick);
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

    friend class PGE;

}; // class PGESysNET