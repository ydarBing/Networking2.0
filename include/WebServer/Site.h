/*
 *  FILE          Site.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles the webpage
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once


istring GetDate(void);


class Page;
class Request;
class Response;

class Site
{
public:
  typedef void (*MakeSite)(Site* site);

  Site(MakeSite s);
  // deletes all pages
  ~Site();
  // callback used for receiving a request
  istring HandleRequest(const Request& rqst, Response& rsp, void* data_);
  void AddPage(Page* page);
  const std::vector<Page*> GetPages(void) const;

private:
  void SetUpSitePages(void);

private:
  MakeSite m_makeSite_CB;
  std::unordered_map<istring, Page*> m_pages;
};


