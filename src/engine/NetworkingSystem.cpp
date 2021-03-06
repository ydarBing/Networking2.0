#include "NetworkingPrecompiled.h"
#include "NetworkingSystem.h"
#include "Connection.h"

#include "ArgumentParser.h"

#include "Encoder.h"
#include <fstream>//readconfig

#define LOCK_CON(index)   (*lrConnections_)[index]->Lock()
#define UNLOCK_CON(index) (*lrConnections_)[index]->Unlock()
#define CON(index)        (*(*lrConnections_)[index])

Logger* logger = new Logger();

static const s32 MAX_SAFE_PACKET_LEN = 65507;

void DefaultNewTCPConnectionFunc(s32 connectionID)
{
}

void DefaultNewUDPConnectionFunc(s32 connectionID)
{
}

void DefaultHandleDisconnectFunc(s32 connectionID)
{
}

s32 Networking::NetworkingSystem::timeHeartBeatWait_ = 66;
u08 Networking::NetworkingSystem::debugPrint_ = 0;

namespace Networking
{

/************************************************************************/
/*
  Defining public functions for connection data class
*/
/************************************************************************/
NetworkingSystem::ConnectionData::ConnectionData(Address& addr) :
  con_(addr), conID_(-1), timeLastHeartBeat_(0), lastSeqTaken_(1), receiveInOrder(false)
{}
        

NetworkingSystem::ConnectionData::ConnectionData(Connection& con) :
  con_(con), conID_(-1), timeLastHeartBeat_(0), lastSeqTaken_(1), receiveInOrder(false)
{
}

u08 NetworkingSystem::ConnectionData::RemoveAckedMessages(u08 refNum, u16 acks)
{
  u08 remCount = 0;
  auto iter = messagesSent_.begin();
  while(iter != messagesSent_.end())
  {
    u08 deltaSeqNum = (refNum - iter->ackNum_);//Still accurate, even after wrap
    
    if(deltaSeqNum == 0 || acks & (0x8000 >> deltaSeqNum))//If the ack flag for this packet is ticked
    {
      DPRINT(1, con_.GetAddress() << " Packet sent successfully: " << (u32)iter->seqNum_);
      
      //Remove it
      auto toRem = iter;
      ++iter;
      messagesSent_.erase(toRem);
      ++remCount;
      continue;
    }

    ++iter;
  }

  return remCount;
}

void NetworkingSystem::ConnectionData::JoinMessages()
{
  u32 size = 0;
  for(auto it = messagesToJoin_.begin(); it != messagesToJoin_.end(); ++it)
  {
    size += it->GetSize();
  }

  std::sort(messagesToJoin_.begin(), messagesToJoin_.end(), std::less<Message>());

  c08* data = new c08[size];
  c08* dataHead = data;
  for(auto it = messagesToJoin_.begin(); it != messagesToJoin_.end(); ++it)
  {
    memcpy(data, it->GetBuffer(), it->GetSize());
    data += it->GetSize();
  }

  Message toPush(dataHead, size);
  toPush.SetSequence(messagesToJoin_[0].GetSequence());
  messagesToJoin_.clear();
  messagesReceived_.push(toPush);
  delete[] dataHead;
}

/************************************************************************/
/*
  Defining public functions for Custom Protocol Calss
*/
/************************************************************************/
NetworkingSystem::CustomProtocolData::CustomProtocolData(u16 port) :
  protocol(NULL),
  connections(NULL)
{
  //Create the listener socket
  Initialize(port);
}

int NetworkingSystem::CustomProtocolData::Initialize(u16 port)
{
  connections = new LockableResource<std::list<Socket>>();
  (*connections)->clear();
  //Custom protocols always use TCP
  u16 socketType = 0;
  u16 sockProto = 0;
  sockaddr_in address;
  address.sin_family = AF_INET;
   
  socketType = SOCK_STREAM;
  sockProto = IPPROTO_TCP;

  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  
  ERRCHECK((listener = socket(AF_INET, socketType, sockProto)), "Error Opening Socket!", false);

  ERRCHECK(bind(listener, (const sockaddr*)&(address), sizeof(sockaddr_in)), "Error Binding Socket", false);  
  

  DWORD nonBlocking = 1;
  ERRCHECK(ioctlsocket(listener, FIONBIO, &nonBlocking), "Error in ioctlsocket", false);

  ERRCHECK(listen(listener, 5), "Error on listen", false);

  return 0;
}

NetworkingSystem::CustomProtocolData::~CustomProtocolData()
{
  //Destroy all connections
  connections->Lock();
  for(auto iter = (*connections)->begin(); iter != (*connections)->end(); ++iter)
  {
    shutdown(*iter, SD_BOTH);
    closesocket(*iter);
  }
  (*connections).Unlock();

  delete connections;
  //Destroy the listener socket
  closesocket(listener);
  
  //Destroy the protocol pointer
  if(protocol != NULL)
    delete protocol;
}

//Returns number of accepted connections
//Negative if err code
int NetworkingSystem::CustomProtocolData::AcceptConnections(void)
{
  int accepted = 0;
  bool contin = false;
  sockaddr_in newconaddr;
  s32 size = sizeof(sockaddr_in);

  Socket newcon; 
  s32 lastErr = 0;
  do
  {
    newcon = accept(listener, (sockaddr*)&newconaddr, &size);
    lastErr = WSAGetLastError();
    if(newcon != INVALID_SOCKET)
    {
      ++accepted;

      connections->Lock();
      (*connections)->push_back(newcon);
      protocol->AddCon(newcon);
      connections->Unlock();

    }
    else
      contin = false;

  }while(contin);

  if(newcon == INVALID_SOCKET)
  {
    if(lastErr != WSAEWOULDBLOCK)
      return -1;
  }
  //If wouldblocked or completed some
  //Return number completed
  return accepted;
}

int NetworkingSystem::CustomProtocolData::RemoveConnection(Socket con)
{
  //Find socket
  std::list<Socket>::iterator found;
  bool done = true;
  for(auto iter = (*connections)->begin(); iter != (*connections)->end(); ++iter)
  {
    if(*iter == con)
    {
      found = iter;
      done = false;
    }
  }

  if(done)
    return -1;//socket not found

  //Shutdown
  shutdown(*found, SD_BOTH);
  closesocket(*found);

  //Erase
  connections->Lock();
  (*connections)->erase(found);
  connections->Unlock();

  return 0;
}


/************************************************************************/
/*
  Defining public functions for networking system
*/
/************************************************************************/
//For some reason, WSA startup returns it exact error code,
//While WSACleanup relies upon WSAGetLastError
NetworkingSystem::NetworkingSystem(bool acceptConnections, 
                                   NewTCPConnectionFunc newTCPConnectFunc,
                                   NewUDPConnectionFunc newUDPConnectFunc,
                                   HandleDisconnectFunc handleDisconnectFunc) :
  tBetweenUpdates_(16),
  bQuit_(false),
  acceptConnections_(acceptConnections),
  tWaitBeforeResend_(80),
  numConnections_(0),
  newTCPConnectFunc_(newTCPConnectFunc),
  newUDPConnectFunc_(newUDPConnectFunc),
  handleDisconnectFunc_(handleDisconnectFunc),
  maxPacketLen_(4000),
  multicast_(nullptr),
  latency_(0)
{
  if(!InitializeSockets())
  {
    PushError(NetworkingSystemError::WSA_STARTUP_FAILURE);
    return;
  }
}

NetworkingSystem::~NetworkingSystem()
{
  delete logger;

  bQuit_ = true;
  updateThread_.join();
  recvThread_.join();
  custProtoUpdateThread_.join();
  custProtoRecvThread_.join();


  for(u32 i = 0;i < conCustomProtocols_->size();++i)
  {
    delete **(*conCustomProtocols_)[i];
    delete (*conCustomProtocols_)[i];
  }  

  //Close/delete all connections
  for(u32 i = 0; i < lrConnections_->size(); ++i)
  {
    if((*lrConnections_)[i] != NULL)
    {
      CON(i)->con_.Close();
      delete (*lrConnections_)[i];
    }
  }
  //Clean up the vector
  lrConnections_->clear();

  tcpAccept_.Close();
  udpConnection_.Close();

#if PLATFORM == PLAT_WIN
  //Cleanup winsock
  s16 result = WSACleanup();

  if(result == SOCKET_ERROR)
  {
    PushError(NetworkingSystemError::WSA_CLEANUP_FAILURE);
  }
#endif

  delete multicast_;
}

void NetworkingSystem::SetHandleDisconnectFunc(HandleDisconnectFunc func)
{
  handleDisconnectFunc_ = func;
}

void NetworkingSystem::SetNewTCPConnectionFunc(NewTCPConnectionFunc func)
{
  newTCPConnectFunc_ = func;
}

void NetworkingSystem::SetNewUDPConnectionFunc(NewUDPConnectionFunc func)
{
  newUDPConnectFunc_ = func;
}

void NetworkingSystem::SetLag(u32 time)
{
  latency_ = time;
}

u32 NetworkingSystem::GetLag(void)
{
  return latency_;
}

int NetworkingSystem::Initialize(const ArgumentParser& parser)
{
  int connection = -1;
  TODO("ANNOUNCE VERSION");
  bool isServer = parser.Argument("--server") || parser.Argument("-s");
  acceptConnections_ |= isServer;//Set to true if not true, otherwise leave default
  
  // check if there was a config file given
  istring hasConfig = parser.GetFollowingToken("--config");
  if(hasConfig.empty())
    hasConfig = parser.GetFollowingToken("-C");

  Initialize();

  return connection;
}

void NetworkingSystem::Initialize()
{
  updateThread_ = std::thread(&NetworkingSystem::ThreadUpdate, this);
  recvThread_ =   std::thread(&NetworkingSystem::ThreadRecieve, this);
  custProtoUpdateThread_ = std::thread(&NetworkingSystem::ThreadCustomProtocolUpdate, this);
  custProtoRecvThread_ = std::thread(&NetworkingSystem::ThreadCustomProtocolReceive, this);
}

s32 NetworkingSystem::Connect(Address con, bool udp, u16 localPort)
{
  LockableResource<ConnectionData>* cd = new LockableResource<ConnectionData>(con);

  //only tcp because udp uses one (udpConnection_) socket to send and receive
  if(!udp)
  {
    if(!(*cd)->con_.Open(localPort, false))
    {
      PushError(NetworkingSystemError::CONNECTION_OPEN_FAILURE);
      return NetworkingSystemError::CONNECTION_OPEN_FAILURE;
    }
  }

  //Return negative error codes
  //Find a place for the connection in the vector
  u32 id = 0;

  //Adding a connection locks the entire queue until it has found its place
  //Could potentially be very expensive
  TODO("Figure out a lockeless/faster solution");
  lrConnections_.Lock();
  {
    for(; id < lrConnections_->size(); ++id)
    {
      //If there is an open space for the connection, 
      //Place it there
      if((*lrConnections_)[id] == NULL)
      {
        (*cd)->conID_ = id;
        (*lrConnections_)[id] = cd;
        break;
      }
    }

    //if there is no open space for the connection
    //Add it to the back of the vector
    if(id == lrConnections_->size())
    {
      (*cd)->conID_ = id;
      lrConnections_->push_back(cd);
    }
  }
  lrConnections_.Unlock();
  ++numConnections_;
  return id;
}

s32 NetworkingSystem::Multicast(Address& localAddr, Address con, bool _listen /* = true */)
{
  if(multicast_ == nullptr)
  {
    multicast_ = new Connection(con);
  }
  else
  {
    multicast_->SetAddress(con);
  }

  if(!multicast_->OpenMulticast(localAddr, _listen))
  {
    PushError(NetworkingSystemError::CONNECTION_OPEN_FAILURE);
    return NetworkingSystemError::CONNECTION_OPEN_FAILURE;
  }

  return MCAST_ID;
}

s32 NetworkingSystem::Send(Message msg, s32 hCon)
{
  if(hCon == MCAST_ID && multicast_ != nullptr)
  {
    c08* data = (c08*)msg.GetSendBuffer();
    multicast_->Send(data, msg.GetSendSize());
    delete[] data;
    return NetworkingSystemError::OK;
  }
  
  if(lrConnections_->size() == 0)
    return NetworkingSystemError::NO_CONNECTION;

  if(msg.GetSize() == 0)
  {
    PushError(NetworkingSystemError::EMPTY_MESSAGE);
    return NetworkingSystemError::EMPTY_MESSAGE;
  }
    
  if((*lrConnections_)[hCon] == NULL)
  {
    PushError(NetworkingSystemError::CONNECTION_INVALID_OPERATION);
    return NetworkingSystemError::CONNECTION_INVALID_OPERATION;
  }

  //Don't need to worry about master queue locking,
  s32 maxSendLen = maxPacketLen_ - (PACKET_HEADER_SIZE + MESSAGE_HEADER_SIZE);
  if(msg.GetSize() > maxSendLen)//If we need to split packets up
  {
    s32 remaining = msg.GetSize();
    for(u32 i = 0; i < msg.GetSize(); i += maxSendLen, remaining -= maxSendLen)
    {
      Message nm((c08*)msg.GetBuffer() + i, min(maxSendLen, remaining), msg.MustSend());//Construct packet of max size
      if(remaining > maxSendLen)
        nm.SetFlags(Message::IS_CONTINUED);
      else
        nm.SetFlags(Message::END_CONTINUE);
      
      LOCK_CON(hCon);
        CON(hCon)->messagesToSend_.push(nm);
      UNLOCK_CON(hCon);
    }
  }
  else//If the whole packet fits!
  {
    LOCK_CON(hCon);
      CON(hCon)->messagesToSend_.push(msg);
    UNLOCK_CON(hCon);
  }
    
  return NetworkingSystemError::OK;
}

s32 NetworkingSystem::NumReceivedInQueue(s32 con)
{
  if(con == MCAST_ID)
  {
    return multicastRecieved_->size();
  }

  if(lrConnections_->size() == 0)
    return -1;

  if((*lrConnections_)[con] == NULL)
  {
    return -1;
  }

  return CON(con)->messagesReceived_.size();
}

Message NetworkingSystem::Receive(s32 hCon)
{
  if(hCon == MCAST_ID)
  {
    if(multicast_ == nullptr || multicastRecieved_->size() == 0)
      return Message(MessageError::EMPTY_MESSAGE);

    multicastRecieved_.Lock();
      Message ret = multicastRecieved_->front();
      multicastRecieved_->pop();
    multicastRecieved_.Unlock();
    return ret;
  }

  if(lrConnections_->size() == 0)
    return Message(MessageError::EMPTY_MESSAGE);
  //Check if connection is null, if it is, shutdown
  if((*lrConnections_)[hCon] == NULL)
  {
    PushError(NetworkingSystemError::CONNECTION_INVALID_OPERATION);
    Message m(MessageError::NETWORK_ERROR);//Give an "error message" back so the user knows something is wrong
    return m;
  }

  Message m;
  LOCK_CON(hCon);
  if(CON(hCon)->messagesReceived_.size() > 0)
  {
    m = CON(hCon)->messagesReceived_.front();
    CON(hCon)->messagesReceived_.pop();
  }
  UNLOCK_CON(hCon);

  return m;
}

s32 NetworkingSystem::Disconnect(s32 hCon)
{
  if(hCon == MCAST_ID)
  {
    multicast_->Close();
    delete multicast_;
    multicast_ = nullptr;
    return NetworkingSystemError::OK;
  }

  //Assert validity of hCon (within reasonable bounds, is not null handle)
  if((*lrConnections_)[hCon] == NULL || hCon < 0)
  {
    PushError(NetworkingSystemError::CONNECTION_INVALID_OPERATION);
    return -1;
  }

  CON(hCon)->con_.Terminate();

  return NetworkingSystemError::OK;
}

void NetworkingSystem::SetBufferSize(s32 size)
{
  if(size < PACKET_HEADER_SIZE * 2 + 1)
    return;
  maxPacketLen_ = min(size, MAX_SAFE_PACKET_LEN);
}

s32 NetworkingSystem::GetBufferSize(void) const
{
  return maxPacketLen_;
}

s32 NetworkingSystem::GetMaxDataSize(void) const
{
  //packet header size is times two because if it's resent it needs to be added again
  return maxPacketLen_ - (PACKET_HEADER_SIZE * 2 + MESSAGE_HEADER_SIZE);
}

NetworkingSystemError::NETWORKING_SYSTEM_ERROR NetworkingSystem::GetNextError(void)
{
  NetworkingSystemError::NETWORKING_SYSTEM_ERROR nerr;
  lrErrorQueue_.Lock();
  if(lrErrorQueue_->size() > 0)
  {
    nerr = lrErrorQueue_->front();
    lrErrorQueue_->pop();
  }
  else
  {
    nerr = NetworkingSystemError::OK;
  }
  lrErrorQueue_.Unlock();
  return nerr;
}

s32 NetworkingSystem::NumConnections()
{
  return lrConnections_->size();
}

void NetworkingSystem::PrintConnections()
{
  lrConnections_.Lock();
  for(u32 i = 0; i < lrConnections_->size(); i++)
  {
    if( ((*lrConnections_)[i]) == NULL)
      continue;
    std::string tcpUdp = CON(i)->con_.IsUDP() ? "UDP " : "TCP ";
    LOCK_CON(i);
      DPRINT(0, i << ": " << tcpUdp << CON(i)->con_.GetAddress());
    UNLOCK_CON(i);
  }
  lrConnections_.Unlock();
}

istring NetworkingSystem::StringifyConnectionInfo(s32 con)
{
  istring str;
  lrConnections_.Lock();
  if(con >= 0 && (*lrConnections_)[con] != NULL)
  {
    str += CON(con)->con_.GetAddress().GetStringAddress() + ":";
    str += std::to_string(CON(con)->con_.GetAddress().GetPort());
    str += "-";
    str +=  std::to_string( unsigned(CON(con)->con_.GetPing()) );
    str += "MS";
    str += " [";
    str += std::to_string( unsigned(CON(con)->con_.GetPacketLossRatio() * 100.0f) );
    str += "% loss]";
  }
  lrConnections_.Unlock();
  return str;
}

unsigned NetworkingSystem::GetConnectionPing(s32 con)
{
  unsigned ping;
  lrConnections_.Lock();
  if(con >= 0 && (*lrConnections_)[con] != NULL)
  {
    ping = unsigned(CON(con)->con_.GetPing());
  }
  else
  {
    ping = -1;
  }
  lrConnections_.Unlock();
  return ping;
}



void NetworkingSystem::SetAcceptConnections(bool allow)
{
  acceptConnections_ = allow;
}
bool NetworkingSystem::GetAcceptConnections(void)const
{
  return acceptConnections_;
}

const Address& NetworkingSystem::GetConnectionAddress(s32 con)
{
  if(con == MCAST_ID)
  {
    return multicastRecieveAddr_;
  }
  return CON(con)->con_.GetAddress();
}

std::list<s32> NetworkingSystem::GetConnections()
{
  std::list<s32> cons;

  lrConnections_.Lock();
  for(auto i = lrConnections_->begin(); i != lrConnections_->end(); ++i)
  {
    if((*i))
      cons.push_back((**i)->conID_);
  }
  lrConnections_.Unlock();

  return cons;
}

/************************************************************************/
/*
  Defining private functions for networking system
*/
/************************************************************************/
bool NetworkingSystem::InitializeSockets()
{
#if PLATFORM == PLAT_WIN
  WSADATA dat;
  ERRCHECK(WSAStartup(MAKEWORD(2, 2), &dat), "Error on WSAStartup", false);
#endif
  return true;
}

void NetworkingSystem::ThreadCustomProtocolUpdate()
{
  while(!bQuit_)
  {
    conCustomProtocols_.Lock();
    u32 size = conCustomProtocols_->size();
    conCustomProtocols_.Unlock();

    for(u32 i = 0;i < size;++i)
    {
      LockableResource<CustomProtocolData*>* proto = (*conCustomProtocols_)[i];
      
      std::list<std::list<Socket>::iterator> toDelete;
      //Should do all of this for every connection
      for(auto iter = (*(**proto)->connections)->begin(); iter != (*(**proto)->connections)->end(); ++iter) 
      {
        bool deleting = false;
        //Process received messages
        (*proto).Lock();
        s32 err = (**proto)->protocol->ProcessMessages(*iter);
        (*proto).Unlock();

        if(err != 0)
        {
          lrErrorQueue_.Lock();
          lrErrorQueue_->push(NetworkingSystemError::CUSTOM_PROTO_PROCESS_FAILURE);
          lrErrorQueue_.Unlock();

          toDelete.push_back(iter);
          deleting = true;
        }
        //Send all messages in queue
        err = 0;
        do
        {
          (*proto).Lock();
          err = (**proto)->protocol->SendNextMessage(*iter);
          (*proto).Unlock();
        }while(err > 0);

        if(err < 0)
        {
          if(err != -2) // If not a disconnect
          {
            lrErrorQueue_.Lock();
            lrErrorQueue_->push(NetworkingSystemError::CUSTOM_PROTO_SEND_FAILURE);
            lrErrorQueue_.Unlock();
          }
          toDelete.push_back(iter);
          deleting = true;
        }
      }

      
      for(auto i = toDelete.begin(); i != toDelete.end(); ++i)
      {
        (**proto)->RemoveConnection(**i);
      }
      
      //Receive new connections here
      //Recv con should be in its own thread, but this will do for now
      s32 err = (**proto)->AcceptConnections();
      if(err < 0)
      {
        lrErrorQueue_.Lock();
        lrErrorQueue_->push(NetworkingSystemError::CUSTOM_PROTO_ACCEPT_FAILURE);
        lrErrorQueue_.Unlock();
      }

      SLEEPMILLI(tBetweenUpdates_);
    }
  }  
}

void NetworkingSystem::ThreadCustomProtocolReceive()
{
  while(!bQuit_)
  {
    conCustomProtocols_.Lock();
    u32 size = conCustomProtocols_->size();
    conCustomProtocols_.Unlock();

    for(u32 i = 0;i < size;++i)
    {
      LockableResource<CustomProtocolData*>* proto = (*conCustomProtocols_)[i];
      
      //Should do all of this for every connection
      (**proto)->connections->Lock();
      for(auto iter = (*(**proto)->connections)->begin(); iter != (*(**proto)->connections)->end(); ++iter) 
      {
        //Recv a single message at a time to avoid starvation of other protocols
        (*proto).Lock();
        s32 err = (**proto)->protocol->ReceiveNextMessage(*iter);
        (*proto).Unlock();
        if(err <  0)
        {
          lrErrorQueue_.Lock();
          lrErrorQueue_->push(NetworkingSystemError::CUSTOM_PROTO_RECV_FAILURE);
          lrErrorQueue_.Unlock();
        }
      }
      (**proto)->connections->Unlock();
    }

    SLEEPMILLI(tBetweenUpdates_);
  }  
}

void NetworkingSystem::ThreadUpdate()
{
  while(!bQuit_)
  {
    //Don't need to lock lrConnection_ since I dont't modify the container directly
    u32 numCons = lrConnections_->size();
    //Loop over every connection
    for(u32 i = 0; i < numCons; ++i)
    {
      LockableResource<ConnectionData>* cd = (*lrConnections_)[i];
      if(cd != NULL)
      {
        cd->Lock();
        {
          //Remove the acked packets
          (*cd)->RemoveAckedMessages((*cd)->con_.GetAckReferenceNumber(), (*cd)->con_.GetDestinationAcks());
  
          UpdateReceive(cd);
          UpdateResend(cd);
          UpdateSend(cd);
          UpdateHeartbeat(cd);
        }
        cd->Unlock();

        //Disconnection
        if((*cd)->con_.ConnectionTerminated() || (*cd)->con_.DisconnectTimeOut())
        {
          handleDisconnectFunc_((*cd)->conID_);
          RemoveConnection((*cd)->conID_);
        }
      }
    } //~for

    //recieve from multicast socket
    if(multicast_ != nullptr)
    {
      RecievedData* data = new RecievedData;
      data->data = new u08[maxPacketLen_];

      data->dataSize = (*multicast_).Recieve(data->data, maxPacketLen_, multicastRecieveAddr_);

      if(data->dataSize > 0)
      {
        Encoder reader(true, data->data, data->dataSize);

        multicast_->ReadHeader(reader);

        multicastRecieved_.Lock();
          ReadMessagesFromBuffer(reader, *multicastRecieved_);
        multicastRecieved_.Unlock();
      }

      delete[] data->data;
      delete data;
    }

    dataToParse_.Lock();
    s32 toParse = dataToParse_->size(); //this is fine, just reading an int
    dataToParse_.Unlock();
    
    while(toParse-- > 0)
    {
      dataToParse_.Lock();
        RecievedData* dat = dataToParse_->front();
        dataToParse_->pop();
      dataToParse_.Unlock();
      
      ParseMessages(dat->data, dat->dataSize, dat->addr);
      
      delete [] dat->data;
      delete dat;
    }

    //this only accepts tcp connections as the connection recieve will catch 
    if(acceptConnections_)
    {
      TCPAccept();
    }

    //Sleep until it should get called next
    SLEEPMILLI(tBetweenUpdates_);//Wait can be changed whenever, defaults to 1 frames
  }//~while
}

bool NetworkingSystem::RecieveHelper(Connection& con, RecievedData* data)
{
  data->dataSize = con.Recieve(data->data, maxPacketLen_, data->addr);

  if(data->dataSize > 0)
  {
    dataToParse_.Lock();
      dataToParse_->push(data);
    dataToParse_.Unlock();
    return true;
  }
  return false;
}

void NetworkingSystem::ThreadRecieve()
{
  RecievedData* data = new RecievedData;
  data->data = new u08[maxPacketLen_];

  while(!bQuit_)
  {
    if(RecieveHelper(udpConnection_, data))
    {
      data = new RecievedData;
      data->data = new u08[maxPacketLen_];
    }
    else
    {
      SLEEPMILLI(1);
    }
  }

  delete [] data->data;
  delete data;
}

void NetworkingSystem::UpdateReceive(LockableConData cd)
{
  if(!(*cd)->con_.IsUDP())
  {
    RecievedData* data = new RecievedData;
    data->data = new u08[maxPacketLen_];
    if(!RecieveHelper((*cd)->con_, data))
    {
      delete [] data->data;
      delete data;
    }
  }
}

void NetworkingSystem::UpdateResend(LockableConData cd)
{
  if((*cd)->messagesSent_.size() != 0)
  {
    LOG("Number of Packets in resend queue: " << (*cd)->messagesSent_.size());
    u32 time = GetTickCount();
    auto iter = (*cd)->messagesSent_.begin();
    while(iter != (*cd)->messagesSent_.end())
    {
      //Check resend timer
      if(time - iter->resendTimer_ >= tWaitBeforeResend_)
      {
        //If timer is past resend time
        //resend packet
        u08* buffer = iter->m_.GetSendBuffer();
        u16 size = iter->m_.GetSendSize();
        u08* sendBuffer = ResendEncapsulate(buffer, size, iter->seqNum_);
        iter->ackNum_ = (*cd)->con_.SendWith(udpConnection_.GetSocket(), (c08*)sendBuffer, size, PHF_RSN, iter->seqNum_);
        delete[] sendBuffer;

        //reset timer
        iter->resendTimer_ = GetTickCount();
      }
      ++iter;
    }
  }
}

void NetworkingSystem::UpdateSend(LockableConData cd)
{
  if((*cd)->messagesToSend_.size() != 0)
  {
    Message m = (*cd)->messagesToSend_.front();
    
    u32 current = GetTickCount();
    u32 mTime = m.GetCreateTime();
    if(current - mTime >= latency_)
    {
      (*cd)->messagesToSend_.pop();
    }
    else
    {
      return;
    }

    u08* sendDat = m.GetSendBuffer();
    if((*cd)->con_.IsUDP())
    {
      u08 seqNum = (*cd)->con_.SendWith(udpConnection_.GetSocket(), (c08*)sendDat, m.GetSendSize());
      //Packet resending needs to be handled around here
      if(m.MustSend())
      {
        NetMessageSent sent;
        sent.seqNum_ = seqNum;
        sent.ackNum_ = seqNum;
        sent.m_ = m;//Encapsulating the message
        sent.resendTimer_ = GetTickCount();//Start the resend timer

        DPRINT(2, "Added " << (u16)seqNum << " to Resend Queue");

        (*cd)->messagesSent_.push_back(sent);//Add to the resend queue
      }
    }
    else
    {
      (*cd)->con_.Send((c08*)sendDat, m.GetSendSize());
    }
    delete[] sendDat;
  }
}

void NetworkingSystem::UpdateHeartbeat(LockableConData cd)
{
  c08 heartbeat = 'h';
  if(GetTickCount() - (*cd)->timeLastHeartBeat_ > timeHeartBeatWait_)//send heartbeat if udp
  {
    (*cd)->timeLastHeartBeat_ = GetTickCount();
    if((*cd)->con_.IsUDP())
    {
      (*cd)->con_.SendWith(udpConnection_.GetSocket(), &heartbeat, 0, PHF_HRT);
    }
    else
    {
      (*cd)->con_.Send(&heartbeat, 0, PHF_HRT);
    }
  }
  else if((*cd)->con_.sendHeartBeat_)
  {
    if((*cd)->con_.IsUDP())
    {
      (*cd)->con_.SendWith(udpConnection_.GetSocket(), &heartbeat, 0);
    }
    else
    {
      (*cd)->con_.Send(&heartbeat, 0, PHF_HRT);
    }
  }
}

void NetworkingSystem::CreateUdpListener(u16 port)
{
  if(udpConnection_.IsOpen())
    udpConnection_.Close();

  if(udpConnection_.Open(port))
  {
    DPRINT(0, "UDP listener open on port " << port);
  }
}

void NetworkingSystem::CreateTcpListener(u16 port)
{
  if(tcpAccept_.IsOpen())
    tcpAccept_.Close();
  if(tcpAccept_.Open(port, false, true))
  {
    DPRINT(0, "TCP listener open on port " << port);
  }
}

void NetworkingSystem::RemoveConnection(s32 hCon)
{
  assertion((*lrConnections_)[hCon] != NULL || hCon > 0)
 
  DPRINT(0, "Disconnected: " << hCon);

  lrConnections_.Lock();
  LOCK_CON(hCon);
  //Close the connection
  CON(hCon)->con_.Close();

  UNLOCK_CON(hCon);
  //Delete the connection
  delete (*lrConnections_)[hCon];
  //Assign NULL to its position so others know the space is free
  (*lrConnections_)[hCon] = NULL;
  lrConnections_.Unlock();
  --numConnections_;
}

void NetworkingSystem::TCPAccept()
{
  Connection* con = tcpAccept_.Accept();
  if(con != nullptr)
  {
    DPRINT(0, "New TCP Connection");

    lrConnections_.Lock();
    {
      lrConnections_->push_back(new LockableResource<ConnectionData>(ConnectionData(*con)));
      (*lrConnections_->back())->conID_ = lrConnections_->size() - 1;
    }
    lrConnections_.Unlock();
    
    //Call the callback function
    newTCPConnectFunc_(lrConnections_->size() - 1);
    ++numConnections_;
  }
}

void NetworkingSystem::ParseMessages(u08* data, s32 dataSize, Address& fromAddr)
{
  LockableResource<ConnectionData>* con = nullptr;
  Encoder reader(true, data, dataSize);
  s32 sequenceNumber = 0;
  for(auto it = lrConnections_->begin(); it != lrConnections_->end(); ++it)
  {
    if(*it != nullptr && fromAddr == (**it)->con_.GetAddress())
    {
      con = (*it);
      sequenceNumber = (**it)->con_.ReadHeader(reader);
    }
  }
  
  if(con == nullptr && multicast_ != nullptr && fromAddr == multicast_->GetAddress())
  {
    sequenceNumber = multicast_->ReadHeader(reader);
    multicastRecieved_.Lock();
      ReadMessagesFromBuffer(reader, *multicastRecieved_, sequenceNumber);
    multicastRecieved_.Unlock();
  }
  else if(con == nullptr)
  {
    //make sure this packet has the syn flag set
    u08 flags;
    reader.Read(flags, (u08)0, BW_7);
    reader.StartReading();
    
    if(flags & PHF_SYN)
    {
      DPRINT(0, "New Connection: " << fromAddr);
      s32 newCon = Connect(fromAddr);
      con = (*lrConnections_)[newCon];
      sequenceNumber = (*con)->con_.ReadHeader(reader);
      ++numConnections_;
      //A new UDP connection, so let's let inteterested parties know
      newUDPConnectFunc_(newCon);
    }
    else
    {
      return;
    }
  }
  
  if(sequenceNumber == -1)//Packet is expired
    return;

  con->Lock();
    ReadMessagesFromBuffer(reader, (*con)->messagesReceived_, sequenceNumber);
  con->Unlock();
}


void NetworkingSystem::ReadMessagesFromBuffer(Encoder& reader, std::queue<Message>& recievedQueue, s32 sequence /* = 0 */)
{
  u16 msgSize;
  u08 msgFlags;
  while(!reader.AtEnd())
  {
    reader.Read(msgFlags);
    reader.Read(msgSize);

    c08* data = reader.ReadData(msgSize);
    
    Message m(data, msgSize);
    m.SetSequence(sequence);

    delete[] data;

    TODO("this again");
    /*if(msgFlags & Message::IS_CONTINUED)
    {
      (*con)->messagesToJoin_.push_back(m);
    }
    else if(msgFlags & Message::END_CONTINUE)
    {
      (*con)->messagesToJoin_.push_back(m);
      (*con)->JoinMessages();
    }
    else
    {*/
      recievedQueue.push(m);
    //}
  }
}


u08* NetworkingSystem::ResendEncapsulate(u08* buffer, u16& size, u08 seq)
{
  s32 buffSize = size;
  c08* ret = udpConnection_.AddHeaderToData((c08*)buffer, buffSize, 0, seq);
  delete[] buffer;
  size = buffSize;
  return (u08*)ret;
}

void NetworkingSystem::PushError(NetworkingSystemError::NETWORKING_SYSTEM_ERROR nseErr)
{
  lrErrorQueue_.Lock();
  lrErrorQueue_->push(nseErr);
  lrErrorQueue_.Unlock();
}

}//~networking


