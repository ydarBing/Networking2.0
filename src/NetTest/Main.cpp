#include "MainPrecompiled.h"

#include "ArgumentParser.h"
#include "NetworkingSystem.h"
#include "Encoder.h"

Networking::NetworkingSystem* Net;
void HandleTCPConnect(s32 connectionID)
{
  std::cout << "Handle tcp connect: " << connectionID << std::endl;
}
void HandleUDPConnect(s32 connectionID)
{
  std::cout << "Handle udp connect: " << connectionID << std::endl;
}

void HandleDisconnect(s32 connectionID)
{
  std::cout << "Handle disconnect: " << connectionID << std::endl;
}

int main(int argv, char** argc)
{
  ArgumentParser parser(argv, argc);

  //If the user requests it, run tests
  if(parser.Argument("--test") || parser.Argument("-t"))
  {
    Connection con;
    con.TestSequenceWrap();
    return 0;
  }

  Networking::BitArray::UnitTest();
  Net = new Networking::NetworkingSystem(true, HandleTCPConnect, HandleUDPConnect, HandleDisconnect);
  int cmdlinecon = Net->Initialize(parser);

  bool isServer = parser.Argument("--server") || parser.Argument("-s");

  Address mCastAddr = Address(226, 0, 0, 2, 8009);
  if(isServer)
  {
    Address localAddr(192, 168, 1, 3, 8010);
    Net->Multicast(localAddr, mCastAddr, false);
    //Net->CreateUdpListener(8010);
    Net->SetLag(300);
    Networking::Message toSend("This is a multicasted Message");
    Net->Send(toSend, MCAST_ID);
    while(true)
      SLEEPMILLI(100);
  }
  else
  {
    Address localAddr(192, 168, 1, 3, 8009);
    Net->Multicast(localAddr, mCastAddr);
    //Net->CreateUdpListener(8006);
    while(true)
    {
      SLEEPMILLI(100);
      Networking::Message m = Net->Receive(MCAST_ID);
      if(m.GetSize() == 0)
        continue;
      istring str;
      str.assign(m.GetBuffer(), m.GetSize());
      std::cout << str << " From: " << Net->GetConnectionAddress(MCAST_ID).GetStringAddress() 
        << ":" << Net->GetConnectionAddress(MCAST_ID).GetPort() << std::endl;
    }
  }
  
  return 0;
}