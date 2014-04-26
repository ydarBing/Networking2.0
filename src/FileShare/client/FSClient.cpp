#include "MainPrecompiled.h"
#include "FSClient.h"
#include "Encoder.h"

#include "InputParser.h"

FSClient::FSClient(Networking::NetworkingSystem *net_) 
  : net(net_), lastSeqTaken(0), nextMessageSeq(1)
{
  msgFunctionMap["filelist|"] = &FSClient::FileListMsg;
  msgFunctionMap["sendfile|"] = &FSClient::SendFileMsg;
  msgFunctionMap["receivefile|"] = &FSClient::ReceiveFileMsg;
  msgFunctionMap["heartbeat|"] = &FSClient::HeartBeatMsg;
}

FSClient::~FSClient()
{
}

bool FSClient::Initialize()
{
  //Attempt to read in the config data.
  if(!config.ReadConfigFile("config.txt", shareFiles))
  {
    return false;
  }

  net->CreateUdpListener(config.clientUDPport);

  Address servAddr(inet_addr(config.serverIP.c_str()), config.serverPort);
  LOG("Connecting to server at: " << servAddr);

  //Connect should return a bad value if it is unsuccessful, but for now we are assuming a 
  //good connection
  serverhConn = net->Connect(servAddr, false);
  serverHeartBeatTimer = GetTickCount();
  /*if(serverhConn == BAD_VALUE)
  {
    return false;
  }*/
  
  //Just send the list of files on initialize
  List();
  SendEndpointPort();
  //We should now be connected to the server.  The client is initialized and ready to start sending
  //messages.
  return true;
}

void FSClient::Receive(Networking::Message& m, s32 id)
{
  if(id == serverhConn)
  {
    HandleServerMessage(m, id);
  }
  else
  {
    HandleServerMessage(m, id);
  }
}

void FSClient::Maintain()
{
  //send a heart beat to the server
  if(GetTickCount() - serverHeartBeatTimer > HEART_BEAT_WAIT)
  {
    net->Send(istring("heartbeat|"), serverhConn);
    serverHeartBeatTimer = GetTickCount();
  }

  Networking::Message m = GetNextMessage();
  if(m.GetSize() != 0)
    HandleFileSaveMessage(m);
}

void FSClient::List()
{
  istring toSend = "list|";
  toSend.append(shareFiles.AllFilesOneString());
  net->Send(toSend, GetServerConn());
}

void FSClient::SendEndpointPort()
{
  std::string sPort("port|");
  sPort += std::to_string(config.clientUDPport);
  net->Send(sPort, serverhConn);
}

s32 FSClient::GetServerConn() const
{
  return serverhConn;
}

