#pragma once
#include "FSClientConfigData.h"
#include "NetworkingSystem.h"
#include "FileReader.h"

#define HEART_BEAT_WAIT 2000

//This class encapsulates the client side of the FileShare program for assignment one
class FSClient
{
public:
  FSClient(Networking::NetworkingSystem *net_);
  ~FSClient();

  //If the config file is valid, it tries to connect to the server, and also opens a port
  //for file transfer.  
  //Is there a way to check if the connection with the server is succesful?
  //Can we run an update that will check if we ned to try to reconnect?
  bool Initialize();
  void Receive(Networking::Message& m, s32 id);
  void HandleDisconnect(s32 id);
  void Maintain();
  void List();
  void Close();
  void SendEndpointPort();
  s32 GetServerConn() const;
private:
  FSClientConfigData config;
  FileReader  shareFiles;
  s32 serverhConn;
  u32 serverHeartBeatTimer;
  Networking::NetworkingSystem *net;

  //for receiving files in order
  u32 lastSeqTaken;
  u32 nextMessageSeq;
  std::priority_queue<Networking::Message, 
                      std::vector<Networking::Message>, 
                      std::greater<Networking::Message>> messagesReceived_;//The "Received" queue
  u32 NextMessageSequence();
  Networking::Message GetNextMessage();
  void HandleFileSaveMessage(Networking::Message m);

  void HandleServerMessage(Networking::Message& m, s32 id);
  void HandleFileTransferMessage(Networking::Message& m, s32 id);


  //Server Message functions
  typedef void (FSClient::*MsgFunction)(std::string&, s32);
  std::map<std::string, MsgFunction> msgFunctionMap;
  void FileListMsg(std::string &message, s32 id);
  void SendFileMsg(std::string &message, s32 id);
  void ReceiveFileMsg(std::string &message, s32 id);
  void HeartBeatMsg(std::string &message, s32 id);
  //File transfer command functions
  void IncomingFile(Networking::Message& m, s32 id);
};