/////////////////////////////////////////////////
/*
 * Precompiled header for the networking system
 *  Also includes the common precompied with all defines and stl includes
 *
 * Author: Harrison Beachey
 * 
 *  Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////
 
#ifndef PRECOMPILED_H
#define PRECOMPILED_H

//this will have to change if we can't use c++11
#include <chrono>
#include <thread>
#include <mutex>

#include <map>
#include <list>

#define PLAT_WIN   1
#define PLAT_APPLE 2
#define PLAT_UNIX  3

//find what platform we're on
#if defined(_WIN32)
  #define PLATFORM PLAT_WIN
#elif defined(__APPLE__)
  #define PLATFORM PLAT_APPLE
#else 
  #define PLATFORM PLAT_UNIX
#endif

//headers
#if PLATFORM == PLAT_WIN
  #include <winsock2.h>
  #include <ws2ipdef.h>
  typedef int socklen_t;
  typedef SOCKET Socket;
#define ACCEPT_ERROR INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <fcntl.h>
#define ACCEPT_ERROR -1
#endif

bool SocketErrorCheck(int ret, const std::string& outMessage);
#define ERRCHECK(checkVal, outputMessage, retVal) \
  if(SocketErrorCheck(checkVal, outputMessage) == false) \
  { return retVal; }


///flags
#define F_SERVER 0x0001

//packet header flags
enum PacketHeaderFlags
{
  PHF_SYN = 1 << 0, // sync used in handshake
  PHF_RST = 1 << 1, // reset
  PHF_FIN = 1 << 2, // end of connection
  PHF_WND = 1 << 3, // change in window size
  PHF_HRT = 1 << 4, // heartbeat
  PHF_DBG = 1 << 5, // debug
  PHF_RSN = 1 << 6, // resend
};
// HRT + RST = echo of a heartbeat
// SYN + RST = echo of a syn
// FIN + RST = echo of a fin

#include "CommonDefines.h"

#endif
