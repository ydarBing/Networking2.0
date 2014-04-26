/////////////////////////////////////////////////
/*
 * Implementation of Address class
 *
 * Author: Harrison Beachey
 * 
 *  Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#include "NetworkingPrecompiled.h"
#include "Address.h"

Address::Address()
{
  SetAddress(127, 0, 0, 1, 0);
}

Address::Address(u08 a, u08 b, u08 c, u08 d, u16 port)
  : port_(port)
{
  SetAddress(a, b, c, d, port);
}

Address::Address(u32 netAddr, u16 port)
  : port_(port)
{
  addr_ = ntohl(netAddr);
  sockAddr_.sin_family = AF_INET;
  sockAddr_.sin_addr.S_un.S_addr = netAddr;
  sockAddr_.sin_port = htons(port);
}

bool Address::operator==(const Address& rhs)
{
  if(rhs.addr_ == addr_ && rhs.port_ == port_)
    return true;
  return false;
}

std::ostream& operator<<(std::ostream& str, const Address& addr)
{
  str << addr.GetStringAddress() << ":" << addr.GetPort();

  return str;
}

void Address::SetAddress(u08 a, u08 b, u08 c, u08 d, u16 port)
{
  SetAddress((a << 24) | (b << 16) | (c << 8) | d, port);
}

void Address::SetAddress(u32 addr, u16 port)
{
  addr_ = addr;
  port_ = port;

  sockAddr_.sin_family = AF_INET;
  sockAddr_.sin_addr.s_addr = htonl(addr_);
  SetAddrPort(port);
}

void Address::SetAddrPort(u16 port)
{
  port_ = port;
  sockAddr_.sin_port = htons(port);
}

u32 Address::GetAddress() const
{
  return sockAddr_.sin_addr.S_un.S_addr;
}

u16 Address::GetPort() const
{
  return port_;
}

istring Address::GetStringAddress() const
{
  istring out;
  
  out.assign(inet_ntoa(sockAddr_.sin_addr));

  return out;
}

const sockaddr_in& Address::GetSendToAddress() const
{
  return sockAddr_;
}
