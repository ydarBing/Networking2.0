#include "MainPrecompiled.h"

#include "MasterServer.h"
#include "Service.h"

#include "asteroidsCommon\ArgumentParser.h"

#include "FileShare\io\ThreadedReader.h"
#include "FileShare\io\ThreadSafeQueue.h"

INetService* currentService = nullptr;

int main(int argc, char ** argv, char** env)
{
  istring localIP;

  std::ifstream file("NetServiceConfig.txt", std::ifstream::in);
  if(file.is_open() == false)
  {
    std::cout << "unable to open config" << std::endl;
    return 0;
  }

  file >> localIP;
  file.close();

  ArgumentParser parser(argc, argv);

  ThreadSafeQueue<istring>  ioQueue;
  ThreadedReader consoleIO(&ioQueue);
  consoleIO.Begin();

  INetService* serv = nullptr;
  if(parser.Argument("-s"))
  {
    serv = new MasterServer(localIP);
  }
  else
  {
    serv = new Service(localIP);
  }

  serv->Run(ioQueue);

  consoleIO.End();

  return 0;
}
