#include "MainPrecompiled.h"
#include "HTTP.h"
#include "Page.h"
#include "Site.h"

#include "URIParser.h"

//HTTP setup
HTTP::HTTP()
{
  const istring accepted[AT_TOTAL] =
  {
    "text",
    "plain",
    "html",
    "css",
    "image",
    "webp",
    "*"
  };
  // set up accepted types
  for(u32 i = 0; i < AT_TOTAL; ++i)
    m_acceptLookUp[accepted[i]] = SCAST(Accepted_Types, i);
}


//HTTP teardown
HTTP::~HTTP()
{
}

//The processing function
//Return error code (0 is no error)
int HTTP::Process(c08* bytes, u32 size, Socket& from)
{
  ConnectionTimeOut(from);

  if(size == 0)
    return 0;

  istring data(bytes, size);
  Request rqst = ParseIncomingRequest(data);
  Response rsp;
  bool valid = CheckForRequestErrors(rqst);
  KeepAlive(rqst, rsp, from);
  WhatIsRequestAskingFor(rqst, rsp);
  rsp.SetHeader("Date", GetDate());

#ifdef DEBUG
  rqst.m_lastRequest = data;
#endif // DEBUG

  istring rspBody = ProcessRequest(rqst, rsp);

  rsp.SetBody(rspBody);
  istring toSend = rsp.MakeResponse();
  
//////////////// DEBUG ////////////// 
  istring fullRequest = "\r\nREQUEST\r\n" + data + "\r\n";
  std::cout << fullRequest;
  LOG(fullRequest);

  if(m_acceptType == TEXT) // outputting images is not a good idea
  {
    istring fullResponse = "\r\nRESPONSE to: " + std::to_string(from) + "\r\n" + toSend + "\r\n";
    std::cout << fullResponse;
    LOG(fullResponse);
  }
//////////////// DEBUG ////////////// 

  AddToSendQueue(toSend.c_str(), toSend.size(), from);

  
  return 0;
}

Request HTTP::ParseIncomingRequest(istring data)
{
  std::stringstream streamData(data);
  istring method, uri, httpVer;
  streamData >> method >> uri >> httpVer;
  Request rqst(method, uri, httpVer);
  
  int slashr = streamData.get(); // get the \r\n
  int slashn = streamData.get();
  istring line("notEmptyToStart");
  while(line.empty() == false)
  {
    std::getline(streamData, line);
    if(line[0] =='\r')
      break; // at the body
    rqst.DocumentHeader(line);
  }

  istring bod;
  line.clear();
  streamData >> line;
  u32 pos = SCAST(u32, streamData.tellp());
  if(pos != istring::npos)
  { 
    bod = streamData.str().substr(pos);
    line += bod;
    rqst.SetBody(line);
  }

  return rqst;
}

bool HTTP::KeepAlive(Request& rqst, Response& rsp, Socket& from)
{
  istring isKeepAlive = rqst.GetHeaderValue("Connection");

  if(isKeepAlive.find("close") != istring::npos // client requested to close connection
    || m_webConnections.find(from) == m_webConnections.end()) 
  {
    // send response and close connection
    std::cout << "received a connection close request" << std::endl;
    rsp.SetHeader("Connection", "close");
    ClientKeepAlive::size_type erased = m_webConnections.erase(from);// client said to close connection
    if(erased)
      DisconnectWhenDone(from);
    return false;
  }
  else
  {
    m_webConnections[from].howLong = DEFAULT_KEEP_ALIVE_TIME;
    //std::cout << "received a connection keep alive request" << std::endl;
    rsp.SetHeader("Connection", "keep-alive");
    return true;
  }
}

void HTTP::SetCallBack(void* userData, Site* site)
{
  assertion(site);
  m_site = site;
  m_userData = userData;
}

bool HTTP::CheckForRequestErrors(Request& rqst)
{ 
  bool ret = true;
  istring mediaType = rqst.GetHeaderValue("Content-Type"); // indicates the media type of the entity-body
  if(mediaType.empty() == false)
    ret = CheckContentType(mediaType);

  return ret;
}

bool HTTP::CheckContentType(const istring& mediaType)
{
  //media-type = type "/" subtype *( ";" parameter )
  // ex: Content-Type: text/html; charset=ISO-8859-4

  return true;
}

