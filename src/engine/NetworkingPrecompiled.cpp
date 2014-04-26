#include "NetworkingPrecompiled.h"

bool SocketErrorCheck(int x, const std::string& outMessage)
{
#if PLATFORM == PLAT_WIN
  s32 err = WSAGetLastError();
  if(err == 0 || WSAEWOULDBLOCK)
    return true;
  LPSTR errorStr = 0;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
                nullptr, err, 0, (LPSTR)&errorStr, 0, nullptr);
#else
  return true;
  std::string errorStr = "Linux Error";
  //DO LINUX VERSION using x
#endif

  std::cout << outMessage << std::endl;
  std::cerr << "___NetworkingError: " << errorStr;
  return false;
}
