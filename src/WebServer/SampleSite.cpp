/*
 *  FILE          SimpleSite.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Handles all pages on website
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "MainPrecompiled.h"
#include "NetworkingSystem.h"// userdata
#include "SampleSite.h"
#include "Site.h"  // Site/time

static time_t AppStartTime, curTime;

void CreateSampleSite(Site* site)
{
  time(&AppStartTime);
  // all page names should be under case and no spaces
  // first param is what is going to be in the uri
  Page* index = new Page("", SamplePages::IndexPage, "Index");
  Page* stats = new Page("netsysstats", SamplePages::NetStatsPage, "Net System Stats");
  Page* chuck = new Page("chuck", SamplePages::AnotherPage, "Chuck");
  Page* process = new Page("serverstats", SamplePages::ProcessStatsPage, "Server Stats");
  Page* css = new Page("csstest", SamplePages::CSSExamplePage, "Css");
  Page* js = new Page("javascriptexample", SamplePages::JavaScriptExamplePage, "JavaScript");
  site->AddPage(index);
  site->AddPage(stats);
  site->AddPage(chuck);
  site->AddPage(process);
  site->AddPage(css);
  site->AddPage(js);
}



HTTP_STATUS_CODE SamplePages::IndexPage(const PageInfo& info, void* userData_)
{
  Networking::NetworkingSystem* data = SCAST(Networking::NetworkingSystem*, userData_);

  info.page.AddTitle("Index");
  info.page.StartBody();
  istring host = info.rqst.GetHeaderValue("Host");
  istring hyperlnk = info.rqst.GetURI();

  HTML::MakeList lst;
  std::vector<Page*> pages = info.site->GetPages();
  for(u32 i = 0; i < pages.size(); ++i)
    if(!pages[i]->GetURI().empty()) // don't show index page
      lst.push_back(ADDTOLIST(pages[i]->GetPageName(), "http://" + host + "/" + pages[i]->GetURI()));

  info.page.CreateList(lst, false);
  info.page.AddImage("web/CloudyCloudsWow.png", true);

  info.page.AddHyperLink("http://www.google.com", "THE Googs!!", true);
  return OK;
}

HTTP_STATUS_CODE SamplePages::NetStatsPage(const PageInfo& info, void* userData_)
{
  info.page.AddTitle("Net Sys Stats");
  info.page.StartBody();

  Networking::NetworkingSystem* data = SCAST(Networking::NetworkingSystem*, userData_);

  s32 connections = data->NumConnections();
  s32 maxMsg = data->GetMaxDataSize();

  istring conMsg = "Connected: " + std::to_string(connections);
  istring bandMsg = "MaxMessageSize: " + std::to_string(maxMsg);
  info.page.AddBoldLine("--General Info--", true);
  info.page.AddBoldLine(conMsg, true);
  info.page.AddBoldLine(bandMsg, true);

  info.page.AddBoldLine("--Errors--", true);
  s32 err = data->GetNextError();
  HTML::MakeList errorList;
  while(err != Networking::NetworkingSystemError::OK)
  {
    istring errMsg = "Error >> " + std::to_string(err);
    errorList.push_back(ADDTOLIST(errMsg, ""));
    err = data->GetNextError();
  }
  info.page.CreateList(errorList, true);
  info.page.AddImage("http://imgs.xkcd.com/comics/heartbleed_explanation.png", true);
  return OK;
}



HTTP_STATUS_CODE SamplePages::AnotherPage(const PageInfo& info, void* userData_)
{
  info.page.AddTitle("Chuck's Page");
  //info.page.AddInternalCSS("h2{color:blue; text-align:center;}");
  info.page.AddExternalCSS("web/css/test.css");
  info.page.StartBody();

  info.page.AddEmailForm();

  info.page.AddBoldLine("</br></br></br>Chuck is a kind of stupid!", true);
  info.page.AddText("but fur reel dough..", true);
  info.page.AddTextBox("Your opinion:");
  istring clickOne = "http://www.imdb.com/name/nm0799011/";
  istring clickTwo = "https://www.youtube.com/watch?v=KV-RqPtT2PU";
  static bool yup = true;
  yup = !yup; 
  info.page.AddHyperLink(yup ? clickOne : clickTwo, "More Info on cHuck!", true);

  return OK;
}

HTTP_STATUS_CODE SamplePages::ProcessStatsPage(const PageInfo& info, void* userData_)
{
  info.page.AddTitle("Server Stats");
  info.page.AddMeta("http-equiv=\"refresh\" content=\"5\"");
  //info.page.AddInternalScript("var time = new Date().getTime(); $(document.body).bind(\"mousemove keypress\", function(e) { time = new Date().getTime(); }); function refresh() { if(new Date().getTime() - time >= 11000) window.location.reload(true); else setTimeout(refresh, 10000);} setTimeout(refresh, 10000);");
  info.page.StartBody();
  time(&curTime);

  
  f64 seconds = difftime(curTime, AppStartTime);
  time_t lastRqst;
  time(&lastRqst);
  f64 lastTime = difftime(lastRqst, info.debug.lastRequestTime);

  info.page.AddBoldLine("Server Stats!", true);
  info.page.AddText("-------------------", true);
  info.page.AddBoldLine("Total Run Time: " + std::to_string(SCAST(int, seconds)) + " seconds", true);
  info.page.AddBoldLine("Time Since Last Request: "+ std::to_string(SCAST(int, lastTime)) + " seconds",true);

#ifdef DEBUG
  info.page.AddBoldLine("Last Request: ", true);
  info.page.AddText(info.debug.lastRequest, true);

#endif // DEBUG



  return OK;
}

HTTP_STATUS_CODE SamplePages::CSSExamplePage(const PageInfo& info, void* userData_)
{
  info.page.AddTitle("Earth");
  info.page.AddExternalCSS("web/css/orbit.css");
  info.page.StartBody();
  info.page.AddText("<img id=\"sun\" src=\"http://goo.gl/dEEssP\"> <div id=\"earth-orbit\"> <img id=\"earth\" src=\"http://goo.gl/o3YWu9\"> </div>");

  return OK;
}

HTTP_STATUS_CODE SamplePages::JavaScriptExamplePage(const PageInfo& info, void* userData_)
{
  info.page.AddTitle("JavaScript");
  info.page.AddExternalCSS("web/css/JS_Test.css");
  info.page.StartBody();
  istring text = "<p id=\"test\"> Testing JavaScript function call </p>";
  info.page.AddText(text, true);

  istring button = "<button type=\"button\" onclick=\"DisplayDate()\">Display Date</button>";
  info.page.AddText(button, true);
  info.page.AddExternalScript("web/js/test.js");

  return OK;
}
