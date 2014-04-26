

#include "MainPrecompiled.h"
#include "ConsoleIO.h"

#include "ServerConsoleCommands.h"
#include "ClientConsoleCommands.h"
#include "NetworkingSystem.h"

#include "ThreadedReader.h"

#include "FSClient.h"
#include "FSServer.h"

extern FSClient* client;
extern FSServer* server;

void DefaultRecieve(Networking::Message& m)
{}

ConsoleIO::ConsoleIO(Networking::NetworkingSystem* Net)
  : mNet(Net)
{
}

ConsoleIO::~ConsoleIO()
{
  for(auto it = mCommands.begin(); it != mCommands.end(); ++it)
    delete it->second;
}

void ConsoleIO::InitCommands(bool hasServerCommands)
{
  // only server allows connections at beginning
  mNet->SetAcceptConnections(hasServerCommands);

  if(!hasServerCommands)
  {
    mCommands["list"]    = l;
    mCommands["show"]    = shw;
    mCommands["get" ]    = get;
  }
    mCommands["enum"]    = e;
    mCommands["connect"] = c;
    mCommands["send"]    = s;
    mCommands["buffsize"]= bs;
    mCommands["dprint"]  = dp;
    mCommands["listen"]  = li;
    mCommands["clumsy"]  = clu;
    mCommands["lag"]     = lag;
}

void ConsoleIO::Run(RecieveFunc recFunc, u32 sleeptime)
{
  ThreadSafeQueue<istring> q;
  ThreadedReader reader(&q);
  InputParser parser(q);
  reader.Begin();
  std::string command;

  while(command != "exit")
  {
    //Maintain the connections (Check for timeouts, bad things, i dunno whatever the fuck.)
    if(client)
      client->Maintain();
    else if(server)
      server->Maintain();
    if(!q.Empty())
    {
      parser.NextLine();
      command = parser.GetString();
      
      auto it = mCommands.find(command);
      if(it != mCommands.end())
      {
        it->second->operator()(parser, mNet);
      }
      else if(command == "exit")
      {
        continue;
      }
      else
      {
        LOG("===Commands are:");
        auto it = mCommands.begin();
        for(; it != mCommands.end(); ++it)
          it->second->Usage(std::cout);
        LOG("exit Exits the program");
      }
    }

    s32 numConnections = mNet->NumConnections();
    for(s32 i = 0; i < numConnections; ++i)
    {
      Networking::Message msg = mNet->Receive(i);
      if(msg.GetError() == Networking::MessageError::OK && msg.GetSize() > 0)
      {
        recFunc(msg, i);
      }
    }

    SLEEPMILLI(sleeptime);
  }
  if(client)
    client->Close();
  if(server)
  {
    //server->Close()
  }
  reader.End();
}