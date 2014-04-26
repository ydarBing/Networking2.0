#pragma once

#include "INetService.h"

class Service : public INetService
{
public:
  Service(istring& localIP);
  ~Service();

  virtual void Run(ThreadSafeQueue<istring>& userIO);

  virtual void Receive();

  virtual bool HandleCommand(istring& command);
  virtual void HandleTCPConnect(s32 id);
  virtual void HandleUDPConnect(s32 id);

  virtual void HandleDisconnect(s32 id);

private:
  void StartProcess(LPSTR exeName);

  std::vector<HANDLE> m_pIDs;

  Networking::NetworkingSystem* m_net;

  bool m_connectedToMaster;
};
