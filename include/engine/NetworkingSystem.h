#pragma once
/************************************************************************/
/*
  File created by Nathan Hitchcock (nathan.hitchcock@digipen.edu)
  CS261 - Project 1
  1.29.2014
  All content copyright DigiPen Institute of Technology

  This file defines the general Networking systems behavior
*/
/************************************************************************/
#pragma comment (lib, "ws2_32.lib")

#include "Message.h"
#include "Connection.h"
#include "IProtocol.h"
#include <LockableResource.h>
#include <thread>
#include <list>

class ArgumentParser;
struct Listen;//Forward declare listen command class for debug purposes
struct BufferSize;

typedef void (*NewTCPConnectionFunc)(s32 connectionID);
typedef void (*NewUDPConnectionFunc)(s32 connectionID);
typedef void (*HandleDisconnectFunc)(s32 connectionID);

void DefaultNewTCPConnectionFunc(s32 connectionID);
void DefaultNewUDPConnectionFunc(s32 connectionID);
void DefaultHandleDisconnectFunc(s32 connectionID);

static const s32 MCAST_ID = -2;

namespace Networking
{

namespace NetworkingSystemError
{
  //Negative errors are errors from winsock
  enum NETWORKING_SYSTEM_ERROR
  {
    OK,                          //Nothing is wrong
    EMPTY_MESSAGE,               //If someone tries to send an empty message
    CONNECTION_OPEN_FAILURE,     //If the socket could not be initiated and connected
    CONNECTION_INVALID_OPERATION,//If the connection is closed and someone decides to send/recv on it
    NO_CONNECTION,
    WSA_STARTUP_FAILURE,         //If winsock experiences any error on startup
    WSA_CLEANUP_FAILURE,         //If winsock experiences any errors on cleanup
    CUSTOM_PROTO_PROCESS_FAILURE,
    CUSTOM_PROTO_ACCEPT_FAILURE,
    CUSTOM_PROTO_SEND_FAILURE,
    CUSTOM_PROTO_RECV_FAILURE,
    NETWORKING_SYSTEM_ERROR_COUNT//The count of how many errors we have in the enum
  };

}//~NetworkingSystemError NS

class NetworkingSystem
{
  public:
    NetworkingSystem(bool acceptConnections = false, //Construct/initialize winsock
                     NewTCPConnectionFunc newTCPConnectFunc_ = DefaultNewTCPConnectionFunc,
                     NewUDPConnectionFunc newUDPConnectFunc_ = DefaultNewUDPConnectionFunc,
                     HandleDisconnectFunc connectionTimeoutFunc_ = DefaultHandleDisconnectFunc);
    ~NetworkingSystem();//Cleanup winsock
    
    int Initialize(const ArgumentParser& parser);
    void Initialize();

    //////////////
    /////Custom protocol funcs

    //Adds a default initiliazed protocol listener
    template <typename T>
    void AddCustomProtocol(u16 port);

    //Adds a copy constructed protocol listener
    template <typename T>
    void AddCustomProtocol(T& toCopy, u16 port);

    s32 Connect(Address con, bool upd = true, u16 localPort = 0); //Return connection handle, negative values are errors
    //valid addresses are 224.0.0.0 - 239.255.255.255
    s32 Multicast(Address& localAddr, Address con, bool _listen = true);

    s32 Send(Message msg, s32 hCon);      //Returns negative error codes, positive if successful (Perhaps estimated RTT?) DOES NOT SEND IF EMPTY MESSAGE
    s32 NumReceivedInQueue(s32 hCon);
    Message Receive(s32 hCon); //Takes the handle to the connection to send to, returns message, empty message mean nothing to recv
    s32 Disconnect(s32 hCon); //Cleans and closes the connection of the given handle

    void SetBufferSize(s32 size);
    s32 GetBufferSize(void) const;
    s32 GetMaxDataSize(void) const;

    void CreateUdpListener(u16 port); //I think we should give the user the option to create this listener.
    void CreateTcpListener(u16 port); // Also this listener

    s32 NumConnections();
    void PrintConnections();
    std::list<s32> GetConnections();
    void SetAcceptConnections(bool allow);
    bool GetAcceptConnections(void)const;
    void SetHandleDisconnectFunc(HandleDisconnectFunc func);
    void SetNewTCPConnectionFunc(NewTCPConnectionFunc newTCPConnectFunc);
    void SetNewUDPConnectionFunc(NewUDPConnectionFunc newUDPConnectFunc);

    void SetLag(u32 time);
    u32  GetLag(void);
    //This is so I can output debug info easily for asteroids
    istring StringifyConnectionInfo(s32 con);
    unsigned GetConnectionPing(s32 con);
    NetworkingSystemError::NETWORKING_SYSTEM_ERROR GetNextError(void);

    const Address& GetConnectionAddress(s32 con);

  private:
#pragma region Structs
    struct NetMessageSent //A message our system sent
    {
      Message m_; //The message data
      u08 seqNum_; //The sequence number given to this message when sent
      u08 ackNum_;
      u08 flags_;
      time_t resendTimer_; //The timer to count up till RESEND_TIME
    };

