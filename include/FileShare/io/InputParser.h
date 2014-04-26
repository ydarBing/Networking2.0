//string parser
#pragma once

#include "ThreadSafeQueue.h"

static istring EMPTY_STR ="";

class InputParser
{
  public:
    InputParser(ThreadSafeQueue<istring>& queue);
    ~InputParser();
    
    void Test();
    //throws out all of current line for the next one in queue
    void NextLine();
    
    //these eat the text they return from the line
    s32     GetInt(s32 defaultVal = 0, istring& input = EMPTY_STR);
    istring GetString(const istring& defaultVal = "", istring& input = EMPTY_STR);
    istring GetLine();

    //if delim is the current front character, eats all consecutive delims
    void    EatChar(c08 delim = ' ', istring& input = EMPTY_STR);

  private:
    istring& InitLine(istring& input);

    std::string mLine;
    ThreadSafeQueue<istring>& mQueue;
};
