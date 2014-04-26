#include "MainPrecompiled.h"
#include "NetworkingSystem.h"
#include "Encoder.h"
#include <list>
#include "InputParser.h"
#include "ArgumentParser.h"
#include "FileReader.h"

#include "ConsoleIO.h"
#include "FSClient.h"
#include "FSServer.h"

FSClient *client;
FSServer *server;

void ClientRecieve(Networking::Message& m, s32 id)
{
  client->Receive(m, id);
}

void ServerRecieve(Networking::Message& m, s32 id)
{
  server->Receive(m, id);
}

void ServerNewTCPConnect(s32 id)
{
  server->HandleNewTCPConnection(id);
}

void ServerHandleDisconnect(s32 id)
{
  server->HandleDisconnect(id);
}

void ClientHandleDisconnect(s32 id)
{
  client->HandleDisconnect(id);
}

int main(int argv, char** argc)
{
  ArgumentParser parser(argv, argc);
  
  //If the user requests it, run tests
  if(parser.Argument("--test") || parser.Argument("-t"))
  {
    //Networking::BitArray::UnitTest();
    //FileReader::UnitTest();
  }

  Networking::NetworkingSystem Net(true);
  int cmdlinecon = Net.Initialize(parser);


  bool isServer = parser.Argument("--server") || parser.Argument("-s");
  ConsoleIO console(&Net);
  console.InitCommands(isServer);

  if(isServer)
  {
    server = new FSServer(&Net);
    Net.SetNewTCPConnectionFunc(ServerNewTCPConnect);
    Net.SetHandleDisconnectFunc(ServerHandleDisconnect);
    client = NULL;
    if(server->Initialize())
    {
      console.Run(ServerRecieve, 5);
    }
    delete server;
  }
  else
  {
    client = new FSClient(&Net);
    Net.SetHandleDisconnectFunc(ClientHandleDisconnect);
    server = NULL;
    if(client->Initialize())
    {
      console.Run(ClientRecieve);
    }
    delete client;
  }

  return 0;
}
