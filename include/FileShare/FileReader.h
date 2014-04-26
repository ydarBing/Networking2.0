/*
 *  FILE          FileReader.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Goes into directory and list's all of the files currently in it
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#pragma once
#include <fstream>

class FileReader
{
public:
  FileReader(); 
  ~FileReader();  
  static s32 UnitTest();

  struct FileInfo
  {
    istring fileName;
    u64     fileSize;
  };

  void ArchieveFilesFromFolder(istring dir);
  const std::vector<FileInfo>& GetFiles(void)const;
  istring AllFilesOneString(void);
  const istring GetFileName(u32 index);
  const u32 NumberOfFiles(void)const;
  bool OpenFile(istring fName);
  bool OpenFile(u32 fIndex);
  s32 ReadFile(c08* buffer, s32 numBytes);
  void PrintFiles(void)const;


private:
  bool ArchiveFiles(void);
  bool PrintError(istring badOpen)const;
  bool WindowsRead(void);
  bool UnixRead(void);
  istring               m_dir;
  std::vector<FileInfo> m_files;
  std::ifstream         m_file;
  std::streamoff        m_openFileSize;
};
