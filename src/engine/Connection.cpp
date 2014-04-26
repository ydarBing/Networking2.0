/////////////////////////////////////////////////
/*
 * Implementation of both the Connection Classe
 *
 * Author: Harrison Beachey
 * 
 *  Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#include "NetworkingPrecompiled.h"
#include "Connection.h"
#include "Encoder.h"

#include <errno.h>

#define OFF 0
#define ON 1
#define BROADCAST_INSTEAD OFF

//If dprint level 1, dont print on heart beats
#define FLAG_PRINT(statement) if((debugPrint_ == 1 && !(flags & PacketHeaderFlags::PHF_HRT)) || debugPrint_ == 2) \
  DPRINT(0, statement);

static const u32 INIT_SEQUENCE_NUM = 0;

u08 Connection::debugPrint_ = 0;
u32 Connection::disconnectWaitTime_ = 10000;// 10 seconds to wait before dc
f32 Connection::packetLoss_ = 0.0f;

Connection::Connection()
  : open_(false), addr_(Address(127, 0, 0, 1, 8003)), packetsRecieved_(0),
  lastSeqRecieved_(INIT_SEQUENCE_NUM), nextSequenceNum_(INIT_SEQUENCE_NUM + 1), ping_(16), 
  ackRefNum_(INIT_SEQUENCE_NUM), sendSyn_(false), sendFin_(false), 
  startConnection_(true), timeLastReceived_(GetTickCount()), terminateConnection_(false), multicast_(false),
  uniqueRatioPacketLoss_(0.0f), timeSent_(GetTickCount())
{}

Connection::Connection(Address addr)
  : open_(false), addr_(addr), packetsRecieved_(0),
  lastSeqRecieved_(INIT_SEQUENCE_NUM), nextSequenceNum_(INIT_SEQUENCE_NUM + 1), ping_(16),
  ackRefNum_(INIT_SEQUENCE_NUM), sendSyn_(false), sendFin_(false),
   startConnection_(true), timeLastReceived_(GetTickCount()), terminateConnection_(false), multicast_(false),
  uniqueRatioPacketLoss_(0.0f), timeSent_(GetTickCount())
{}

Connection::Connection(Address& addr, Socket s, bool udp)
  : open_(true), addr_(addr), packetsRecieved_(0),
  lastSeqRecieved_(INIT_SEQUENCE_NUM), nextSequenceNum_(INIT_SEQUENCE_NUM + 1), ping_(16),
  socket_(s), UDP_(udp), ackRefNum_(INIT_SEQUENCE_NUM), sendSyn_(false), 
  sendFin_(false), startConnection_(true), timeLastReceived_(GetTickCount()), 
  terminateConnection_(false), multicast_(false), uniqueRatioPacketLoss_(0.0f),
  timeSent_(GetTickCount())
{}

void Connection::SetAddress(Address a)
{
  addr_ = a;
}

Address& Connection::GetAddress()
{
  return addr_;
}

f32 Connection::GetPing()
{
  return ping_;
}

bool Connection::Open(u16 port, bool udp, bool _listen)
{
  listen_ = _listen;

  u16 socketType = 0;
  u16 sockProto = 0;
  sockaddr_in address;
  address.sin_family = AF_INET;

  if(udp)
  {
    UDP_ = true;
    socketType = SOCK_DGRAM;
    sockProto = IPPROTO_UDP;
    
    listen_ = false;
    
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
  }
  else
  {
    UDP_ = false;
    socketType = SOCK_STREAM;
    sockProto = IPPROTO_TCP;

    if(listen_)
    {
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      address.sin_port = htons(port);
    }
    else
    {
      address.sin_addr.s_addr = addr_.GetAddress();
      address.sin_port = htons(addr_.GetPort());
    }
  }

  ERRCHECK((socket_ = socket(AF_INET, socketType, sockProto)), "Error Opening Socket!", false);

  if(udp || listen_)
  {
    ERRCHECK(bind(socket_, (const sockaddr*)&(address), sizeof(sockaddr_in)), "Error Binding Socket", false);
  }
  else
  {
    ERRCHECK(connect(socket_, (const sockaddr*)&address, sizeof(sockaddr_in)), "Error on Connect", false);
  }

  SetNonBlocking();

  if(listen_)
  {
    ERRCHECK(listen(socket_, 5), "Error on listen", false);
  }

  open_ = true;
  return true;
}

bool Connection::OpenMulticast(Address& localInterface, bool listen /* = true */)
{
  listen_ = listen;
  UDP_ = true;
  multicast_ = true;

#if BROADCAST_INSTEAD == ON
  ERRCHECK((socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)), "Error Opening Socket!", false);
  addr_.SetAddress(192, 168, 1, 255, addr_.GetPort());
