#pragma once

/*
    ###################################################################################
    PgePacket.h
    This file is part of PGE.
    External header.
    PR00F's Game Engine packets
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../PGEallHeaders.h"

#include <stddef.h>  // offsetof()

#include <limits>

// These packet and message structs are sent between server and clients.
// Different endianness is not considered as an issue because all machines expected to use this have same endianness for now.
// In case this changes in the future, use a lib like cereal to easily solve endianness issue.

namespace pge_network
{

    typedef uint32_t PgeNetworkConnectionHandle;

    // since 0 is considered as an invalid handle at GSN level, we use this to refer to server, since
    // server never wants to send to this connection, or it automatically injects when this value is used.
    constexpr PgeNetworkConnectionHandle ServerConnHandle = 0;

    enum class PgePktId : uint32_t
    {
        UserConnectedServerSelf = 0,
        UserDisconnectedFromServer,
        Application
    };

    // server -> self (injection)
    // server app can process this in arbitrary way and might send another custom message to all clients about new user;
    // server app should not send this to clients; it is NOT allowlisted for clients by default;
    // it is NOT allowlisted either for server app because server should not receive this over network, it just injects it for itself
    struct MsgUserConnectedServerSelf
    {
        static const PgePktId id = PgePktId::UserConnectedServerSelf;
        static const uint8_t nIpAddressMaxLength = 48;

        bool bCurrentClient;  // true means server injected itself its own birth, false means a client has connected
        char szIpAddress[nIpAddressMaxLength];
        // TODO: server always knows IP address of clients, but doesn't know its own IP address used by the clients to connect.
        // It should be grabbed from the first connecting client.
        // See if remote server address logged is the good server address in: PgeGnsWrapper::onSteamNetConnectionStatusChanged(),
        // client code, when client reaches k_ESteamNetworkingConnectionState_Connected. Jump there by searching for "KEKEKEKEKE"
    };

    // server -> clients and to self
    // it is NOT allowlisted for server app because server should not receive this over network, it just injects it for itself
    struct MsgUserDisconnectedFromServer
    {
        static const PgePktId id = PgePktId::UserDisconnectedFromServer;
    };

    typedef uint32_t TPgeMsgAppMsgId;

    // application-specific message
    // server <-> client
    // With allow listing, app messages can be separately allowed to be processed by clients and server based on TPgeMsgAppMsgId.
    struct MsgApp
    {
        static const PgePktId id = PgePktId::Application;
        static const uint8_t nMaxMessageLength = 240;

        static TPgeMsgAppMsgId& getMsgAppMsgId(MsgApp& msgApp);
        static uint8_t&  getMsgAppDataActualSize(MsgApp& msgApp);  // TODO: delete this non-const version later when not needed
        static const uint8_t& getMsgAppDataActualSize(const MsgApp& msgApp);
        static const uint8_t getMsgAppTotalActualSize(const MsgApp& msgApp);
        static uint8_t* getMsgAppData(MsgApp& msgApp);
        static bool fillMsgApp(
            pge_network::MsgApp& myAppMsg,
            const pge_network::TPgeMsgAppMsgId& msgAppMsgId,
            const uint8_t* msgAppData,
            uint8_t nMsgAppDataSize);
        
        TPgeMsgAppMsgId msgId;  // this is checked by engine upon polling for new messages against the allowlists
        uint8_t nMsgSize;
        /* This 'cMsgData' memory area is for 1 application message defined at application level, not here.
           So the application is responsible for copying the message here.
           The C++ standard guarantees that the members of a class or struct appear in memory in the same order as they are declared.
           This 'cMsgData' member needs to be the LAST member of this struct.
           When packaging MsgApp into MsgAppArea, we should put the actually used memory area (nMsgSize) and not the whole area, and
           increase MsgAppArea::m_nMessageCount by 1.
         */
        uint8_t cMsgData[nMaxMessageLength];
    };

    static_assert(MsgApp::nMaxMessageLength <= 255u /*std::numeric_limits<uint8_t>::max()  TODO: #define NOMINMAX so using windows headers wont define min and max macros! */,
        "Size of MsgApp data should fit in MsgApp::nMsgSize");

    // memory area within a PgePacket, used when we are sending app message(s) in the packet
    struct MsgAppArea
    {
        static const uint8_t nMessagesAreaLength = 255 /*std::numeric_limits<uint8_t>::max()  TODO: #define NOMINMAX so using windows headers wont define min and max macros!*/;

        uint8_t m_nMessageCount;              // should be readable only by application
        uint8_t m_nActualMessagesAreaLength;  // should be readable only by application
        /* This 'cData' memory area is for m_nMessageCount number of different app messages.
           The C++ standard guarantees that the members of a class or struct appear in memory in the same order as they are declared.
           This 'cData' member needs to be the LAST member of this struct.
           When sending, we should send the actually used memory area and not the whole area.
           We don't need to store the actually used size since GSN will tell the actual pkt size anyway on polling messages.
           In 'cData' we store at least 1 variable-sized MsgApp.
         */
        uint8_t cData[nMessagesAreaLength];   // should be writable by application, but the pointer to it should be calculated by wrapper function
    };
    
    static_assert(
        sizeof(MsgApp) <= MsgAppArea::nMessagesAreaLength,
        "At least 1 full MsgApp should fit into MsgAppArea.cData!");

    struct PgePacket
    {
        enum class AutoFill {
            NONE,
            ZERO
        };

        // basic packet get/set

        static PgePktId& getPacketId(pge_network::PgePacket& pkt);
        static const PgePktId& getPacketId(const pge_network::PgePacket& pkt);

        static PgeNetworkConnectionHandle& getServerSideConnectionHandle(pge_network::PgePacket& pkt);
        static const PgeNetworkConnectionHandle& getServerSideConnectionHandle(const pge_network::PgePacket& pkt);

        static MsgUserConnectedServerSelf& getMessageAsUserConnected(pge_network::PgePacket& pkt);
        static const MsgUserConnectedServerSelf& getMessageAsUserConnected(const pge_network::PgePacket& pkt);

        static MsgUserDisconnectedFromServer& getMessageAsUserDisconnected(pge_network::PgePacket& pkt);
        static const MsgUserDisconnectedFromServer& getMessageAsUserDisconnected(const pge_network::PgePacket& pkt);

        static MsgAppArea& getMessageAppArea(pge_network::PgePacket& pkt);
        static const MsgAppArea& getMessageAppArea(const pge_network::PgePacket& pkt);

        static uint8_t getMessageAppCount(const pge_network::PgePacket& pkt);
        static uint8_t getMessageAppsTotalActualLength(const pge_network::PgePacket& pkt);

        static bool isMessageAppAreaFull(const pge_network::PgePacket& pkt);

        // packet initializers

        static void initPktBasic(
            pge_network::PgePacket& pkt,
            const PgePktId& pktId,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const AutoFill& autoFill);

        /*
        * Initializes the given packet to be used for sending MsgUserConnectedServerSelf.
        * A packet needs to be initialized only once.
        *
        * @param pkt                  The packet to be initialized.
        * @param connHandleServerSide The server-side connection handle of the sender (whoever is filling and sending the packet).
        * @param bCurrentClient       True if the user is the receiver as well (server notifies self about being born), false if the user is a different entity (a client).
        * @param sIpAddress           The IP address of the user being connected.
        */
        static bool initPktPgeMsgUserConnected(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sIpAddress);

        /*
        * Initializes the given packet to be used for sending MsgUserDisconnectedFromServer.
        * A packet needs to be initialized only once.
        *
        * @param pkt                  The packet to be initialized.
        * @param connHandleServerSide The server-side connection handle of the sender (whoever is filling and sending the packet).
        */
        static void initPktPgeMsgUserDisconnected(
            PgePacket& pkt,
            const PgeNetworkConnectionHandle& connHandleServerSide);

        /*
        * Initializes the given packet to be used to store app messages.
        * A packet needs to be initialized only once.
        * Then one or more addPktMsgApp() or preparePktMsgAppFill() can be used to store app messages inside it.
        * 
        * @param pkt                  The packet to be initialized.
        * @param connHandleServerSide The server-side connection handle of the sender (whoever is filling and sending the packet).
        * @param autoFill             If we want to fill the whole memory area or leave it untouched.
        */
        static void initPktMsgApp(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const AutoFill& autoFill = AutoFill::ZERO);

        /*
        * Copies the given msgApp into the given pkt.
        * Due to copying the given msgApp, from performance perspective try to avoid using this function and prefer preparePktMsgAppFill()!
        * Note that not the full MsgApp is copied, only the actually used memory area of the struct.
        * 
        * @param pkt    The packet to where we are going to copy the given msgApp struct.
        * @param msgApp The struct which will be copied into the given packet.
        * 
        * @return True on success, false otherwise.
        */
        static bool addPktMsgApp(
            pge_network::PgePacket& pkt,
            const pge_network::MsgApp& msgApp);

        /*
        * Gives a pointer to the proper memory area within the given pkt to where the application can write its custom app message data.
        * It checks if there is enough space available in the given packet for the app message data we are planning to write, and if so,
        * it sets the given app message id and app message data size at the proper location within the packet and returns a pointer pointing
        * to that memory area within the packet that can be directly written by the application to store the actual app message data.
        * 
        * From performance perspective using this function is preferred over addPktMsgApp() because in practice it results in less memory
        * copy operations since the caller can directly write its data instead of copying a MsgApp struct.
        * 
        * @param pkt             The packet to where we are going to copy the given msgApp struct.
        * @param msgAppId        The id of the app message we are planning to write into the given packet.
        * @param nMsgAppDataSize Size of the app message data we are planning to write into the given packet.
        * 
        * @return On success: a valid pointer pointing into the proper area of the given packet, nullptr otherwise.
        */
        static uint8_t* preparePktMsgAppFill(
            pge_network::PgePacket& pkt,
            TPgeMsgAppMsgId msgAppId,
            uint8_t nMsgAppDataSize
        );

        PgePktId pktId;
        PgeNetworkConnectionHandle m_connHandleServerSide;

        union
        {
            MsgUserConnectedServerSelf userConnected;
            MsgUserDisconnectedFromServer userDisconnected;
            MsgAppArea app; // application should load/store its custom messages here
        } msg;
    };

} // namespace pge_network
