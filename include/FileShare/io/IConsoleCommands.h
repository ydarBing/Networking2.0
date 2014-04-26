#pragma once

#include "InputParser.h"

namespace Networking{
  class NetworkingSystem;
}

#define SPACER "            "
#define USAGE  "            Usage: "


struct ICommand
{
  virtual void operator()(InputParser&, Networking::NetworkingSystem* net) = 0;
  virtual void Usage(std::ostream&) = 0;
};