void FSClient::HandleServerMessage(Networking::Message& m, s32 id)
{
  std::string out;
  out.assign(m.GetBuffer(), m.GetSize());
  //LOG(out << std::endl;
  //Get the first line from the message, and handle the rest of the data
  //accordingly.
  size_t commandEnd = out.find_first_of("|");
  if(commandEnd == istring::npos)
  {
    LOG("Received a server message without a command");
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
  (this->*msgFunction)(message, id);
}

void FSClient::FileListMsg(std::string &message, s32 id)
{
  LOG(message);
}

void FSClient::SendFileMsg(std::string &message, s32 id)
{
  //Message from the server that we need to send a file to a client
  //message should look like "filename IP port"
  istring filename, ip, sPort;
  u32 port;
  
  std::istringstream stream(message);
  stream >> filename;
  stream >> ip;
  stream >> sPort;
  port = std::stoi(sPort);

  shareFiles.OpenFile(filename);

  //we should probably do some check here
  Address newAddr(inet_addr(ip.c_str()), port);
  s32 newCon = net->Connect(newAddr);

  istring command = "receivefile|";

  //-1 for the \n and sizeof(u32) for the message sequence number
  s32 bufSize = net->
    GetMaxDataSize() - command.size() - filename.size() - sizeof(u32) - 1;
  assertion(bufSize > 0);
  c08* buffer = new c08[bufSize];
  LOG("Sending file " << filename << " Chunck Size: " << bufSize << " to " << ip << " on port " << port);
  filename += '\n';
  //Send like a clear file message or something
  s32 totalSent = 0;
  s32 DBG_COUNT = 0;
  while((bufSize = shareFiles.ReadFile(buffer, bufSize)) > 0)
  {
    Encoder encode(false, net->GetMaxDataSize());
    encode.StartWriting();
    {
      encode.WriteData(command.c_str(), command.size());
      encode.Write(NextMessageSequence());
      encode.WriteData(filename.c_str(), filename.size());
      encode.WriteData(buffer, bufSize);
    }
    encode.DoneWriting();

    Networking::Message m((c08*)encode.GetData(), encode.GetDataSize());

    net->Send(m, newCon);
    totalSent += bufSize;
    ++DBG_COUNT;
    SLEEPMILLI(20);
  }

  //send the message signifying end
  Encoder encode(false, command.size() + sizeof(u32));
  encode.StartWriting();
  encode.WriteData(command.c_str(), command.size());
  encode.Write(0U);
  encode.DoneWriting();
  Networking::Message m((c08*)encode.GetData(), encode.GetDataSize());
  net->Send(m, newCon);
  //reset the message sequence for next file transfer
  nextMessageSeq = 1;

  LOG("SendCount: " << DBG_COUNT);
  LOG(" Done sending file " << filename);
}

void FSClient::ReceiveFileMsg(istring &message, s32 id)
{
  c08* data = new c08[message.size()];
  memcpy(data, message.c_str(), message.size());
  
  Encoder e(true, (u08*)data, message.size());
  u32 seq;
  e.Read(seq);
  delete[] data;

  if(seq == 0)
  {
    seq = UINT_MAX;
    //don't disconnect right away, might be waiting for dropped packets
    //add con ID on to message so save func can disconnect
    message += std::to_string(id);
  }

  Networking::Message toPush(message);
  toPush.SetSequence(seq);
  LOG("Recieved file transfer packet " << seq);
  messagesReceived_.push(toPush);
  return;
}

void FSClient::HeartBeatMsg(istring &message, s32 id)
{
  //LOG("Just receive a heartbeat from " << id << std::endl;
}
void FSClient::HandleDisconnect(s32 id)
{
  LOG("I'm a client and I just got notified about a disconnect.");
  if(id == serverhConn)
  {
    LOG("Disconnected from the server!");
  }
}

void FSClient::Close()
{
  //Send a message that we are disconnecting from the server
  //net->Send(istring("disconnect|"), serverhConn);
  //Disconnect from the server
  net->Disconnect(serverhConn);
  //Not sure how we will handle clients yet...
}

Networking::Message FSClient::GetNextMessage()
{
  if(messagesReceived_.size() == 0)
    return Networking::Message(Networking::MessageError::EMPTY_MESSAGE);

  Networking::Message ret = messagesReceived_.top();

  if(ret.GetSequence() < lastSeqTaken)
  {
    messagesReceived_.pop();
    return Networking::Message(Networking::MessageError::EMPTY_MESSAGE);
  }

  if(ret.GetSequence() != lastSeqTaken + 1 && ret.GetSequence() != UINT_MAX)
  {
    LOG("GetNextMessage didn't find next packet: " 
      << lastSeqTaken + 1 << " this: " << ret.GetSequence());
    return Networking::Message();
  }
  else
    ++lastSeqTaken;

  messagesReceived_.pop();
  return ret;
}

void FSClient::HandleFileSaveMessage(Networking::Message message)
{
  static s32 packetCount = 0;
  static istring filename;

  Encoder encode(true, (u08*)message.GetBuffer(), message.GetSize());
  u32 seq;
  
  encode.StartReading();
  encode.Read(seq);

  if(seq == 0)
  {
    istring con = encode.ReadData(message.GetSize() - sizeof(u32));
    LOG("Final File Transfer Packet -- Disconnecting");
    net->Disconnect(atoi(con.c_str()));
    packetCount = 0;
    filename = "";
    lastSeqTaken = 0;
    return;
  }

  c08* data = encode.ReadData(message.GetSize() - sizeof(u32));
  istring stringData(data, message.GetSize() - sizeof(u32));

  u32 fileNameLen = stringData.find_first_of('\n', 0);
  istring thisFile = stringData.substr(0, fileNameLen);
  if(thisFile == filename)
  {
    ++packetCount;
  }
  else
  {
    packetCount = 0;
    filename = thisFile;
  }

  ++fileNameLen;
  std::cout << packetCount << ": Receiving file " << filename << " Chunk Size: " << stringData.size() - fileNameLen << std::endl;

  std::ofstream out("Received\\" + filename, std::ios::out | std::ios::binary | std::ios::app);
  if(fileNameLen < stringData.size())
    out << stringData.substr(fileNameLen);
}

u32 FSClient::NextMessageSequence()
{
  return nextMessageSeq++;
}