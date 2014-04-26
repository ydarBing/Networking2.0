/*
 *  FILE          HTML.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Makes puts strings into HTML format
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once

#define ADDTOLIST(name, hyperText) std::make_pair(name, hyperText)

class HTML
{
  friend class Site; // site will call finish
public:
  HTML();
  ~HTML();
  void StartBody(void);
  bool AddedContent(void)const;

  //istring Add(istring line, TAG tag);
  void AddExternalCSS(istring link);
  void AddInternalCSS(istring css);

  void AddExternalScript(istring src);
  void AddInternalScript(istring js);

  istring AddImage(istring image, bool addNewLine = false);
  istring AddText(istring text, bool addNewLine = false);
  istring AddTitle(istring title);
  istring AddMeta(istring meta);
  istring AddBoldLine(istring bold, bool addNewLine = false);
  istring AddHyperLink(istring link, istring name, bool addNewLine = false);
  istring AddTextBox(istring boxName);
  istring AddEmailForm(void);
  // for regular lists, the first string is the name
  // and for hyper linked lists, the second string is the hyper link
  typedef std::vector<std::pair<istring,istring>> MakeList;
  
  istring CreateList(MakeList sList, bool ordered = false);
private:
  istring Finish(void);
  void AddNewLine(istring& line);
  void SetAddedContent(bool param1);
public:
  bool    m_addedContent;
  bool    m_inBody;
  istring m_head;
  istring m_body;
  istring m_combined;

};
