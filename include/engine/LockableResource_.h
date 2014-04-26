/************************************************************************/
/*
    Public ctors, copy ctors, assignment operators, etc...
*/
/************************************************************************/
template <typename T>
LockableResource<T>::LockableResource(void) :
  object_(),
  mtx_(),
  locked_(false)
{
  //Let everthing default construct
}

template <typename T>
LockableResource<T>::LockableResource(const T& rhs) :
  object_(rhs),
  mtx_(),
  locked_(false)
{
}

template <typename T>
LockableResource<T>::LockableResource(const LockableResource<T>& rhs) :
  object_(rhs.object_),
  mtx_(),//Just create a new mutex, since the resource is technically a new resource
  locked_(false)
{
  //should never copy
  bool shouldNeverCopy = false;
  assert(shouldNeverCopy);
}

template <typename T>
LockableResource<T>::~LockableResource(void)
{
  //If currently locking mutex, unlock it and continue
  //Maybe some debug print is necessary here, since you should never
  //Be in a position to destruct a resource that currently owns a mutex
  if(locked_)
    mtx_.unlock();

  //Let everything destruct naturally
}

template <typename T>
LockableResource<T>& LockableResource<T>::operator=(const LockableResource<T>& rhs)
{
  object_ = rhs.object_();
}
  
/************************************************************************/
/*
    Functions to access the object inside the class
*/
/************************************************************************/
template <typename T>
T& LockableResource<T>::operator*(void)
{
  return object_; 
}

template <typename T>
T* LockableResource<T>::operator->(void)
{
  return &object_;
}

/************************************************************************/
/*
    Mutex management functions
*/
/************************************************************************/
template <typename T>
bool LockableResource<T>::IsLocked(void)
{
  return locked_;
}

template <typename T>
void LockableResource<T>::Lock(void)
{
  //[TODO]: Assert unlocked before the locking operation
  mtx_.lock();
  assert(!locked_);
  locked_ = true;
}

template <typename T>
void LockableResource<T>::Unlock(void)
{
  //[TODO]: Assert that the resource is locked before unlocking operation
  assert(locked_);
  locked_ = false;
  mtx_.unlock();
}
