/*
 *  FILE          REST.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Basic ReST interface
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "MainPrecompiled.h"
#include "REST.h"

#include "HTTP.h"


Request::Request() 
  : m_method(M_GET)
{
}

Request::~Request()
{

}

METHOD Request::GetMethod() const
{
  return m_method;
}

void Request::SetMethod(METHOD m)
{
  m_method = m;
}

Request::Request(istring method, istring uri, istring httpVer, Headers headers /*= std::vector<istring>()*/, istring body /*= ""*/)
  : m_uri(uri), m_httpVer(httpVer), m_body(body)
{
  for(u32 i = 0; i < M_TOTAL; ++i)
  {
    if(strMETHOD[i] == method)
    {
      m_method = SCAST(METHOD, i);
      break;
    }
  }

  bool hasContentLen = false;
  // check headers for content-length
  assertion(headers.size() % 2 == 0); // name value pairs, has to be even
  for(unsigned i = 0; i < headers.size(); i += 2)
  {
    if(headers[i] == "content-length")
      hasContentLen = true;
  }

  // if not a content length already put on there
  if(!body.empty() && !hasContentLen)
    AddToHeader("Content-Length", body.size());

  for(unsigned i = 0; i < headers.size(); i += 2)
    AddToHeader(headers[i], headers[i + 1]);
}


// all info ready for whole request, just call MakeRequest
Request::Request(METHOD method, istring uri, istring httpVer, Headers headers, istring body)
  : m_method(method), m_body(body), m_uri(uri), m_httpVer(httpVer)
{
  bool hasContentLen = false;
  // check headers for content-length
  assertion(headers.size() % 2 == 0); // name value pairs, has to be even
  for(unsigned i = 0; i < headers.size(); i += 2)
  {
    if(headers[i] == "content-length")
      hasContentLen = true;
  }

  // if not a content length already put on there
  if(!body.empty() && !hasContentLen)
    AddToHeader("content-length", body.size());

  for(unsigned i = 0; i < headers.size(); i += 2)
    AddToHeader(headers[i], headers[i + 1]);

}

void Request::AddToHeader(istring name, istring value)
{
  m_headers[name] = value;
}

void Request::AddToHeader(istring name, s32 value)
{
  m_headers[name] = std::to_string(value);
}

const istring Request::GetHeaderValue(istring name)const
{
  auto it = m_headers.find(name);
  if(it == m_headers.end())
    return "";
  else
    return it->second;
}


// call this once all headers and information is done
istring Request::MakeRequest(void)
{
  static const istring endLine("\r\n");

  std::stringstream output;
  output << strMETHOD[m_method] << " " << m_uri << " HTTP/1.1" << endLine; 
  output << "Host: " << GetHeaderValue("Host") << endLine;

  for(auto it = m_headers.begin(); it != m_headers.end(); ++it)
    output << it->first << ": " << it->second << endLine;

  // make sure the content length is always in
  auto len = m_headers.find("content-length");
  if(len == m_headers.end())
  {
    istring bodLen;
    if(m_method == M_HEAD)
      bodLen = std::to_string(0);
    else
      bodLen = std::to_string(m_body.size());

    output << "Content-Length: " << bodLen << endLine;
  }

  output << endLine;

   output << m_body;

   std::cout << output << std::endl;

   //if(!m_body.empty())
   //{
   //  // probably send body separately
   //}

   return output.str();
}


istring Request::GetHttpVersion(void) const
{
  return m_httpVer;
}

void Request::DocumentHeader(istring line)
{
  std::stringstream headers(line);
  istring name, value;

  headers >> name >> value;// start value without spaces
  name.pop_back(); // take out colon

  u32 spot = SCAST(u32, headers.tellg()); // first space if any
  value += line.substr(spot);             // add rest of value
  value.pop_back();                       // take out /r

  AddToHeader(name, value);
}
void Request::SetBody(istring body)
{
  m_body = body;
}
istring Request::GetURI(void) const
{
  return m_uri;
}




// network receives request 
Response::Response()
  : m_section(CS_STATUS), m_method(),
    m_majorVersion(1), m_minorVersion(1),
    m_reason(), m_header(),
    m_status(500), m_bodyLength(-1), m_bytesRead(0)
{
}


void Response::AddNameValuePair(istring name, istring value)
{
  m_headerPairs[name] = value;
}

void Response::AddNameValuePair(istring& nvPair)
{
  istring name, value;

  std::size_t start = nvPair.find_first_not_of(' ');
  std::size_t end   = nvPair.find_first_of(':', start); 

  name = nvPair.substr(start, end - start);
  std::transform(name.begin(), name.end(), name.begin(), tolower);
  start = nvPair.find_first_not_of(' ', end + 1);
  value = nvPair.substr(start);// TODO does it matter if it has \r\n ?

  // put name value pair in map
  m_headerPairs[name] = value;
}


void Response::StartBody(void)
{
  m_bodyLength = -1;

  istring bLen = GetHeaderValue("Content-Length");
  if(bLen.empty() == false)
    m_bodyLength = std::atoi(bLen.c_str());

  if(m_method == "HEAD")// RFC says "MUST NOT return a message-body in the response"
    m_bodyLength = 0;
}

  // find a specific value from name value pair in header
istring Response::GetHeaderValue(istring name) const
{
  std::transform(name.begin(), name.end(), name.begin(), tolower);
  auto found = m_headerPairs.find(name);
  if(found == m_headerPairs.end())
    return "";
  else 
    return found->second;
}

bool Response::IsCompleted(void) const
{
  return m_section == CS_DONE;
}

s32 Response::GetStatus(void) const
{
  return m_status;
}

void Response::SetStatus(s32 statusCode)
{
  m_status = statusCode;
}

istring Response::GetReason(void) const
{
  return m_reason;
}

void Response::FinishResponse(void)
{
  m_section = CS_DONE;
}

void Response::SetHeader(istring name, istring value)
{
  m_headerPairs[name] = value;
}

void Response::SetHeader(istring name, s32 value)
{
  m_headerPairs[name] = std::to_string(value);
}

istring Response::MakeResponse(void)
{
  static const istring EndLine("\r\n");
  std::stringstream output;
  istring ReasonPhrase = GetReasonPhrase();
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
  output << "HTTP/" << m_majorVersion << "." << m_minorVersion << " " << m_status << " " << ReasonPhrase << EndLine;

  for(auto it = m_headerPairs.begin(); it != m_headerPairs.end(); ++it)
    output << it->first << ": " << it->second << EndLine;

  istring len = GetHeaderValue("Content-Length");
  if(len.empty())
    output << "Content-Length: " << m_bodyLength << EndLine; 

  output << EndLine;
  output << m_body;

  return output.str();
}

istring Response::GetReasonPhrase(void) const
{
  switch(m_status)
  {
  case CONTINUE:
    return "Continue";
  case SWITCH_PROTO:
    return "Switching Protocols";
  case OK:
    return "OK";
  case CREATED: 
    return "Created";
  case ACCEPTED:
    return "Accepted";
  case MULT_CHOICES:
    return "Multiple Choices"; 
  case BAD_REQUEST:
    return "Bad Request"; 
  case NOT_FOUND: 
    return "Not Found"; 
  case INTERNAL_SERVER_ERR:
    return "Internal Server Error"; 
  case BAD_GATEWAY:
    return "Bad Gateway"; 
  default:
    return "Reason Phrase not accounted for";
  }
}

void Response::SetBody(istring body)
{
  m_body = body;
  m_bodyLength = body.size();
}