#else  
  ERRCHECK((socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)), "Error Opening Socket!", false);
#endif

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = localInterface.GetAddress();
  address.sin_port = htons(localInterface.GetPort());

  return OpenMulticastHelper(address);
}

bool Connection::IsOpen()
{
  return open_;
}

bool Connection::IsUDP()
{
  return UDP_;
}

u16 Connection::GetAckBits()
{
  return packetsRecieved_;
}

u32 Connection::GetLastRecievedSeq()
{
  return lastSeqRecieved_;
}

u16 Connection::GetDestinationAcks()
{
  return destAcks_;
}

u08  Connection::GetAckReferenceNumber()
{
  return ackRefNum_;
}

void Connection::Close()
{
  if(multicast_ && listen_)
  {
    ip_mreq multicastStruct;
    multicastStruct.imr_multiaddr.S_un.S_addr = addr_.GetAddress();
    multicastStruct.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
    
    setsockopt(socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&multicastStruct, sizeof(multicastStruct));
  }

#if PLATFORM == PLAT_WIN
  closesocket(socket_);
#else
  close(socket_);
#endif
}

void Connection::Terminate()
{
  LOG("Terminating connection " << addr_);
  sendFin_ = true;
}

Socket& Connection::GetSocket()
{
  return socket_;
}

Connection* Connection::Accept()
{
  if(!listen_ || UDP_)
    return nullptr;

  struct sockaddr_in clientAddr;
  socklen_t socklen = sizeof(struct sockaddr);
  Socket acceptedSock = accept(socket_, (struct sockaddr*)&clientAddr, &socklen);
  ERRCHECK(acceptedSock, "Accept Failed", nullptr);
#if PLATFORM==PLAT_WIN
  if(WSAGetLastError() == WSAEWOULDBLOCK)
    return nullptr;
#endif
  if(acceptedSock == ACCEPT_ERROR) 
  {
    //perror("accept failed");
    return nullptr;
  }

  Address addr(clientAddr.sin_addr.S_un.S_addr, clientAddr.sin_port);

  return new Connection(addr, acceptedSock, false);
}

u08 Connection::Send(const c08* data, s32 dataSize, u08 flags)
{
  return SendWith(socket_, data, dataSize, flags);
}

u08 Connection::SendWith(Socket& s, const c08* data, s32 dataSize, u08 flags, s32 seqNum)
{
  if(DroppedPacket() == false)
  {
    if(seqNum != -1)
    {
      DPRINT(2, "Re-Sending Packet " << seqNum << " as " << (u32)nextSequenceNum_);
    }

    s32 newSize = dataSize;
    c08* dataWithHeader = AddHeaderToData(data, newSize, flags);
    BaseSend(s, addr_, dataWithHeader, newSize);
    delete dataWithHeader;
  }
  else
  {
    if(seqNum == -1)
    {
      DPRINT(1, "Simulated Dropped Packet. Chance " << packetLoss_ << "% " << (u32)nextSequenceNum_);
    }
    else
    {
      DPRINT(1, "Simulated Dropped Resend Packet for " << seqNum << " as " << (u32)nextSequenceNum_);
    }
  }
  
  return nextSequenceNum_++;
}

s32 Connection::Recieve(void* data, s32 size, Address& fromAddr)
{
  sockaddr_in from;
  socklen_t fromLength = sizeof(from);

  assertion(data != nullptr);

  s32 rec;
  if(UDP_)
  {
    rec = recvfrom(socket_, (char*)data, size,
                   0, (sockaddr*)&from, &fromLength);
  }
  else
  {
    rec = recv(socket_, (char*)data, size, 0);
  }

  //no data to receive
  if(rec < 0)
    return rec;
  
  if(UDP_)
  {
    u32 addr = ntohl(from.sin_addr.s_addr);
    u16 port = ntohs(from.sin_port);
    fromAddr.SetAddress(addr, port);
  }
  else
  {
    fromAddr = addr_;
  }

  return rec;
}

