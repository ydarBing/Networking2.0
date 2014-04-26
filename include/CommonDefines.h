#pragma once

#define SLEEPMILLI(X) std::this_thread::sleep_for(std::chrono::milliseconds(X))

#define SCAST(want, have) static_cast<want>(have)

#define RECAST(want, have) reinterpret_cast<want>(have)

/////////////////
//SUPER HELPFUL FOR VS TODO OUTPUT
#define STRINGIZE_(X) #X
#define STRINGIZE(X) STRINGIZE_(X)
#define TODO(X) \
  __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) "): TODO_" X))

#define DEV_NOTE(X) \
  __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) "): NOTE_" X))

// assertion includes
#include <cassert>
// assertions
#define _USE_CONTROL_ADV_ASSERT_
#ifdef _USE_CONTROL_ADV_ASSERT_
#ifdef _DEBUG
#define assertion(x) \
{ \
  if(!(x)) \
  { \
    std::cout << "Assert " << __FILE__ << ":" << __LINE__ << "(" << #x << ")\n"; \
    __debugbreak(); \
  } \
}
#else
#define assertion(x) {}
#endif//_DEBUG
#else
#define assertion(x) assert(x)
#endif //_USE_CONTROL_ADV_ASSERT_

// static size types
#include <stdint.h>
#include <intrin.h>
#include <limits>

//define basic types
typedef char      c08;

typedef int8_t    s08;
typedef uint8_t   u08;

typedef int16_t   s16;
typedef uint16_t  u16;

typedef int32_t   s32;
typedef uint32_t  u32;

typedef int64_t   s64;
typedef uint64_t  u64;

typedef float     f32;
typedef double    f64;

#include <string>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

#include <thread>
#include <mutex>
#include <chrono>

class Logger
{
public:
  Logger()
  {
    m_outfile.open("logfile.txt", std::ios::out | std::ios::trunc);
    assert(m_outfile.is_open());
  }
  ~Logger()
  {
    m_outfile.close();
  }

  std::ofstream m_outfile;
  std::mutex m_ofstreamLock;
};
extern Logger* logger;
#define PRINT_LOG(statement) logger->m_ofstreamLock.lock(); logger->m_outfile << statement << std::endl; \
  std::cout << statement << std::endl; logger->m_ofstreamLock.unlock();

#define LOG(statement) logger->m_ofstreamLock.lock(); logger->m_outfile << statement << std::endl; logger->m_ofstreamLock.unlock();

#define DPRINT(level, statement) if(debugPrint_ >= level) std::cout << statement << std::endl; \
  logger->m_ofstreamLock.lock(); logger->m_outfile << statement << std::endl; logger->m_ofstreamLock.unlock();

typedef std::string istring;

#define SYNCH_WAIT_TIME 2.0f
#define NEARBY_SYNCH_WAIT_TIME 0.5f
#define DEFAULT_NEARBY_HEURISTIC 10.0f
#define MULTICAST 0
