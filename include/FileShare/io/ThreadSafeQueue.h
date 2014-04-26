/////////////////////////////////////////////////
/*
 * ThreadSafeQueue.h
 * A queue that will be safe for use by many threads.
 *
 * Author: Kevin Gray
 * 
 *  Copyright (C) 2014 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#pragma once
#include <queue>

template<typename T>
class ThreadSafeQueue
{
public:
  ThreadSafeQueue()
  {

  }
  ~ThreadSafeQueue()
  {

  }

  //Pushes a new item into the queue
  void Push(T item)
  {
    mutex.lock();
    safeQueue.push(item);
    mutex.unlock();
  }
  //Pops and returns the top of the queue
  T Pop()
  {
    if(safeQueue.empty())
      return NULL;
    mutex.lock();
    T item = safeQueue.front();
    safeQueue.pop();
    mutex.unlock();
    return item;
  }
  //Tells us whether the queue is empty or not
  bool Empty()
  {
    return safeQueue.empty();
  }
private:
  std::queue<T> safeQueue;
  std::mutex mutex;
};