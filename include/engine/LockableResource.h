#pragma once
#include <mutex>


//A class that handles a mutex for an object
//General idiom: Nothing is threadsafe, until you call lock
// I.E. The user controls all the mutex operations
template <typename T>
class LockableResource
{
private:
  T object_;//Needs defined def ctor, copy ctor, dtor, assignment operator
  std::mutex mtx_;//Does this class acually provide mutual exclusion? Test + research required
  bool locked_;//John the locke

  
public:
  typedef T type;
  LockableResource(void);
  LockableResource(const LockableResource<T>& rhs);
  LockableResource(const T& rhs);
  
  ~LockableResource(void);

  LockableResource<T>& operator=(const LockableResource<T>& rhs);
  
  //Operators to access the object
  T& operator*(void);
  T* operator->(void);

  bool IsLocked(void);
  void Lock(void);
  void Unlock(void);
};

/************************************************************************/
/*
    Implementation in the header, because templates
*/
/************************************************************************/
#include "LockableResource_.h"