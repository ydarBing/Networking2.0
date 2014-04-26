#include "MainPrecompiled.h"
#include "FSServer.h"

FSServer::FSServer(Networking::NetworkingSystem *net_) : net(net_)
{
  msgFunctionMap["list|"] = &FSServer::ListMsg;
  msgFunctionMap["get|"] = &FSServer::GetFileMsg;
  msgFunctionMap["show|"] = &FSServer::GetFileListMsg;
  msgFunctionMap["port|"] = &FSServer::PortMsg;
  msgFunctionMap["heartbeat|"] = &FSServer::HeartBeatMsg;
}

FSServer::~FSServer()
{

}

bool FSServer::Initialize()
{
	if(!config.ReadConfigFile("config.txt"))
	{
		return false;
	}

  //Open up for connections
  net->CreateTcpListener(config.serverPort);
  return true;
}

void FSServer::Receive(Networking::Message& m, s32 id)
{
  HandleClientMessage(m, id);
}

void FSServer::HandleClientMessage(Networking::Message& m, s32 id)
{
  std::string out;
  //out.assign(m.GetBuffer(), m.GetSize());
  //std::cout << std::endl << std::endl << out << std::endl << std::endl;
  //Get the first line from the message, and handle the rest of the data
  //accordingly.
  size_t commandEnd = out.find_first_of("|");
  if(commandEnd == istring::npos)
  {
    LOG("Recieved a server message without a command");
    return;
  }

  std::string command = out.substr(0, commandEnd + 1);
  std::string message = out.substr(commandEnd + 1);

  //call the appropriate function
  std::map<std::string, MsgFunction>::iterator it = msgFunctionMap.find(command);
  if(it == msgFunctionMap.end())
  {
    LOG("Invalid command received from server: " << command);
    return;
  }

  MsgFunction msgFunction = it->second;
  (this->*msgFunction)(id, message);
}

void FSServer::Maintain()
{
  //IF enough time has passed since a client sent a heartbeat, he is 
  //disconnected from the server
  for(auto it = activeCons.begin(); it != activeCons.end(); ++it)
  {
    s32 con = it->first;
    s32 lastHeartBeatSent = it->second;

    if(GetTickCount() - lastHeartBeatSent > HEART_BEAT_WAIT)
    {
      net->Send(istring("heartbeat|"), con);
      it->second = GetTickCount();
    }
  }
}

void FSServer::ListMsg(s32 con, std::string &message)
{
  if(fileList.find(con) != fileList.end())
  {
    auto it = fileList.find(con);
    it->second.clear();
  }
  
  //filename\n
  u32 lastPos = 0;
  for(u32 i = 0; i < message.size();)
  {
    lastPos = i;
    i = message.find_first_of('\n', i);
    
    istring filename = message.substr(lastPos, i - lastPos);
    fileList[con].push_back(filename);

    LOG("filename: " << filename);
    ++i;
  }
}

void FSServer::RemoveDsiconnectedClientFiles(s32 id)
{
  if(fileList.find(id) != fileList.end())
  {
    auto it = fileList.find(id);
    fileList.erase(it);
  }
}

void FSServer::GetFileListMsg(s32 con, std::string& message)
{
  istring toSend = "filelist|";
  for(auto it = fileList.begin(); it != fileList.end(); ++it)
  {
    if(it->first == con) //dont show your own files
      continue;

    for(auto fileIt = it->second.begin(); fileIt != it->second.end(); ++fileIt)
    {
      toSend += *fileIt + '\n';
    }
  }
  net->Send(toSend, con);
}

void FSServer::GetFileMsg(s32 con, std::string &message)
{
  istring toSend;
  for(auto it = fileList.begin(); it != fileList.end(); ++it)
  {
    if(it->first == con)
      continue;
    for(auto fileIt = it->second.begin(); fileIt != it->second.end(); ++fileIt)
    {
      if(*fileIt == message)
      {
        //con wants the file, it->first has the file!
        //con has a port it is expecting to receive shit it, it->first needs to know about that port, and the IP
        //Have it->first connect directly to con, then send a file
        const Address& sendToConnection = net->GetConnectionAddress(con);
        istring toSend = "sendfile|" + message + " " + sendToConnection.GetStringAddress() + " " + std::to_string(endpointPorts[con]);
        net->Send(toSend, it->first);
        return;
      }
    }
  }
}

void FSServer::PortMsg(s32 con, std::string &message)
{
  u32 port = std::stoi(message);
  endpointPorts[con] = port;
}

void FSServer::HeartBeatMsg(s32 con, std::string &message)
{
  //LOG("Heart beat received from " << con);
}

void FSServer::HandleNewTCPConnection(s32 id)
{
  LOG("I'm the server and I just got a new TCP connection.");
  activeCons[id] = GetTickCount();

}

void FSServer::HandleDisconnect(s32 id)
{
  LOG("I'm the server and I just got notified about a disconnect");
  //Remove the files from the list
  RemoveDsiconnectedClientFiles(id);
}