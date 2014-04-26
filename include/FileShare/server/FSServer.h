#pragma once
#include "FSServerConfigData.h"
#include "NetworkingSystem.h"

//This class encapsulates the client side of the FileShare program for assignment one

#define ARE_YOU_ALIVE_WAIT 1000
#define HEART_BEAT_WAIT 2000

class FSServer
{
public:
  FSServer(Networking::NetworkingSystem *net_);
  ~FSServer();


  bool Initialize();
  void Receive(Networking::Message& m, s32 id);
  void HandleNewTCPConnection(s32 id);
  void HandleDisconnect(s32 id);
  void Maintain();
  s32 GetServerConn() const;

private:
  FSServerConfigData config;
  Networking::NetworkingSystem *net;

  std::map<s32, u32> activeCons;

  void HandleClientMessage(Networking::Message& m, s32 id);
  //Server Message functions
  typedef void (FSServer::*MsgFunction)(s32, std::string&);
  std::map<std::string, MsgFunction>  msgFunctionMap;
  std::map<s32, std::vector<istring>> fileList;
  std::map<s32, u32> endpointPorts;

  void ListMsg(s32 con, std::string &message);
  void GetFileMsg(s32 con, std::string &message);
  void GetFileListMsg(s32 con, std::string &message);
  void PortMsg(s32 con, std::string &message);
  void HeartBeatMsg(s32 con, std::string &message);

  //Helper
  void RemoveDsiconnectedClientFiles(s32 id);
};