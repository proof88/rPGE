#pragma once

/*
    ###################################################################################
    PgeGsnWrapper.h
    This file is part of PGE.
    Internal header.
    PR00F's Game Engine networking subsystem
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../PGEallHeaders.h"

#include <chrono>  // requires cpp11
#include <cstdint>
#include <deque>
#include <set>
#include <string>

#include "../Config/PGEcfgProfiles.h"
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


/**
    PR00F's Game Engine's wrapper for GameNetworkingSockets library.
*/
class PgeGsnWrapper
{
#ifdef PGE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PgeGsnWrapper is included")   
#endif

public:

    static const uint16 DEFAULT_SERVER_PORT = 27020;

    // ---------------------------------------------------------------------------

    virtual ~PgeGsnWrapper();

    /**
    * Initializes the GameNetworkingSockets library.
    * To be used by initialization of derived classes.
    *
    * @return True if initialization successful, false if initialization is unsuccessful or if it is already initialized.
    */
    bool init();

    /**
    * Uninitializes the GameNetworkingSockets library.
    * Should be extended by derived classes.
    * 
    * @return True if uninitialization is successful or not initialized, false otherwise.
    */
    virtual bool destroy();

    /**
    * Gets the state of initialization of the GameNetworkingSockets library.
    * Should be extended by derived classes.
    *
    * @return True if initialized, false otherwise.
    */
    virtual bool isInitialized() const;
    
    /**
    * Moves incoming SteamNetworkingMessages from GameNetworkingSockets layer to m_queuePackets as PgePackets.
    * 
    * @return True on success, false on error.
    */
    bool PollIncomingMessages();

    void PollConnectionStateChanges();
   
    std::size_t getPacketQueueSize() const;
    pge_network::PgePacket popFrontPacket() noexcept(false);

    std::set<pge_network::PgePktId>& getAllowListedPgeMessages();
    std::set<pge_network::TPgeMsgAppMsgId>& getAllowListedAppMessages();

    bool connectToServer(const std::string& sServerAddress); /* temporal, now I dont have better idea in this time */
    bool DisconnectClient();
    bool startListening();
    bool stopListening();

    uint32_t getRxPacketCount() const;
    uint32_t getTxPacketCount() const;
    uint32_t getInjectPacketCount() const;

    uint32_t getRxPacketPerSecondCount() const;
    uint32_t getTxPacketPerSecondCount() const;
    uint32_t getInjectPacketPerSecondCount() const;

protected:

    static PgeGsnWrapper* s_pCallbackInstance;

    PGEcfgProfiles& m_cfgProfiles;
    
    ISteamNetworkingSockets* m_pInterface;

    std::deque<pge_network::PgePacket> m_queuePackets;
    std::set<pge_network::PgePktId> m_allowListedPgeMessages;
    std::set<pge_network::TPgeMsgAppMsgId> m_allowListedAppMessages;

    uint32_t m_nRxPktCount;
    uint32_t m_nTxPktCount;
    uint32_t m_nInjectPktCount;

    std::chrono::time_point<std::chrono::steady_clock> m_time1stRxPkt;
    std::chrono::time_point<std::chrono::steady_clock> m_time1stTxPkt;
    std::chrono::time_point<std::chrono::steady_clock> m_time1stInjectPkt;

    // ---------------------------------------------------------------------------

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

    explicit PgeGsnWrapper(PGEcfgProfiles& cfgProfiles);
    PgeGsnWrapper(const PgeGsnWrapper&); 
    PgeGsnWrapper& operator=(const PgeGsnWrapper&);

    virtual int receiveMessages(ISteamNetworkingMessage** pIncomingMsg, int nIncomingMsgArraySize) = 0;
    virtual bool validateSteamNetworkingMessage(const HSteamNetConnection& connHandle) const = 0;
    virtual void updateIncomingPgePacket(pge_network::PgePacket& pkt, const HSteamNetConnection& connHandle) const = 0;
    virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) = 0;

    friend class PGE;

}; // class PgeGsnWrapper