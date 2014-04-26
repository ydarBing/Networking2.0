#include "MainPrecompiled.h"
#include "InputParser.h"

InputParser::InputParser(ThreadSafeQueue<istring>& que)
  : mQueue(que)
{}

InputParser::~InputParser()
{}

void InputParser::Test()
{
  {
    mQueue.Push("1 2 3");
    mQueue.Push("54");
    NextLine();
    s32 num = GetInt();
    std::cout << num << " " << std::endl;
    num = GetInt();
    std::cout << num << " " << std::endl;
    num = GetInt();
    std::cout << num << " " << std::endl;

    NextLine();
    num = GetInt();
    std::cout << num << " " << std::endl;
  }
  {
    mQueue.Push("test line");
    mQueue.Push("thisisalong.string-withthingsinit");
    NextLine();
    istring str = GetString();
    std::cout << str << " " << std::endl;
    str = GetString();
    std::cout << str << " " << std::endl;
    NextLine();
    str = GetString();
    std::cout << str << " " << std::endl;
  }
}

void InputParser::NextLine()
{
  if(!mQueue.Empty())
  {
    mLine = mQueue.Pop();
  }
  else
  {
    mLine = EMPTY_STR;
  }
}

s32 InputParser::GetInt(s32 defaultVal, istring& input)
{
  istring& line = InitLine(input);
  if(line == EMPTY_STR)
    return defaultVal;

  s32 ret = defaultVal;

  u32 pos = line.find_first_of(' ', 0);
  if(pos == 0 || pos == istring::npos)
    pos = line.find_first_of('.', 0);
  if(pos == 0)
    return ret;

  ret = std::stoi(line.substr(0, pos));
  line.erase(0, pos);
  EatChar(' ', line);
  EatChar('.', line);
  return ret;
}

istring InputParser::GetString(const istring& defaultVal, istring& input)
{
  istring& line = InitLine(input);
  if(line == EMPTY_STR)
    return defaultVal;

  istring ret = defaultVal;

  u32 pos = line.find_first_of(' ', 0);
  if(pos == 0)
    return ret;

  ret = line.substr(0, pos);
  line.erase(0, pos);
  EatChar(' ', line);
  return ret;
}

istring InputParser::GetLine()
{
  return mLine;
}

void InputParser::EatChar(c08 delim, istring& input)
{
  istring& line = InitLine(input);
  if(line == EMPTY_STR)
    return;

  u32 endPos = line.find_first_not_of(delim, 0);
  if(endPos == 0 || endPos == istring::npos)
    return;

  line.erase(0, endPos);
}

//////////
//private
istring& InputParser::InitLine(istring& input)
{
  if(input != EMPTY_STR)
  {
    return input;
  }

  return mLine;
}