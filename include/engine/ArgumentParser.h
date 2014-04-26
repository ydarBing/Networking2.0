#pragma once

#include <vector>
#include <string>
class ArgumentParser
{
  public:
    ArgumentParser(int argv, char** argc, char delim='-');

    //True if argument present
    bool Argument(std::string) const;
    std::string GetFollowingToken(std::string) const;
  private:
    void ParseArguments(int argv, char** argc);

    char delim_;
    std::vector<std::string> args_;
};