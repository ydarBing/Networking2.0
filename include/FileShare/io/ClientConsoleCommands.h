#pragma once
#include "IConsoleCommands.h"

//Lists the files hosted by current client
struct List : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern List* l;

//Enumerates the connections on the current client
struct Enumerate : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Enumerate* e;

//Connects to an end point
struct Connect : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Connect* c;

//Sends data to an endpoint
struct Send : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Send* s;

//Enables administrative features such as changing the state of the system (changing ports, client/server mode, etc)
struct Listen : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Listen* li;

struct Clumsy : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Clumsy* clu;

struct Lag : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Lag* lag;

struct BufferSize : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern BufferSize* bs;


struct DebugPrint : public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern DebugPrint* dp;

struct Show: public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Show* shw;

struct Get: public ICommand
{
  virtual void operator()(InputParser& p, Networking::NetworkingSystem* net);
  virtual void Usage(std::ostream&);
};
extern Get* get;