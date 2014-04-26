#include "MainPrecompiled.h"
#include "FSClientConfigData.h"
#include "FileReader.h"


FSClientConfigData::FSClientConfigData()
{

}

FSClientConfigData::~FSClientConfigData()
{

}

bool FSClientConfigData::ReadConfigFile(const istring &fileDir, FileReader& files)
{
  std::ifstream file(fileDir.c_str(), std::ifstream::in);
  if(file.is_open() == false)
  {
    LOG("Unable to open \"" << fileDir << ".\\");
    return false;
  }

  file >> serverIP;
  file >> serverPort;
  file >> clientUDPport;
  file >> receiveFolder;
  file >> shareFolder;
  file >> defaultBufSize;

  files.ArchieveFilesFromFolder(shareFolder);

  return true;
}
