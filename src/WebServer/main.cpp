#include "MainPrecompiled.h"
#include "NetworkingSystem.h"
#include "ArgumentParser.h"

#include "Page.h" // where the webpage is handled, also includes HTTP and HTML stuff
#include "Site.h"
#include "SampleSite.h"


void ServerNewTCPConnect(s32 id)
{
  std::cout << "Got new Connection with id: " << id << std::endl;
}

void ServerHandleDisconnect(s32 id)
{
  std::cout << "Got new disconnect with id: " << id << std::endl;
}


int main(int argv, char** argc)
{
  Site web(CreateSampleSite);

  ArgumentParser parser(argv, argc);

  //If the user requests it, run tests
  if(parser.Argument("--test") || parser.Argument("-t"))
  {

  }

  Networking::NetworkingSystem Net(true);
  HTTP http;
  http.SetCallBack(&Net, &web);  
  Net.Initialize(parser);

  Net.AddCustomProtocol<HTTP>(http, 8012);
  Net.SetNewTCPConnectionFunc(ServerNewTCPConnect);
  Net.SetHandleDisconnectFunc(ServerHandleDisconnect);

  while(true)
  {
    SLEEPMILLI(1);
  }

  return 0;
}
