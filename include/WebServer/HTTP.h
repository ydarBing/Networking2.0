#pragma once
#include "IProtocol.h"
#include "REST.h"

class Site;

static f64 DEFAULT_KEEP_ALIVE_TIME = 7.0f; 


//typedef istring (*GotRequest_CB)(const Request& rqst, void* userData, Site* site);
class HTTP : public Networking::IProtocol
{
public:
  HTTP();
  virtual ~HTTP();

  //The processing function
  virtual int Process(c08* bytes, u32 size, Socket& from);
  void SetCallBack(void* userData, Site* site);

  struct KeepAliveInfo
  {
  public: 
    KeepAliveInfo();
    f64 howLong;
    time_t timeSinceLastKeepAlive;
  };

private:
  enum Accepted_Types
  {
    TEXT = 0,//type, subtypes below
    PLAIN,
    HTML,
    CSS,
    IMAGE,//type, subtypes below
    WEBP,
    ANY, // *
    AT_TOTAL
  };
  istring ProcessRequest(Request& rqst, Response& rsp);
  void ConnectionTimeOut(Socket& from);
  bool KeepAlive(Request& rqst, Response& rsp, Socket& from);
  Request ParseIncomingRequest(istring data);
  bool CheckForRequestErrors(Request& rqst);
  bool CheckContentType(const istring& mediaType);
  void WhatIsRequestAskingFor(Request& rqst, Response& rsp);
  istring ProcessTextRequest(Request& rqst, Response& rsp);
  istring ProcessImageRequest(Request& rqst, Response& rsp);

  istring GetFile(const istring fileName);
  istring ProcessAnyRequest(Request& rqst, Response& rsp);
private:
  typedef std::unordered_map<Socket, KeepAliveInfo> ClientKeepAlive;
  ClientKeepAlive m_webConnections;
  
  std::unordered_map<istring, Accepted_Types> m_acceptLookUp;
  Accepted_Types m_acceptType;
  Accepted_Types m_acceptSubType;

  Site*          m_site;
  void*          m_userData;
  //GotRequest_CB  m_gotRequest_CB;
};
