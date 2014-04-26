#include "MainPrecompiled.h"
#include "FSServerConfigData.h"
#include "FileReader.h"


FSServerConfigData::FSServerConfigData()
{

}

FSServerConfigData::~FSServerConfigData()
{

}

bool FSServerConfigData::ReadConfigFile(const istring &fileDir)
{
  std::ifstream file(fileDir.c_str(), std::ifstream::in);
  if(file.is_open() == false)
  {
    std::cout << "Unable to open \"" << fileDir << ".\\" << std::endl;
    return false;
  }

  file >> serverIP;
  file >> serverPort;
  file >> clientUDPport;
  file >> receiveFolder;
  file >> shareFolder;
  file >> defaultBufSize;

  return true;
}