    struct NetMessageRecvd //A message our system recvd
    {
      Message m_; //Message data
      u16 lastSeqNumRecvd_; //The last message that the dest system successfully received
      u16 ackBits_; //The ack bitfield on the packet
    };
        
    struct RecievedData
    {
      u08* data;
      s32  dataSize;
      Address addr;
    };

    struct ConnectionData
    {
      ConnectionData(Address& addr);
      ConnectionData(Connection& con);

      //Returns the number of packets removed from sent queue
      u08 RemoveAckedMessages(u08 refNum, u16 acks);
      void JoinMessages(void); //join packets that were split

      Connection con_;//The connection to the other endpoint
      s32 conID_;
      
      u32 timeLastHeartBeat_;
      //All the message queues here
      std::queue<Message> messagesToSend_;//The "send" queue
      std::list<NetMessageSent> messagesSent_;//The "waiting for ack" queue
      std::queue<Message> messagesReceived_;//The "Received" queue
      std::vector<Message> messagesToJoin_;

      bool receiveInOrder;
    private:
      s32  lastSeqTaken_;
    };

    struct CustomProtocolData
    {
    private:
      int Initialize(u16 port);
    public:
      //Ctor and dtor to help with management
      CustomProtocolData(u16 port);
      ~CustomProtocolData();

      //Returns number of accepted connections
      int AcceptConnections(void);

      int RemoveConnection(Socket con);

      Socket listener;
      LockableResource<std::list<Socket>>* connections;

      IProtocol* protocol;
    };
#pragma endregion

    typedef u16 time_t;// The timer type
    typedef LockableResource<std::vector<LockableResource<ConnectionData>*>>             ConnectionDataCont;
    typedef LockableResource<std::queue<NetworkingSystemError::NETWORKING_SYSTEM_ERROR>> ErrorCont;
    //used for passing recieved data between recv and parser
    typedef LockableResource<std::queue<RecievedData*>>                                  RecievedDataQueue;
    typedef LockableResource<ConnectionData>*                                            LockableConData;
    //Custom protocol lists
    typedef LockableResource<std::vector<LockableResource<CustomProtocolData*>*>>         CustomProtocolCont;
         
#pragma region Functions
    //not implemented constructors
    NetworkingSystem(const NetworkingSystem& rhs);
    NetworkingSystem& operator=(const NetworkingSystem& rhs);

    bool InitializeSockets();
    //accept new TCP connections
    void TCPAccept();

    void PushError(NetworkingSystemError::NETWORKING_SYSTEM_ERROR nseErr);

    //returns true if something was recieved
    bool RecieveHelper(Connection& con, RecievedData* dat);
    //reads message header and seperates out messages
    void ParseMessages(u08* data, s32 dataSize, Address& fromAddr);
    void ReadMessagesFromBuffer(Encoder& reader, std::queue<Message>& recievedQueue, s32 sequence = 0);
    u08* ResendEncapsulate(u08* buffer, u16& size, u08 seq);

    void RemoveConnection(s32 hCon);

    //////////////
    /////thread funcs
    void ThreadCustomProtocolUpdate(); // Updates and sends
    void ThreadCustomProtocolReceive();
    void ThreadUpdate();//Updates the queue by sending
    void ThreadRecieve();
    
    void UpdateReceive(LockableConData cd);
    void UpdateResend(LockableConData cd);
    void UpdateSend(LockableConData cd);
    void UpdateHeartbeat(LockableConData cd);
    
#pragma endregion

#pragma region Vars
    ConnectionDataCont lrConnections_;
    CustomProtocolCont conCustomProtocols_;
    RecievedDataQueue  dataToParse_;
    s32                numConnections_;

    std::thread updateThread_;
    std::thread recvThread_;
    std::thread custProtoUpdateThread_;
    std::thread custProtoRecvThread_;

    bool bQuit_;//Tells threads when to exit

    time_t tWaitBeforeResend_;
    time_t tBetweenUpdates_;

    bool acceptConnections_;
    Connection tcpAccept_;
    Connection udpConnection_;
    
    Connection* multicast_;
    Address multicastRecieveAddr_; //yay hacks
    LockableResource<std::queue<Message>> multicastRecieved_;

    NewTCPConnectionFunc newTCPConnectFunc_;
    NewUDPConnectionFunc newUDPConnectFunc_;
    HandleDisconnectFunc handleDisconnectFunc_;

    //Error queue needs to be thread safe
    //Need some kind of semaphore to count messages to send
    ErrorCont lrErrorQueue_;
    s32 maxPacketLen_;
    
    u32 latency_;

    static s32 timeHeartBeatWait_;
    static u08 debugPrint_;
#pragma endregion


    friend struct Listen;
    friend struct BufferSize;
    friend struct DebugPrint;
}; //~NetworkingSystem

}//~Networking

#include "NetworkingSystem_.h"