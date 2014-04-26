/*
 *  FILE          Page.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles the webpage
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "MainPrecompiled.h"
#include "Page.h"

#include "NetworkSystem.h"// for the void* userData
#include "Rest.h"
#include "URIParser.h"


Page::Page(istring uriName, HandleRequest_CB cb, istring pageName /*= ""*/)
  : m_uriName(uriName),  m_pageFunct(cb), m_pageName(pageName)
{
}

HTTP_STATUS_CODE Page::HandleRequestedPage(const PageInfo& info, void* userData_)
{
  if(m_pageFunct)
    return m_pageFunct(info, userData_);
  else
    return INTERNAL_SERVER_ERR;
}

void Page::AddSubPage(Page* page)
{
  m_subPages.push_back(page);
}

istring Page::GetURI(void) const
{
  return m_uriName;
}

istring Page::GetPageName(void) const
{
  return m_pageName;
}


PageInfo::PageInfo(const Request& r, HTML& html, const Site* site)
  : rqst(r), page(html), site(site)
{
}

