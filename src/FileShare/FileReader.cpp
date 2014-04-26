/*
 *  FILE          FileReader.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Goes into directory and list's all of the files currently in it
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */

#include "MainPrecompiled.h"
#include "FileReader.h"

#include <fstream>

#if PLATFORM == PLAT_WIN
  #include <FileAPI.h> // FindFirstFile, FindNextFile
#else
  #include <dirent.h> //dirent   opendir readdir
#endif

FileReader::FileReader():m_openFileSize(0)
{
}

void FileReader::ArchieveFilesFromFolder(istring dir)
{
  if(m_file.is_open())
    m_file.close();
  m_dir = dir;
  ArchiveFiles();
}

FileReader::~FileReader()
{
  if(m_file.is_open())
    m_file.close();
}

bool FileReader::ArchiveFiles(void)
{
#if PLATFORM == PLAT_WIN
  return WindowsRead();
#else
  return UnixRead();
#endif

}

// reads number of bytes from current file pos into buffer
// returns number of bytes read
s32 FileReader::ReadFile(c08* buffer, s32 numBytes)
{
  if(!m_file || buffer == nullptr || numBytes <= 0)
    return 0;

  m_file.read(buffer, numBytes);
  
  return SCAST(s32, m_file.gcount());
}

// opens file and sets size of file
bool FileReader::OpenFile(istring fName)
{
  m_file = std::ifstream((m_dir + "/" + fName), std::ifstream::binary);
  if(!m_file)
    return PrintError(fName);
  // setting file size
  m_file.seekg(0, m_file.end);
  m_openFileSize = m_file.tellg();
  m_file.seekg(0, m_file.beg);

  return true;
}

bool FileReader::OpenFile(u32 fIndex)
{
  return OpenFile(GetFileName(fIndex));
}

const istring FileReader::GetFileName(u32 index)
{
  if(index >= m_files.size())
    return m_files.back().fileName;

  return m_files[index].fileName;
}

const std::vector<FileReader::FileInfo>&  FileReader::GetFiles(void)const
{
  return m_files;
}

bool FileReader::WindowsRead(void)
{
#if PLATFORM == PLAT_WIN
  WIN32_FIND_DATA name;
  HANDLE searchHandle = INVALID_HANDLE_VALUE;
  istring inDir = m_dir + "/*";
  searchHandle = FindFirstFile(inDir.c_str(), &name);

  if( searchHandle == INVALID_HANDLE_VALUE )
    return PrintError(m_dir);

  m_files.clear();
  FileInfo fInfo;
  do
  {
    // Don't list other directories in this directory
    //  (this also makes sure .  and ..  are not included)
    if(name.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
    {
      fInfo.fileName = name.cFileName;
      fInfo.fileSize = (name.nFileSizeHigh * (MAXDWORD+1)) + name.nFileSizeLow;
      m_files.push_back(fInfo);
    }

  }while(FindNextFile(searchHandle, &name) != 0 );

  FindClose(searchHandle);

#endif
  return true;
}

bool FileReader::UnixRead(void)
{
#if PLATFORM != PLAT_WIN
  DIR* dir = opendir(m_dir.c_str());

  if(dir == nullptr) 
    return PrintError(m_dir);
  
  dirent* file = readdir(dir);
  FileInfo fInfo;
  while(file != nullptr) 
  {
    if(file->d_type == DT_REG)
    {
      fInfo.fileName = file->d_name;
      fInfo.fileSize = 0;// if needed add later
      m_files.push_back(fInfo);
    }
    file = readdir(dir);
  }

  closedir (dir);
#endif
  return true;
}

bool FileReader::PrintError(istring badOpen)const
{
  std::cout << "Could not open/find " << badOpen << std::endl;
  return false;
}

const u32 FileReader::NumberOfFiles(void)const
{
  return m_files.size();
}

void FileReader::PrintFiles(void)const
{
  for(u32 i = 0; i < m_files.size(); ++i)
    std::cout << i << ". " << m_files[i].fileName << " : size = " << std::to_string(m_files[i].fileSize) << std::endl;
}

istring FileReader::AllFilesOneString(void)
{
  istring ret;
  for(u32 i = 0; i < m_files.size(); ++i)
  {                //filename\n
    istring temp = m_files[i].fileName + "\n";
    ret += temp;
  }
  return ret;
}

s32 FileReader::UnitTest(void)
{
  s32 retVal = 0;
  FileReader test;
  test.ArchieveFilesFromFolder("ToShare");

  test.PrintFiles();
  istring f1  = test.GetFileName(1);
  istring f2  = test.GetFileName(-1);
  istring ft2 = test.GetFileName(test.GetFiles().size());
  assertion(f2 == ft2);

  test.OpenFile(4);
  char knownSize[50] = {0};
  char knownSize2[50] = {0};
  s32 bytesRead = test.ReadFile(knownSize, 20);
  s32 bytesRead2 = test.ReadFile(knownSize2, 30);

  return retVal;
}