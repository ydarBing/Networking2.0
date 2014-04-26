#include "MainPrecompiled.h"

#include "NetworkingSystem.h"
#include "MasterServer.h"
#include "asteroidsCommon\game\MessageIDs.h"

void TCPConnect(s32 id)
{
  currentService->HandleTCPConnect(id);
}

void UDPConnect(s32 id)
{
  currentService->HandleUDPConnect(id);
}

void Disconnect(s32 id)
{
  currentService->HandleDisconnect(id);
}

MasterServer::MasterServer(istring& localIP) : m_sleepTime(200), m_multicastUpdateTime(5000),
  m_lastUpdateTime(0)
{
  currentService = this;

  m_net = new Networking::NetworkingSystem(true);
  m_net->Initialize();

  Address mCastAddr(226, 0, 0, 1, 8000);
  TODO("Don't hard-code this");
  Address localAddr(inet_addr(localIP.c_str()), 8010);
  m_net->Multicast(localAddr, mCastAddr, false);

  m_net->CreateTcpListener(8010);
  m_net->CreateUdpListener(8010);
  m_net->SetNewTCPConnectionFunc(TCPConnect);
  m_net->SetNewUDPConnectionFunc(UDPConnect);

  m_net->SetHandleDisconnectFunc(Disconnect);
}

MasterServer::~MasterServer()
{
  delete m_net;
  for(InstanceServer* i : m_instanceServers)
  {
    delete i;
  }
}

void MasterServer::Run(ThreadSafeQueue<istring>& userIO)
{
  while(true)
  {
    if(GetTickCount() - m_lastUpdateTime > m_multicastUpdateTime)
    {
      m_lastUpdateTime = GetTickCount();
      Networking::Message portUpdate = CreateNetMessage(MessageID::Locate, "8010", 4);
      m_net->Send(portUpdate, MCAST_ID);
    }

    if(!userIO.Empty())
    {
      istring command = userIO.Pop();
      HandleCommand(command);
    }

    Receive();
    
    SLEEPMILLI(m_sleepTime);
  }
}

void MasterServer::Receive()
{
  s32 con = m_net->NumConnections();
  while (con-- > 0)
  {
    Networking::Message m = m_net->Receive(con);
    if(m.GetSize() > 0)
    {
      s32 id;
      c08* buffer = GetIDAndDataFromMessage(m, id);

      switch(id)
      {
        case MessageID::Locate:
        {
          std::cout << "Locate message recieved from " << m_net->GetConnectionAddress(con) << std::endl;
          InstanceServer* instance = new InstanceServer(con);

          //all connections are clients to start, remove a client to become instance server
          auto it = GetClientIt(con);
          m_clients.erase(it);

          m_instanceServers.push_back(instance);
          break;
        }
        case MessageID::StartNetworkPlay:
        {
          RelocateToInstance(con);
          break;
        }
        case MessageID::StartNetworkGameLevel:
        {
          auto it = GetInstanceIt(con);
          (*it)->gameRunning = true;

          break;
        }
        case MessageID::NetworkGameEnd:
        {
          auto it = GetInstanceIt(con);
          (*it)->gameRunning = false;
          
          break;
        }
        default:
        {
          std::cout << "Got this message " << id << std::endl;
        }
      }//switch
      delete [] buffer;
    }//while size
  }//ifmessage
}

