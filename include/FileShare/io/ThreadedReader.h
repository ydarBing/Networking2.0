/////////////////////////////////////////////////
/*
 * ThreadedReader.h
 * Used to read input from the console window for an indefinite amount of time, while
 * allowing the main thread to continue.  It stores input to a supplied thread-safe queue
 * that can be read from at will.
 *
 * Author: Kevin Gray
 * 
 *  Copyright (C) 2014 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#pragma once
#include "ThreadSafeQueue.h"
class ThreadedReader
{
public:
  //Constructors/Destructors
  ThreadedReader(ThreadSafeQueue<istring> *queue);
  ~ThreadedReader();

  //Functions
  void Begin();
  void End();
  bool Reading();
private:
  //Members
  std::thread thread;
  ThreadSafeQueue<istring> *inputQueue;
  bool m_reading;
  //Functions
  void ReadInput();
};