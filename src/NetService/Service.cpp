#include "MainPrecompiled.h"

#include "NetworkingSystem.h"
#include "Service.h"

#include "stdlib.h"

Service::Service(istring& localIP) : m_connectedToMaster(false)
{
  currentService = this;

  m_net = new Networking::NetworkingSystem(true);
  m_net->Initialize();

  Address mCastAddr(226, 0, 0, 1, 8000);
  TODO("Don't hard-code this");
  Address localAddr(inet_addr(localIP.c_str()), 8000);
  m_net->Multicast(localAddr, mCastAddr);
}

Service::~Service()
{
  delete m_net;
}

void Service::Run(ThreadSafeQueue<istring>& userIO)
{
  while(true)
  {
    ////////////////////
    // All connections receive
    Receive();

    ////////////////////
    // Multicast receive
    Networking::Message m = m_net->Receive(MCAST_ID);
    istring out;
    out.assign(m.GetBuffer(), m.GetSize());
    if(m.GetSize() > 0)
    {
      if(!m_connectedToMaster)
      {
        m_connectedToMaster = true;

        m_net->Connect(
          Address(m_net->GetConnectionAddress(MCAST_ID).GetAddress(), atoi(out.c_str())),
          false, 8009);
      }
    }

    SLEEPMILLI(100);
  }
}

void Service::Receive()
{
  s32 con = m_net->NumConnections();
  while (con-- > 0)
  {
    Networking::Message m = m_net->Receive(con);
    if(m.GetSize() > 0)
    {
      istring command;
      command.assign(m.GetBuffer(), m.GetSize());
      HandleCommand(command);
    }
  }
}

bool Service::HandleCommand(istring& command)
{
  s32 debugPrint_ = 0;
  if(command == "client")
  {
    DPRINT(0, "Opening Client");
    StartProcess("AsteroidsClient.exe");
  }
  else if(command == "serv")
  {
    DPRINT(0, "Opening Server");
    StartProcess("AsteroidsServer.exe");
  }
  else if(command == "test")
  {
    DPRINT(0, "Testing with Calc");
    StartProcess("calc");
  }
  else if(command == "kill")
  {
    DPRINT(0, "Killing all processes");
    for(HANDLE h : m_pIDs)
      TerminateProcess(h, 0);
    m_pIDs.clear();
  }

  return true;
}

void Service::HandleTCPConnect(s32 id)
{

}

void Service::HandleUDPConnect(s32 id)
{

}

void Service::HandleDisconnect(s32 id)
{

}

void Service::StartProcess(LPSTR exeName)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  if (!CreateProcess(
    NULL,
    exeName,
    NULL, NULL, FALSE,
    CREATE_NEW_CONSOLE,
    NULL, NULL,
    &si,
    &pi))
  {
    std::cout << "Unable to execute." << std::endl;
  }
  else
  {
    m_pIDs.push_back(pi.hProcess);
  }

}