bool MasterServer::HandleCommand(istring& command)
{
  bool ret = false;
  if(command == "list")
  {
    std::cout << "==Instance Servers==" << std::endl;
    for(auto instance : m_instanceServers)
    {
      std::cout << instance->connectionID << " : " 
        << m_net->GetConnectionAddress(instance->connectionID) << " Game Started: " << (instance->gameRunning ? "yes" : "no") << std::endl;
    }
    std::cout << "\n==Clients==" << std::endl;
    for(auto cl : m_clients)
    {
      std::cout << cl << " : " 
        << m_net->GetConnectionAddress(cl) << std::endl;
    }

    ret = true;
  }
  else if(command == "test" || command == "kill" ||
          command == "serv" || command == "client")
  {
    Networking::Message tosend(command);

    auto connections = m_net->GetConnections();
    for(auto it = connections.begin(); it != connections.end(); ++it)
      m_net->Send(tosend, *it);
    ret = true;
  }
  else if(command.find("start") != istring::npos)
  {
    s32 num = atoi(command.substr(6).c_str());

    Networking::Message portUpdate = CreateNetMessage(MessageID::ServerStartNetworkGameButton, nullptr, 0);
    m_net->Send(portUpdate, num);

    std::cout << "Command to start server " << num << std::endl;
  }
  return ret;
}

void MasterServer::HandleTCPConnect(s32 conID)
{
  std::cout << "New TCP Connection " << conID << std::endl;
}


void MasterServer::HandleUDPConnect(s32 id)
{
  m_clients.push_back(id);
}

void MasterServer::HandleDisconnect(s32 id)
{
  auto instance = GetInstanceIt(id);
  if(instance != m_instanceServers.end())
  {
    std::cout << "Instance " << id << " " << m_net->GetConnectionAddress(id) << " Failed" << std::endl;
    delete *instance;
    m_instanceServers.erase(instance);
  }
  else
  {
    auto it = GetClientIt(id);
    if(it != m_clients.end())
      m_clients.erase(it);
  }

}

////////////////////
// Private functions
Networking::Message MasterServer::CreateNetMessage(MessageID::MessageID id, const c08 *data, const u16 buffSize)
{
  const u32 idSize = sizeof(MessageID::MessageID);
  //create a buffer
  c08 *buffer = new char[idSize + buffSize];

  memcpy(buffer, &id, idSize);
  memcpy(buffer + idSize, data, buffSize);
  
  Networking::Message m(buffer, idSize + buffSize);

  delete [] buffer;
  return m;
}

c08* MasterServer::GetIDAndDataFromMessage(Networking::Message message, s32& id)
{
  const u32 idSize = sizeof(MessageID::MessageID);

  id = *RECAST(MessageID::MessageID*, message.GetBuffer());

  if(message.GetSize() < idSize)
    return nullptr;

  c08 *buffer = new c08[message.GetSize() - idSize];
  memcpy(buffer, message.GetBuffer() + idSize, message.GetSize() - idSize);
  
  return buffer;
}

void MasterServer::RelocateToInstance(s32 con)
{
  u16 debugPrint_ = 0;
  DPRINT(0, "Attempty to relocate client...");
  for(auto instance : m_instanceServers)
  {
    if(!instance->gameRunning)
    {
      ++instance->numClients;

      Address instanceAddr = m_net->GetConnectionAddress(instance->connectionID);

      istring buffer = instanceAddr.GetStringAddress();
      buffer.append(" ");
      buffer.append(std::to_string(instanceAddr.GetPort()));

      DPRINT(0, "Relocating client to " << instanceAddr);
      
      Networking::Message msg = CreateNetMessage(MessageID::Relocate, 
        buffer.c_str(), buffer.length());
      m_net->Send(msg, con);

      //remove client
      auto it = GetClientIt(con);
      m_clients.erase(it);

      return;
    }
  }
  DPRINT(0, "No servers available");
}

MasterServer::ServerVecIt MasterServer::GetInstanceIt(s32 con)
{
  auto it = m_instanceServers.begin();
  for(; it != m_instanceServers.end(); ++it)
  {
    if((*it)->connectionID == con)
      return it;
  }
  return m_instanceServers.end();
}

MasterServer::ClientVecIt MasterServer::GetClientIt(s32 con)
{
  auto it = m_clients.begin();
  for(; it != m_clients.end(); ++it)
  {
    if(*it == con)
      return it;
  }
  return m_clients.end();
}
