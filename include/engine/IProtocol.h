#pragma once
//This file defines the interface for a custom ip protocol
#include "CommonDefines.h"
#include "Connection.h"
#include <list>
#include <unordered_map>
#include "LockableResource.h"

namespace Networking
{
class IProtocol
{
private:
  friend class NetworkingSystem;
  
  struct Message
  {
    c08* data;
    u32 size;

    Message() : data(nullptr)
    {}

    ~Message()
    {
      if(data)
        delete[] data;
    }
  };

  struct IOQueues
  {
    std::list<Message*> send;
    std::list<Message*> recv;
  };

  typedef std::map<Socket, LockableResource<IOQueues>*> MsgQueue;

  //list of all messages sent via protocol
  MsgQueue messages_;
  
  //Destroys a connection
  void AddCon(const Socket& to);
  void RemoveCon(const Socket& to);

  //Sends the  next message from the queue
  //Returns messages left in queue
  int SendNextMessage(Socket& con);

  //Takes new messages and puts them in the recv queue
  //Returns size of message
  int ReceiveNextMessage(Socket& con);

  //Calls process messages asynchronously
  //Return error codes
  int ProcessMessages(Socket& socket);
protected:
  //Used to communicate what data to send back
  //Is copied into the queue, safe to delete afterwards
  void AddToSendQueue(const c08* bytes, u32 size, Socket& to);//Call this function in process to send a reply

  //Disconnects from a socket connections
  void DisconnectWhenDone(Socket& to);
public:
  //Because inheritance
  virtual ~IProtocol();

  //Called whenever a message is received on the protocols connection
  //Return error code
  virtual int Process(c08* bytes, u32 size, Socket& from) = 0;
  
};

};//~Networking