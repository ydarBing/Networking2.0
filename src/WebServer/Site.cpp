/*
 *  FILE          Site.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles website
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "MainPrecompiled.h"
#include "Site.h"
#include "Page.h"
#include "URIParser.h"

#include <ctime>// setting date in response
#pragma warning( disable : 4996 ) // stupid _s functions

// DEBUG GLOBALS
static time_t LastRequestTime;

Site::Site(MakeSite s)
  : m_makeSite_CB(s)
{
  SetUpSitePages();
}

void Site::AddPage(Page* page)
{
  m_pages[page->m_uriName] = page;
}

istring Site::HandleRequest(const Request& rqst, Response& rsp, void* data_)
{
  HTML htmlPage;
  HTTP_STATUS_CODE rspCode = INTERNAL_SERVER_ERR;

  istring uri = rqst.GetURI();
  URIParser parsedUri;
  parsedUri.DecodeURI(uri);
  Path path = parsedUri.DecodePath();

  auto correctPage = m_pages.find(path.segments[0]);
  
  if(correctPage != m_pages.end())
  {
    PageInfo info(rqst, htmlPage, this);
    info.debug.lastRequestTime = LastRequestTime;
  
#ifdef DEBUG
    info.debug.lastRequest = rqst.m_lastRequest;
#endif

    rspCode = correctPage->second->HandleRequestedPage(info, data_);
  }
  else// rqst not in site
  {
    rspCode = NOT_FOUND;
    htmlPage.AddTitle("404 (Not Found)");
    htmlPage.AddExternalCSS("web/css/error.css");
    htmlPage.StartBody();
    htmlPage.AddBoldLine("404", true);
    htmlPage.AddBoldLine("Could not find \"" + uri + "\"", true);
  }

  time(&LastRequestTime);
  
  if(htmlPage.m_inBody == false)
  {
    if(htmlPage.AddedContent())
      htmlPage.StartBody();
    else
      return "";// nothing was written, just
  }

  rsp.SetStatus(rspCode);
  return htmlPage.Finish();
}

void Site::SetUpSitePages(void)
{
  if(m_makeSite_CB)
    m_makeSite_CB(this);
}

Site::~Site()
{
  auto it = m_pages.begin();

  for(; it != m_pages.end(); ++it)
  {
    delete it->second;
  }
}

const std::vector<Page*> Site::GetPages(void) const
{
  auto it = m_pages.begin();
  std::vector<Page*> pages;

  for(; it != m_pages.end(); ++it)
    pages.push_back(it->second);
  
  return pages;
}



istring GetDate(void)
{
  static time_t rawTime;
  time(&rawTime);
  istring date = ctime(&rawTime);
  date.pop_back(); // take out \n
  //date += " GMT";
  return date;
}

