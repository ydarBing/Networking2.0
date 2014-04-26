#pragma once
#include "INetService.h"
#include "Message.h"
//#include "asteroidsCommon\game\MessageIDs.h"

class MasterServer : public INetService
{
public:
  MasterServer(istring& localIP);
  ~MasterServer();

  virtual void Run(ThreadSafeQueue<istring>& userIO);

  virtual void Receive();

  virtual bool HandleCommand(istring& command);

  virtual void HandleTCPConnect(s32 conID);
  virtual void HandleUDPConnect(s32 id);
  virtual void HandleDisconnect(s32 id);

private:
  struct InstanceServer;
  typedef std::vector<InstanceServer*> ServerVec;
  typedef ServerVec::iterator          ServerVecIt;

  typedef std::vector<s32>    ClientVec;
  typedef ClientVec::iterator ClientVecIt;
  
  //Networking::Message CreateNetMessage(MessageID::MessageID id, const c08 *data, const u16 buffSize);
  c08* GetIDAndDataFromMessage(Networking::Message message, s32& id);

  struct InstanceServer
  {
    InstanceServer(s32 id) 
      : connectionID(id), numClients(0), gameRunning(false){}

    s32 connectionID;
    s32 numClients;
    bool gameRunning;
  };
  ServerVec m_instanceServers;
  ClientVec m_clients;

  void RelocateToInstance(s32 con);

  ServerVecIt GetInstanceIt(s32 con);
  ClientVecIt GetClientIt(s32 con);

  Networking::NetworkingSystem* m_net;
  
  u32 m_multicastUpdateTime;
  u32 m_lastUpdateTime;
  u32 m_sleepTime;
};
