#include "NetworkingPrecompiled.h"
#include "IProtocol.h"


namespace Networking
{

///////////////////////////////////////////////////////////////////////////////
////  Private functions meant only for the networking system to call
///////////////////////////////////////////////////////////////////////////////
void IProtocol::AddCon(const Socket& to)
{
  messages_[to] = new LockableResource<IOQueues>();
}

void IProtocol::RemoveCon(const Socket& to)
{
  for(auto it : (*messages_[to])->send)
  {
    delete it;
  }
  for(auto it : (*messages_[to])->recv)
  {
    delete it;
  }

  delete messages_[to];
}

int IProtocol::SendNextMessage(Socket& con)
{
  Message* msg;
  s32 sizeLeft = 0;

  if(messages_[con] == 0)
    return 0;

  (*messages_[con]).Lock();
  if((*messages_[con])->send.size() > 0)
    msg = (*messages_[con])->send.front();
  else
  {
    (*messages_[con]).Unlock();
    return 0;
  }

  if(msg->size == -1)//Disconnect Message
  {
    (*messages_[con]).Unlock();
    return -2;//Disconnect
  }

  s32 err = send(con, (char*)msg->data, msg->size, 0);
  (*messages_[con]).Unlock();

  if(err == SOCKET_ERROR || err != msg->size)
  {
    return -1;
  }

  //Sent successfully
  (*messages_[con]).Lock();
  (*messages_[con])->send.pop_front();
  sizeLeft = (*messages_[con])->send.size();
  (*messages_[con]).Unlock();

  return sizeLeft;
}

int IProtocol::ReceiveNextMessage(Socket& con)
{
  //Probe for size of packet
  const int maxSize = 65535;
  char temp[maxSize];
  
  (*messages_[con]).Lock();
  s32 err = recv(con, temp, maxSize, MSG_PEEK);
  s32 err2 = WSAGetLastError();
  (*messages_[con]).Unlock();

  if(err == SOCKET_ERROR)
  {
    if(err2 == WSAEWOULDBLOCK)
      return 0;
    return -1;// ERROR
  }

  //Otherwise, err is the size to get
  //No chance of getting error here, since we already know there is something to get
  c08* buff = new c08[err];

  (*messages_[con]).Lock();
  s32 size = recv(con, buff, err, 0);
  (*messages_[con]).Unlock();
  if(size == SOCKET_ERROR)
  {
    delete[] buff;
    return -1;//Something went really really wrong, like using the socket for send and recv or somthing
  }
  
  //Add the message to message queue
  Message* msg = new Message();
  msg->data = buff;
  msg->size = size;
  
  (*messages_[con]).Lock();
  (*messages_[con])->recv.push_back(msg);
  (*messages_[con]).Unlock();

  return size;
}

int IProtocol::ProcessMessages(Socket& socket)
{ 
  if(messages_[socket] == NULL)
    return 0;

  (*messages_[socket]).Lock();
  s32 msgNum = 0;
  
   msgNum = (*messages_[socket])->recv.size();
  
  (*messages_[socket]).Unlock();
  

  for(s32 i = 0; i < msgNum; ++i)
  {
    Message* msg;
    (*messages_[socket]).Lock();


    msg = (*messages_[socket])->recv.front();
    (*messages_[socket]).Unlock();

    s32 err = Process(msg->data, msg->size, socket);

    (*messages_[socket]).Lock();
    (*messages_[socket])->recv.pop_front();
    (*messages_[socket]).Unlock();

    delete msg;
    
    if(err != 0)
      return err;
  }  

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
////  Public functions for the children of this class
///////////////////////////////////////////////////////////////////////////////
IProtocol::~IProtocol()
{
  for(auto iter = messages_.begin(); iter != messages_.end(); ++iter)
  {
    RemoveCon(iter->first);
  }

}

void IProtocol::AddToSendQueue(const c08* bytes, u32 size, Socket& to)
{
  Message* msg = new Message();
  //Copy data over
  msg->size = size;
  msg->data = new c08[size]; 
  for(u32 i = 0; i < size; ++i)
    msg->data[i] = bytes[i];

  
  (*messages_[to]).Lock();
  (*messages_[to])->send.push_back(msg);
  (*messages_[to]).Unlock();
}


void IProtocol::DisconnectWhenDone(Socket& to)
{
  
  Message* msg = new Message();
  //max size is diconnect flag
  msg->size = -1;
  msg->data = nullptr;
  
  (*messages_[to]).Lock();
  (*messages_[to])->send.push_back(msg);
  (*messages_[to]).Unlock();
}

};//~Networking namespace