void HTTP::ConnectionTimeOut(Socket& from)
{
  if(m_webConnections.find(from) != m_webConnections.end())
  {
    //update keepAliveTimer
    time_t check;
    time(&check);
    auto user = m_webConnections[from];
    f64 tCheck = difftime(check, user.timeSinceLastKeepAlive);

    time(&user.timeSinceLastKeepAlive);
    //user.howLong = DEFAULT_KEEP_ALIVE_TIME;
    //user.howLong -= tCheck;
    //std::cout << "Time left before socket(" << from << ") times out: " << user.howLong 
    //          << ". Num web connections: " << m_webConnections.size() << std::endl;
    

    if(tCheck > DEFAULT_KEEP_ALIVE_TIME)// timed out
    {
      m_webConnections.erase(from);
      DisconnectWhenDone(from);
    }

  }
  else
  {
    // new connection
    m_webConnections[from] = KeepAliveInfo();
  }
}

void HTTP::WhatIsRequestAskingFor(Request& rqst, Response& rsp)
{
  istring what = rqst.GetHeaderValue("Accept");
  if( !what.empty())
  {
    size_t slash = what.find_first_of("/");
    istring t = what.substr(0, slash);
    size_t comma = what.find_first_of(",", slash);
    slash += 1;
    istring st = what.substr(slash, comma - slash);
    m_acceptType = m_acceptLookUp[t];
    m_acceptSubType = m_acceptLookUp[st];
    rsp.SetHeader("Content-Type", t + "/" + st);
  }
  else // default to plain text
  {
    m_acceptType = TEXT;
    m_acceptSubType = PLAIN;
    rsp.SetHeader("Content-Type", "text/plain");
  }
}

istring HTTP::ProcessRequest(Request& rqst, Response& rsp)
{
  istring rspBody;

  switch(m_acceptType)
  {
  case TEXT:
    rspBody = ProcessTextRequest(rqst, rsp);
    break;
  case IMAGE:
    rspBody = ProcessImageRequest(rqst, rsp);
    break;
  case ANY:// have to do further inspection on uri
    rspBody = ProcessAnyRequest(rqst, rsp);
    break;
  default:// by default send it to the site to handle
    rspBody = m_site->HandleRequest(rqst, rsp, m_userData);
  }

  return rspBody;
}

istring HTTP::ProcessTextRequest(Request& rqst, Response& rsp)
{
  istring rspBody;
  switch (m_acceptSubType)
  {
  case HTTP::PLAIN:
    break;
  case HTTP::HTML:
    rspBody = m_site->HandleRequest(rqst, rsp, m_userData);
    break;
  case HTTP::CSS:
    {
      size_t valid = rqst.GetURI().find_first_of("css");
      if(valid != istring::npos)
      {
        istring loc = rqst.GetURI().substr(1);// dont include the first forward slashes
        rspBody = GetFile(loc);
      }
      if(rspBody.empty())
        rsp.SetStatus(NOT_FOUND);
      else
        rsp.SetStatus(OK);
    }
    break;
  default:
    break;
  }
  return rspBody;
}

istring HTTP::ProcessImageRequest(Request& rqst, Response& rsp)
{
  istring rspBody;
  istring file = rqst.GetURI().substr(1);
  switch (m_acceptSubType)
  {
  case HTTP::WEBP:
    {
      rspBody = GetFile(file);
      if(rspBody.empty())
        rsp.SetStatus(NOT_FOUND);
      else
        rsp.SetStatus(OK);
    }
    break;
  default:
    break;
  }
  return rspBody;
}

istring HTTP::GetFile(const istring fileName)
{
  // ios::ate, open and seed to end immediately after opening
  std::ifstream png(fileName, std::ios::in | std::ios::binary | std::ios::ate);
  if(png.is_open())
  {
    std::ifstream::pos_type fileSize = png.tellg();
    png.seekg(0, std::ios::beg);
    std::vector<c08> bytes(SCAST(u32, fileSize));
    png.read(&bytes[0], fileSize);

    istring ret(&bytes[0], SCAST(u32, fileSize));
    return ret;
  }

  return "";
}

istring HTTP::ProcessAnyRequest(Request& rqst, Response& rsp)
{
  // right now only this funtion can only get favicon.ico
  istring uri = rqst.GetURI();
  URIParser parsed;
  parsed.DecodeURI(uri);

  istring ret = GetFile(parsed.GetPath());

  if(ret.empty())
    rsp.SetStatus(NOT_FOUND);
  else
    rsp.SetStatus(OK);

  return ret;
}

HTTP::KeepAliveInfo::KeepAliveInfo():
  howLong(DEFAULT_KEEP_ALIVE_TIME)
{
  time(&timeSinceLastKeepAlive);
}
