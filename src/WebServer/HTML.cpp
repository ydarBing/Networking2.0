/*
 *  FILE          HTML.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Makes puts strings into HTML format
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "MainPrecompiled.h"
#include "HTML.h"

static const istring EndLine = "\r\n";
HTML::HTML(): m_inBody(false), m_addedContent(false)
{
  m_head = "<!DOCTYPE html>" + EndLine;
  m_head += "<html>" + EndLine + "<head>";
}

HTML::~HTML()
{
}


istring HTML::AddTitle(istring title)
{
  if(m_inBody) // title's can only be in the head
    return "";

  SetAddedContent(true);

  istring t = "<title>" + title + "</title>";
  m_head += t;

  return t;
}


istring HTML::AddMeta(istring meta)
{
  istring met = "<meta " + meta + ">";
  m_head += met;

  return met;
}

void HTML::StartBody(void)
{
  m_body.clear();
  m_inBody = true;
  m_body = "<body>";
}

istring HTML::Finish(void)
{
  m_head += "</head>" + EndLine;
  m_combined = m_head + m_body;

  m_combined += EndLine + "</body>" + EndLine + "</html>";

  return m_combined;
}

istring HTML::AddBoldLine(istring bold, bool addNewLine /*false*/)
{
  SetAddedContent(true);

  istring bLine = "<b>" + bold + "</b>";

  if(addNewLine)
    AddNewLine(bLine);

  if(m_inBody)
    m_body += bLine;
  else
    m_head += bLine;

  return bLine;
}

istring HTML::AddHyperLink(istring link, istring name, bool addNewLine /*false*/)
{
  SetAddedContent(true);
  istring hLink = "<a href=\"" + link + "\">" + name + "</a>";

  if(addNewLine)
    AddNewLine(hLink);

  if(m_inBody)
    m_body += hLink;
  else
    m_head += hLink;

  return hLink;
}


istring HTML::CreateList(MakeList sList, bool ordered /*= false*/)
{
  static const istring entryStart = "<li>";
  static const istring entryEnd = "</li>";

  SetAddedContent(true);

  istring type = ordered ? "<ol>" : "<ul>";
  istring typeEnd = ordered ? "</ol>" : "</ul>";

  istring l = type;
  for(u32 i = 0; i < sList.size(); ++i)
  {
    if(sList[i].second.empty())
      l += entryStart + sList[i].first + entryEnd;
    else
    {
      istring hLink = "<a href=\"" + sList[i].second + "\">" + sList[i].first + "</a>";
      l += entryStart + hLink + entryEnd;
    }
  }
  l += typeEnd;

  if(m_inBody)
    m_body += l;
  else
    m_head += l;

  return l;
}

istring HTML::AddImage(istring image, bool addNewLine)
{
  SetAddedContent(true);
  istring img = "<img src=\"" + image + "\">";

  if(addNewLine)
    AddNewLine(img);

  m_body += img;

  return img;
}

istring HTML::AddText(istring text, bool addNewLine /*false*/)
{
  SetAddedContent(true);
  if(addNewLine)
    AddNewLine(text);

  if(m_inBody)
    m_body += text;
  else
    m_head += text;

  return text;
}

void HTML::AddNewLine(istring& line)
{
  line += "</br>";
}

istring HTML::AddTextBox(istring boxName)
{
  if(m_inBody == false)
    return "";

  SetAddedContent(true);

  istring box = "<form action=\"\">";
  box += boxName + "<input type=\"text\"" + "name=\"" + boxName + "\"><br></form>";

  m_body += box;

  return box;
}

istring HTML::AddEmailForm(void)
{
  if(m_inBody == false)
    return "";
  SetAddedContent(true);

  istring eForm = "<h2>Send e-mail to someone@example.com:</h2>";
  eForm += "<form action=\"MAILTO:someone@example.com\" method=\"post\" enctype=\"text/plain\">";
  eForm += "Name:<br><input type=\"text\" name=\"name\" value=\"your name\"><br>";
  eForm += "E-mail:<br><input type=\"text\" name=\"mail\" value=\"your email\"><br>";
  eForm += "Comment:<br><input type=\"text\" name=\"comment\" value=\"your comment\" size=\"50\"><br><br>";
  eForm += "<input type=\"submit\" value=\"Send\">";
  eForm += "<input type=\"reset\" value=\"Reset\">";  
  eForm += "</form>";

  m_body += eForm;

  return eForm;        
}

bool HTML::AddedContent(void) const
{
  return m_addedContent;
}

void HTML::SetAddedContent(bool param1)
{
  m_addedContent = param1;
}

void HTML::AddExternalCSS(istring link)
{
  m_head += "<link rel=\"stylesheet\" type=\"text/css\" href=\"" + link + "\">" + EndLine;
}

void HTML::AddInternalCSS(istring css)
{
  m_head += "<style>" + css + "</style>" + EndLine;
}

void HTML::AddExternalScript(istring src)
{
  istring script = "<script src=\"" + src + "\"></script>" + EndLine;
  if(m_inBody)
    m_body += script;
  else
    m_head += script;
}

void HTML::AddInternalScript(istring js)
{
  istring script = "<script>" + js + "</script>" + EndLine;
  if(m_inBody)
    m_body += script;
  else
    m_head += script;
}

//istring HTML::Add(istring line, TAG tag)
//{
//  istring ret;
//
//  ret = tags[tag];
//  ret += line;
//  ret += tags[tag + 1];
//}