s32 Connection::ReadHeader(Encoder& decode)
{
  u08 flags, seq;
  
  //Update the disconnect timer
  timeLastReceived_ = GetTickCount();

  decode.Read(flags, (u08)0, BW_7);
  decode.Read(seq);
  decode.Read(ackRefNum_);
  decode.Read(destAcks_);

  //convert sequence to wrapped value
  u32 wrappedSeq = ConvertSequenceNumber(seq);
  u16 packetCount = wrappedSeq - lastSeqRecieved_;
  FLAG_PRINT(addr_ << " Current Seq: " << lastSeqRecieved_ << " Recieved Seq: " << wrappedSeq);
  if(wrappedSeq > lastSeqRecieved_)
  {
    lastSeqRecieved_ = wrappedSeq;
    FLAG_PRINT(addr_ << " Acks Before: " << std::bitset<16>(packetsRecieved_));
    do
    {
      //Count number of dropped packets
      if(packetsRecieved_ > 1 && (~packetsRecieved_ && 1))
      {
        uniqueRatioPacketLoss_ *= packetsRecieved_;
        uniqueRatioPacketLoss_ += 1;
        uniqueRatioPacketLoss_ /= packetsRecieved_ + 1;
      }
      //this while loop will move the acks over
      packetsRecieved_ = (packetsRecieved_ >> 1);
    }while(--packetCount > 0);
  
    packetsRecieved_ |= 0x8000; //set most significant bit
    
    FLAG_PRINT("Acks After:  " << std::bitset<16>(packetsRecieved_));
  }

  //Test to see if we already have it/it is still relevant
  //Essentially delayed duplicate handling
  u32 dseq = lastSeqRecieved_ - wrappedSeq;
  if(dseq > 15)//Useless old information
  {
    FLAG_PRINT(addr_ << " Stale packet [" << lastSeqRecieved_ << ", " << wrappedSeq << "]");
    return -1;
  }
  else if(~packetsRecieved_ & (0x8000 >> (dseq)))//New information I missed
  {
    //I need this packet! I missed it
    FLAG_PRINT(addr_ << " Found a missed packet " << wrappedSeq);
    packetsRecieved_ |= 1 << (15 - dseq);
  }
  else if(dseq != 0)//If It isnt the current packet and I didnt miss it, throw it away
  {
    FLAG_PRINT(addr_ << " Duplicate packet " << wrappedSeq);
    return -1;
  }

  //FLAG_PRINT(addr_ << " Sequence Number: " << wrappedSeq);

  HandleReceiveFlags(flags);

  if(flags & PHF_RSN)
  {
    DPRINT(0, "Resent packet");
    return ReadResentPacketHeader(decode);
  }
    
  return wrappedSeq;
}

void Connection::TestSequenceWrap()
{
  lastSeqRecieved_ = 200;
  u32 testNum = 255;

  u32 ret = ConvertSequenceNumber(testNum);
  assertion(ret == testNum);

  lastSeqRecieved_ = 250;
  testNum = 256;
  ret = ConvertSequenceNumber(testNum);
  assertion(ret == testNum);

  lastSeqRecieved_ = 340;
  testNum = 400;
  ret = ConvertSequenceNumber(testNum);
  assertion(ret == testNum);

  lastSeqRecieved_ = 450;
  testNum = 500;
  ret = ConvertSequenceNumber(testNum);
  assertion(ret == testNum);

  lastSeqRecieved_ = 600;
  testNum = 700;
  ret = ConvertSequenceNumber(testNum);
  assertion(ret == testNum);
  
  for(u32 i = 0; i < 100000; ++i)
  {
    lastSeqRecieved_ = i;
    testNum = i + 10;
    ret = ConvertSequenceNumber(testNum);
    assertion(ret == testNum);

    lastSeqRecieved_ = i + 10;
    testNum = i;
    ret = ConvertSequenceNumber(testNum);
    assertion(ret == testNum);
  }
}

///////////////////
////private
///////////////////
bool Connection::OpenMulticastHelper(sockaddr_in& address)
{
  const c08 on = 1;
  const c08 off = 0;
  u08 ttl = 3;

#if BROADCAST_INSTEAD == ON
  ERRCHECK(setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)), "Setting SO_BROADCAST", false);
  if(listen_)
  {
    ERRCHECK(bind(socket_, (const sockaddr*)&(address), sizeof(sockaddr_in)), "Error Binding Socket", false);
  }
