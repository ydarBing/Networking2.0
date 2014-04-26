#pragma once

class InputParser;
struct ICommand;

namespace Networking{
  class NetworkingSystem;
  class Message;
}

typedef void (*RecieveFunc)(Networking::Message& m, s32 connectionID);
void DefaultRecieve(Networking::Message& m, s32 id);

class ConsoleIO
{
  public:
    ConsoleIO(Networking::NetworkingSystem* net);
    void InitCommands(bool serverCommands);
    ~ConsoleIO();

    void Run(RecieveFunc func = DefaultRecieve, u32 sleepTime = 10);

  private:
    Networking::NetworkingSystem* mNet;
    std::unordered_map<std::string, ICommand*> mCommands;
};