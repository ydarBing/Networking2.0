#pragma once

class FileReader;

class FSServerConfigData
{
public:
  FSServerConfigData();
  ~FSServerConfigData();
  
  bool ReadConfigFile(const istring &file);

  //These should be encapsulated, but I'm lazy and will fix later,
  //cause there are cooler things to work on.
  istring serverIP;
  s32     serverPort;
  s32     clientUDPport;
  istring receiveFolder;
  istring shareFolder;
  s32     defaultBufSize;//in bytes
private:

};