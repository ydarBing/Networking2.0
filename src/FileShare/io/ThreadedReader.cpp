#include "MainPrecompiled.h"
#include "ThreadedReader.h"

ThreadedReader::ThreadedReader(ThreadSafeQueue<istring> *queue) : m_reading(false), inputQueue(queue)
{
  
}

ThreadedReader::~ThreadedReader()
{
  //The thread needs to terminate before ending the program
  if(Reading())
  {
    m_reading = false;
    thread.join();
  }
  //Should thread join be called if not reading also?
}

void ThreadedReader::ReadInput()
{
  istring input;
  while(Reading())
  {
    std::getline(std::cin, input);
    //convert any input to lower case
    //std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    //We got some instructions, put it in the queue and we are done with handling it
    inputQueue->Push(input);
    SLEEPMILLI(1);
  }
}

void ThreadedReader::Begin()
{
  if(Reading())
  {
    LOG("ThreadedReader is already reading!");
    return;
  }
  m_reading = true;
  LOG("Ready to accept input...");
  thread = std::thread(&ThreadedReader::ReadInput, this);
}

void ThreadedReader::End()
{
  m_reading = false;
  std::cout << "Press enter to terminate ThreadedReader..." << std::endl;
  thread.join();
}

bool ThreadedReader::Reading()
{
  return m_reading;
}