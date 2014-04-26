/*
 *  FILE          SimpleSite.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles all of the pages on a website
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include "HTTP.h"
#include "Page.h" // PageInfo

class Site;

void CreateSampleSite(Site* site);

struct SamplePages
{
  static HTTP_STATUS_CODE IndexPage(const PageInfo& info, void* userData_);
  static HTTP_STATUS_CODE NetStatsPage(const PageInfo& info, void* userData_);
  static HTTP_STATUS_CODE AnotherPage(const PageInfo& info, void* userData_);
  static HTTP_STATUS_CODE ProcessStatsPage(const PageInfo& info, void* userData_);
  static HTTP_STATUS_CODE CSSExamplePage(const PageInfo& info, void* userData_);
  static HTTP_STATUS_CODE JavaScriptExamplePage(const PageInfo& info, void* userData_);

};
