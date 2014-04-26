#pragma once

namespace Networking
{
  class NetworkingSystem;
}

#include "FileShare/io/ThreadSafeQueue.h"
class INetService
{
public:
  virtual void Run(ThreadSafeQueue<istring>& userIn) = 0;
  virtual void Receive() = 0;

  virtual bool HandleCommand(istring& command) = 0;
  
  virtual void HandleTCPConnect(s32 id) = 0;
  virtual void HandleUDPConnect(s32 id) = 0;
  virtual void HandleDisconnect(s32 id) = 0;
};

extern INetService* currentService;