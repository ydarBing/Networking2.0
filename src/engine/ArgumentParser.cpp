#include "NetworkingPrecompiled.h"
#include "ArgumentParser.h"

ArgumentParser::ArgumentParser(int argv, char** argc, char delim)
  : delim_(delim)
{
  ParseArguments(argv, argc);
}

bool ArgumentParser::Argument(std::string arg) const
{
  std::vector<std::string>::const_iterator it = args_.cbegin();
  for(; it != args_.end(); ++it)
  {
    if(*it == arg)
      return true;
  }
  return false;
}

std::string ArgumentParser::GetFollowingToken(std::string arg) const
{
  std::vector<std::string>::const_iterator it = args_.cbegin();
  for(; it != args_.end(); ++it)
  {
    if(*it == arg)
      break;
  }

  //If we are at the end of the string, return empty
  if(it == args_.end())
    return "";

  //Otherwise return the found token
  return *it;
}

/////private
void ArgumentParser::ParseArguments(int argv, char** argc)
{
  for(int i = 1; i < argv; ++i)
  {
    std::string s(argc[i]);
    args_.push_back(s);
  }
}