#else
  ip_mreq multicastStruct;
  in_addr localAddr;
  localAddr.s_addr = address.sin_addr.s_addr;

  multicastStruct.imr_multiaddr.S_un.S_addr = addr_.GetAddress();
  multicastStruct.imr_interface = localAddr;
  
  Address dbg(address.sin_addr.S_un.S_addr, ntohs(address.sin_port));
  if(listen_)
  {
    DPRINT(0, "Opening multicast listener for " << addr_ << " on " << dbg);
    
    ERRCHECK(bind(socket_, (const sockaddr*)&(address), sizeof(sockaddr_in)), "Error Binding Socket", false);
   
     /*Enable SO_REUSEADDR to allow multiple instances of this
     application to receive copies of the multicast datagrams.*/
    ERRCHECK(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)),
      "Setting SO_REUSEADDR", false);
    
    ERRCHECK(setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multicastStruct, sizeof(multicastStruct)),
      "Error joining multicast group", false);

  }
  else
  {
    Address dbg(address.sin_addr.S_un.S_addr, ntohs(address.sin_port));
    DPRINT(0, "Opening multicast sender - Sending to " << addr_ << " on " << dbg);

    ERRCHECK(setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localAddr, sizeof(localAddr)),
      "Error setting MULTICAST_IF", false);

    //default ttl is 1 to make sure it can't get out of LAN
    ERRCHECK(setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)),
      "Error setting MULTICAST_TTL", false);

    ERRCHECK(setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off)), 
      "Error Setting MULTICAST_LOOP", false);
  }
#endif

  SetNonBlocking();

  open_ = true;
  return true;
}


