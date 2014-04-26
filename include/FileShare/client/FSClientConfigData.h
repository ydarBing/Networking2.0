#pragma once

class FileReader;

class FSClientConfigData
{
public:
  FSClientConfigData();
  ~FSClientConfigData();

  bool ReadConfigFile(const istring &file, FileReader& files);

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
