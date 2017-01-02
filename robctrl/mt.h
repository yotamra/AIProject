#ifndef mt_h__
#define mt_h__

// Critical section mutual exclusion class
class Mutex
{
  CRITICAL_SECTION cs;
public:
  Mutex() { InitializeCriticalSection(&cs); }
  ~Mutex() { DeleteCriticalSection(&cs); }
  void lock() { EnterCriticalSection(&cs); }
  void unlock() { LeaveCriticalSection(&cs); }
};

// Critical section monitor
class Monitor
{
  Mutex& m_Mutex;
public:
  Monitor(Mutex& m) : m_Mutex(m) { m_Mutex.lock(); }
  ~Monitor() { m_Mutex.unlock(); }
};

// Unnamed monitor using a given Mutex
#define MONITOR(x) NIT::Monitor l_Mutex_##__LINE__(x)

// Use a default monitor variable
#define SYNCHRONIZED MONITOR(m_Mutex)

#endif // mt_h__