void Connection::SetNonBlocking(bool dontBlock /* = true */)
{
#if PLATFORM == PLAT_APPLE || PLATFORM == PLAT_UNIX
  int nonBlocking = 1;
  if(fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
  {
    listen_ = false;
    LOG("Error in ioctlsocket");
    return false;
  }
#elif PLATFORM == PLAT_WIN
  DWORD nonBlocking = 1;
  ERRCHECK(ioctlsocket(socket_, FIONBIO, &nonBlocking), "Error in ioctlsocket", );
#endif
}

void Connection::BaseSend(Socket& s, Address& addr, const c08* data, s32 dataSize)
{
  //we dont want to send heartbeats over multicast
  if(multicast_ && listen_)
    return;
  
  DPRINT(2, "Sending Packet " + std::to_string((u32)nextSequenceNum_) + "     Socket: " + std::to_string(s) + "   IP: " + addr.GetStringAddress());

  int sent = sendto(s, (char*)data, dataSize, 0, 
                    (sockaddr*)&addr.GetSendToAddress(), sizeof(sockaddr_in));
  if(sent != dataSize && !IsUDP())
  {
    DPRINT(1, "Failed to send packet: return = " << sent);
    Terminate();
  }
}

bool Connection::DroppedPacket(void)
{
  if(!UDP_ || multicast_)
    return false;

  f32 randPercent = (rand() / SCAST(f32, RAND_MAX)) * 100.0f;
  return randPercent <= packetLoss_;
}

u32 Connection::ConvertSequenceNumber(u08 seq)
{
  static const u08 LowerLimit = 50;
  static const u08 UpperLimit = 200;
  static const u16 U08_MAX = 256;

  u32 wrapCount = lastSeqRecieved_ / U08_MAX;
  u08 lastAsU08 = (u08)lastSeqRecieved_;// + wrapCount;
  //make sure we wrap correctly if we missed some packets

  //if current is small and we are large and we are not at the wrap number, then current is wrapped further than us
  if(seq < LowerLimit && lastAsU08 > UpperLimit/* && lastAsU08 < (U08_MAX - wrapCount)*/)
    ++wrapCount;
  //if current is large but smaller than us and we are at the wrap number, then we wrapped one too many times
  else if(seq > UpperLimit && seq < lastAsU08 && (lastSeqRecieved_ % U08_MAX) == 0)
    --wrapCount;
  //if current is large and larger than us
  else if(seq > UpperLimit && seq > lastAsU08)
  {
    //if we are small then current is not as wrapped as us
    if(lastAsU08 < LowerLimit)
      --wrapCount;
    //if current is at or past wrap num it is more wrapped than us
    //else if(seq >= U08_MAX - wrapCount)
    //  ++wrapCount;
  }
  
  //u08 oneBasedSeq = seq + wrapCount;
  u32 wrapped = seq + U08_MAX * wrapCount;
  
  DPRINT(2, "Wrapped " << (u32)seq << " to " << wrapped << " with last " << lastSeqRecieved_);
  return wrapped;
}

c08* Connection::AddHeaderToData(const c08* data, s32& dataSize, u08 flags, s32 seqNumOverride)
{
    ////TODO: FIX THIS HEADERSIZE 
  //FLAGS + SEQ + ACKS
  /* 
    // if packing, it would be something like this
  Encoder encode(false);
  encode.StartWriting();
  encode.Write(flags, SCAST(u08,0), SCAST(u08, BW_6));
  encode.Write(nextSequenceNum_);
  encode.Write(lastRecvdSeqNum_); Reference point for acks
  encode.Write(packetsRecieved_);
  encode.WriteData(data, dataSize); // tac on rest of packet (i.e rest of messages)
  encode.DoneWriting();
  */
  const u16 headerSize = PACKET_HEADER_SIZE;
  u16 newSize = dataSize + headerSize;
  c08* dataWithHeader = new c08[newSize];

  HandleSendFlags(flags);

  Encoder encode(false, (u08*)dataWithHeader, newSize);
  encode.Write(flags, SCAST(u08, 0), SCAST(u08, BW_7)); // casting because it complains they are ints
  if(seqNumOverride == -1)//Override sequence number for resent packets
  {
    encode.Write(nextSequenceNum_);
  }
  else
  {
    encode.Write((u08)seqNumOverride);
    seqNumOverride = -1;
  }

  encode.Write((u08)lastSeqRecieved_);
  encode.Write(packetsRecieved_);
  encode.WriteData(data, dataSize);

  dataSize += headerSize;
  return dataWithHeader;
}

void Connection::HandleSendFlags(u08& flags)
{
  //If we are passed a heart beat packet, start the heartbeat
  if(flags & PHF_HRT)
  {
    timeSent_ = GetTickCount();
  }

  //If we received a heart beat, echo it back
  if(sendHeartBeat_)
  {
    flags |= PHF_HRT | PHF_RST;
    sendHeartBeat_ = false;
  }

  if(sendSyn_)
  {
    flags |= PHF_SYN | PHF_RST;
    sendSyn_ = false;
  }

  if(sendFin_)
  {
    flags |= PHF_FIN;
  }

  if(flags & PHF_FIN && !terminateConnection_)
  {
    terminateConnection_ = true;//Blow up the connection if I send a fin packet
  }

  if(startConnection_)
  {
    flags |= PHF_SYN;
  }
}

void Connection::HandleReceiveFlags(u08 flags)
{
  if(flags & PHF_FIN)
  {
    //Terminate the connection
    terminateConnection_ = true;
  }
  
  if(flags & PHF_SYN && 
     flags & PHF_RST)//Got the syns ack
  {
    startConnection_ = false;
  }
  else if(flags & PHF_SYN)
  {
    sendSyn_ = true;
  }
  
  if(flags & PHF_HRT && 
     flags & PHF_RST)
  {
    f32 bias = 0.7f;
    ping_ = (ping_ * bias) + (1.0f - bias) * (GetTickCount() - timeSent_);
   
    FLAG_PRINT(addr_ << " Ping Time: " << ping_);
  }
  else if(flags & PHF_HRT) //heartbeat
  {
    sendHeartBeat_ = true;
  }
}

s32 Connection::ReadResentPacketHeader(Encoder& decode)
{
  u08 flags, seq;

  decode.Read(flags, (u08)0, BW_7);
  decode.Read(seq);
  decode.Read(ackRefNum_);
  decode.Read(destAcks_);

  //convert sequence to wrapped value
  u32 wrappedSeq = ConvertSequenceNumber(seq);
  u16 packetCount = wrappedSeq - lastSeqRecieved_;
  FLAG_PRINT(addr_ << " Current Seq: " << lastSeqRecieved_ << " Recieved Resent Seq: " << wrappedSeq);

  //if this packet should be acked, add it to the ack field
  HandleReceiveFlags(flags);

  if(flags & PHF_RSN)
  {
    DPRINT(0, "This Resent packet contains another resent packet -- THIS SHOULD NEVER HAPPEN");
  }

  return wrappedSeq;
}

u32 Connection::TimeSinceLastSend()
{
  return GetTickCount() - timeLastReceived_;
}

bool Connection::DisconnectTimeOut()
{
  if(TimeSinceLastSend() > disconnectWaitTime_)
  {
    DPRINT(0, "Connection timed out");
    
    DPRINT(1, "Current Seq: " << lastSeqRecieved_);
    DPRINT(1, "Acks:  " << std::bitset<16>(packetsRecieved_));
    DPRINT(1, "Ack Ref Num:  " << (u32)ackRefNum_);

    terminateConnection_ = true;
  }
  return TimeSinceLastSend() > disconnectWaitTime_;
}

bool Connection::ConnectionTerminated()
{
  return terminateConnection_;
}

f32 Connection::GetPacketLossRatio()
{
  return uniqueRatioPacketLoss_;
}