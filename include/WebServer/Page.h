/*
 *  FILE          Page.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles the webpage
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include "HTTP.h"// status codes, contains request and response stuff
#include "HTML.h"

struct DebugInfo
{
  // just for testing server
  time_t lastRequestTime;
  istring lastRequest;
};
struct PageInfo
{
  PageInfo(const Request& r, HTML& html, const Site* site);
  const Request& rqst;
  HTML& page;
  const Site* site;
  DebugInfo  debug;
};
 // Networking::NetworkingSystem* m_userData;

typedef HTTP_STATUS_CODE (*HandleRequest_CB)(const PageInfo& info, void* userData_);

class Site;


class Page
{
public:
  // localhost:8000/uriname
  // uriName   = used to create hyperlink (should be all lowercase)
  // pageName = name used for hyperlink to this page
  Page(istring uriName, HandleRequest_CB cb, istring pageName = "");
  HTTP_STATUS_CODE HandleRequestedPage(const PageInfo& info, void* userData_);
  void AddSubPage(Page* page);
  istring GetURI(void)const;
  istring GetPageName(void)const;

private:
  friend class Site;
  HandleRequest_CB m_pageFunct;
  istring m_pageName;
  std::vector<Page*> m_subPages; 
  istring m_uriName;
};
