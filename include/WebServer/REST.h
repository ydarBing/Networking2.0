/*
 *  FILE          REST.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Basic ReST interface
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once
class HTTP;
// callbacks
class Response;
typedef void (*ResponseBegin)(const Response* rsp, void* userData);
typedef void (*ResponseData)(const Response* rsp, void* userData, const c08* data, s32 numBytes);
typedef void (*ResponseComplete)(const Response* rsp, void* userData);


enum METHOD
{
  M_OPTIONS = 0,
  M_GET,
  M_HEAD,
  M_POST,
  M_PUT,
  M_DELETE,
  M_TRACE,
  M_CONNECT,
  M_TOTAL
};

static const char* strMETHOD[8] =
{
  "OPTIONS",
  "GET",
  "HEAD",
  "POST",
  "PUT",
  "DELETE",
  "TRACE",
  "CONNECT"
};

class Response;

class Request
{
public:
  typedef std::vector<istring> Headers;
  Request();
  ~Request();
  // method is GET, POST etc..
  // host - www.example.com
  // url - /index.html
  // headers should be pushed as 
  //  1. name 
  //  2. value 
  //  3. name  4. value and so on
  Request(istring method, istring uri, istring httpVer, Headers headers = std::vector<istring>(), istring body = "");
  Request(METHOD method, istring uri, istring httpVer, Headers headers = std::vector<istring>(), istring body = "");

  istring MakeRequest(void);

  void AddToHeader(istring name, istring value);
  void AddToHeader(istring name, s32 value);
  const istring GetHeaderValue(istring name)const;

  istring GetHttpVersion(void)const;

  METHOD GetMethod()const;
  void SetMethod(METHOD m);
  void DocumentHeader(istring line);
  void SetBody(istring body);
  istring GetURI(void)const;

#ifdef DEBUG
  istring m_lastRequest;

#endif // DEBUG

private:
  METHOD m_method; 
  istring m_httpVer;
  //ex host is 'www.example.com'
  istring m_uri; // ex  '/index.html'
  istring m_body;

  std::unordered_map<istring, istring> m_headers;
};

enum HTTP_STATUS_CODE
{
  // informational
  CONTINUE     = 100,
  SWITCH_PROTO = 101,
  // successful
  OK           = 200,
  CREATED      = 201,
  ACCEPTED     = 202,
  // Redirection
  MULT_CHOICES = 300,
  // client error
  BAD_REQUEST  = 400,
  NOT_FOUND    = 404,
  // Server error
  INTERNAL_SERVER_ERR = 500,
  BAD_GATEWAY         = 502
};

class Response
{
public:
  Response();
  istring MakeResponse(void);
  
  void Process(istring data);
  void SetHeader(istring name, istring value);
  void SetHeader(istring name, s32 value);
  istring GetHeaderValue(istring name)const;
  bool IsCompleted(void)const;
  s32 GetStatus(void)const;
  void SetStatus(s32 statusCode);
  istring GetReason(void)const;
  istring GetReasonPhrase(void)const;
  void SetBody(istring body);
  // name ends with ':' 
  void AddNameValuePair(istring& nvPair);
  void AddNameValuePair(istring name, istring value);

private:
  void ProcessStatus(istring& data);
  void ProcessHeader(istring& data);
  s32 ProcessBody(istring& data);
  void ReadInHeaderLine(istring& line);
  void StartBody(void);
  void FinishResponse(void);
  enum CurrentSection{
    CS_STATUS,
    CS_HEADER,
    CS_BODY,
    CS_DONE
  };
private:
  HTTP*   m_connection;
  typedef std::unordered_map<istring, istring> Header;
  Header  m_headerPairs;// name value pairs

  CurrentSection m_section;
  s32     m_majorVersion;
  s32     m_minorVersion;
  istring m_method;
  istring m_reason;
  istring m_header;
  istring m_body;
  s32     m_status;

  s32     m_bodyLength;
  s32     m_bytesRead;
};
