
#include "MainPrecompiled.h"
#include "ClientConsoleCommands.h"
#include "FileReader.h"
#include "NetworkingSystem.h"
#include "Connection.h"
#include "FSClient.h"

extern FSClient* client;

Connect* c = new Connect;
List* l = new List;
Enumerate* e = new Enumerate;
Send* s = new Send;
Listen* li = new Listen;
Clumsy* clu = new Clumsy;
Lag* lag = new Lag;
BufferSize* bs = new BufferSize;
DebugPrint* dp = new DebugPrint;
Show* shw = new Show;
Get *get = new Get;

static const s32 iKnowWhatMyConnectionIDIsBecauseImJustTestingDontReallyAssumeThisNumber = 0;

/////////Enumerage command
void Enumerate::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  net->PrintConnections();
}

void Enumerate::Usage(std::ostream& stream)
{
  stream << "enum ------ enumerates all connections" << std::endl;
}

/////////list command
void List::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  client->List();
}

void List::Usage(std::ostream& stream)
{
  stream << "list ------ Sends filenames to connection" << std::endl;
}

/////////////connect command
void Connect::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring ip = p.GetString("localhost");
        
  s32 port = p.GetInt(8000);
  istring proto = p.GetString("udp");

  s32 a = 127, b = 0, c = 0, d = 1;
  if(ip != "localhost")
  {
    a = p.GetInt(a, ip);
    b = p.GetInt(b, ip);
    c = p.GetInt(c, ip);
    d = p.GetInt(d, ip);
  }

  Address addr(a, b, c, d, port);
  LOG("Connect command recieved to: " << addr 
    << " " << proto);

  bool udp = proto == "udp";
  net->Connect(addr, udp);
}

void Connect::Usage(std::ostream& stream)
{
  stream << "connect --- Connects to remote host" << std::endl;
  stream << USAGE << "connect [ip|localhost] [port] [udp|tcp]" << std::endl;
  stream << SPACER << "Ex: connect localhost 8000 udp" << std::endl;
}

///////////send command
void Send::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring str = p.GetLine();
  //send input after send command
  if(str.size() > 0)
    net->Send(str, iKnowWhatMyConnectionIDIsBecauseImJustTestingDontReallyAssumeThisNumber);
}

void Send::Usage(std::ostream& stream)
{
  stream << "send ------ Sends text to connection" << std::endl;
  stream << USAGE << "send [text]" << std::endl;
  stream << SPACER << "Ex: send Hello World!" << std::endl;
}

//Toggle server mode
//Choose udp port to listen on
///////////Listen command
void Listen::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring protocol = p.GetString();
  istring param = p.GetString();

  if(protocol == "udp")
  {
    //Assume param is port num and open listener on that port
    net->acceptConnections_ = true;
    u16 port = (u16)std::stoi(param);
    net->CreateUdpListener(port); 
  }
  else if(protocol == "tcp")
  {
    if(param == "off")
      net->acceptConnections_ = false;//Turn off tcp listening
    else if(param == "on")
      net->acceptConnections_ = true;//turn on tcp listening
  }
}

void Listen::Usage(std::ostream& stream)
{
  stream << "listen ---- Listen for incoming connections" << std::endl;
  stream << USAGE << "listen [udp|tcp] (port)|(off/on)" << std::endl;
  stream << SPACER << "Ex: listen udp 8003" << std::endl;
  stream << SPACER << "Ex: listen tcp on" << std::endl;
  stream << SPACER << "Ex: listen tcp off" << std::endl;
}

//Drop packets
void Clumsy::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  s32 chance = p.GetInt();
  f32 percent = min(max(0.0f, chance/10.0f), 100.0f);

  Connection::packetLoss_ = percent;
}

void Clumsy::Usage(std::ostream& stream)
{
  stream << "clumsy ---- Intentionally drop a percentage of packets" << std::endl;
  stream << USAGE << "clumsy [percent {0 - 1000}]" << std::endl;
  stream << SPACER << "Ex: clumsy 80" << std::endl;
  stream << SPACER << "Ex: clumsy 0" << std::endl;
  stream << SPACER << "WARNING: If you drop 16 packets in a row," << std::endl;
  stream << SPACER << "messages will be forgotten unless they must send" << std::endl;
}

//Lag packets
void Lag::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  s32 userTime = p.GetInt();
  u32 time = min(max(0, userTime), 9999);

  net->SetLag(time);
}

void Lag::Usage(std::ostream& stream)
{
  stream << "lag ------ Artificially lag packets" << std::endl;
  stream << USAGE << "lag [time {0 - 9999}]" << std::endl;
  stream << SPACER << "Ex: lag 100" << std::endl;
  stream << SPACER << "Ex: lag 0" << std::endl;
}

//Set static/dynamic window size
void BufferSize::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring buffType = p.GetString();

  if(buffType == "dynamic")
  {
  }
  else
  {
    s32 newMaxPacketLen = p.GetInt();
    net->SetBufferSize(newMaxPacketLen);
    LOG("Set buffer length to " << net->GetBufferSize());
  }
}

void BufferSize::Usage(std::ostream& stream)
{
  stream << "buffsize --- Change the size of the send buffer (in bytes)" << std::endl;
  stream << USAGE << "buffersize [dynamic|static] ()|(value)" << std::endl;
  stream << SPACER << "Ex: buffersize dynamic" << std::endl;
  stream << SPACER << "Ex: buffersize static 1000" << std::endl;
}

//Debug print
void DebugPrint::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring str = p.GetString();

  if(str == "on")
  {
    Connection::debugPrint_ = 1;
    Networking::NetworkingSystem::debugPrint_ = 1;
  }
  else if(str == "off")
  {
    Connection::debugPrint_ = 0;
    Networking::NetworkingSystem::debugPrint_ = 0;
  }
  else if(str == "overkill")
  {
    Connection::debugPrint_ = 2;
    Networking::NetworkingSystem::debugPrint_ = 2;
  }
}

void DebugPrint::Usage(std::ostream& stream)
{
  stream << "dprint ----- Print debug messages" << std::endl;
  stream << USAGE << "dprint [on|off|overkill]" << std::endl;
}

//Show available files from server
void Show::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring show = "show|";
  net->Send(show, client->GetServerConn());
}

void Show::Usage(std::ostream& stream)
{
  stream << "show ------ Show files that are available for download" << std::endl;
}

//Get a file 
void Get::operator()(InputParser& p, Networking::NetworkingSystem* net)
{
  istring file = "get|" + p.GetString();
  net->Send(file, client->GetServerConn());
}

void Get::Usage(std::ostream& stream)
{
  stream << "get ------- Download an available file" << std::endl;
  stream << USAGE << "get [filename]" << std::endl;
  stream << SPACER << "Ex: get file.txt" << std::endl;
}
