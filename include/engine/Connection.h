/////////////////////////////////////////////////
/*
 * Definition of Connection classes
 * Connection handles sending and receiving from its connected address
 *
 * Author: Harrison Beachey
 * 
 *  Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#pragma once

#include "Address.h"

#define PACKET_HEADER_SIZE (3 * sizeof(u08) + sizeof(u16))

#if PLATFORM == PLAT_WIN
  typedef SOCKET Socket;
#else
  typedef int Socket;
#endif
  
//forward declare for friend class declaration
namespace Networking
{
  class NetworkingSystem;
};

class Encoder;
struct DebugPrint;//Forward declared for debug purposes
struct Clumsy;//Same as above
///class used to connect to other clients/the server
//keeps track of and inserts acks, sequence num, and ping
class Connection
{
  friend class Networking::NetworkingSystem;
  public:
    Connection();
    Connection(Address);
    Connection(Address& addr, Socket s, bool udp);

    void SetAddress(Address);
    Address& GetAddress();

    bool Open(u16 port, bool udp = true, bool listen = false);
    //acceptable ip's for multicasting: 224.0.0.0 - 239.255.255.255
    bool OpenMulticast(Address& localInterface, bool listen = true);

    bool IsOpen();
    void Close();
    void Terminate();

    Socket& GetSocket();
    
    bool IsUDP();
    
    f32  GetPing();
    //returns this connection ack bit field
    u16  GetAckBits();
    //Returns the last packet the ack bits know about
    u08  GetAckReferenceNumber();
    //returns lsat sequence nubmer this connection recieved
    u32  GetLastRecievedSeq();

    //returns the acks from the last packet recieved here
    u16  GetDestinationAcks();

    //only accepts a connection if tcp
    Connection* Accept();

    //sends to stored address
    //Inserts packet header to data
    //returns sent message's sequence number
    u08 Send(const c08* data, s32 dataSize, u08 headerFlags = 0);
    //UDP only
    u08 SendWith(Socket& s, const c08* data, s32 dataSize, u08 headerFlags = 0, s32 seqNum = -1);

    //returns bytes received, size is to be the size of the buffer in data
    //effects:
    //data is set to received data
    //fromAddr is set to address received from
    s32 Recieve(void* data, s32 size, Address& fromAddr);
    
    //returns sequence number of received packet OR negative error codes
    s32 ReadHeader(Encoder& reader);

    f32    GetPacketLossRatio();
    u32    TimeSinceLastSend();
    bool   DisconnectTimeOut();
    bool   ConnectionTerminated();

    void TestSequenceWrap();
  
  private:
    bool OpenMulticastHelper(sockaddr_in& data);
    void SetNonBlocking(bool dontBlock = true);

    bool DroppedPacket(void);
    u32  ConvertSequenceNumber(u08 seq);
    //returns new datapointer, edits dataSize to be new size
    c08*  AddHeaderToData(const c08* data, s32& dataSize, u08 flags, s32 seqNumOverride = -1);
    
    void  HandleSendFlags(u08& flags);
    void  HandleReceiveFlags(u08 flags);

    s32   ReadResentPacketHeader(Encoder& reader);

    void BaseSend(Socket& s, Address& addr, const c08* data, s32 dataSize);

    static u08 debugPrint_;
    bool    UDP_;
    bool    multicast_;
    bool    listen_;
    bool    open_; //if the socket is open
    u08     nextSequenceNum_; //next sequence number to put in packet header
    u32     lastSeqRecieved_;
    u16     packetsRecieved_; //acks

    u08     ackRefNum_;//The last sequence number that the dest successfully received (The base reference for the ack bit field)
    u16     destAcks_;
    u08     destLastSeqRecieved_;//I think this is the same thing as ackRefNum, but not sure, so leaving it alone

    f32     ping_;
    bool    sendHeartBeat_;
    bool    sendSyn_;
    bool    sendFin_;
    bool    startConnection_;
    bool    terminateConnection_;
    u32     timeSent_;
    u32     timeLastReceived_;
    static u32 disconnectWaitTime_;

    f32     uniqueRatioPacketLoss_; // The actuall packet loss for this connection
    static f32 packetLoss_; // in percentage (0.0f - 100.0f), amount of packets to drop

    Socket  socket_;
    Address addr_; // default addr to send/recieve to/from

    friend struct DebugPrint;
    friend struct Clumsy;
    friend class  NetworkSystem;
};
