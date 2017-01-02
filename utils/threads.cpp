#include <threads.h>

class ThreadImpl : public Thread
{
public:
  void set_active(bool state) { m_Active=state; }
};

void ThreadMain(void *param)
{
  ThreadImpl* t=reinterpret_cast<ThreadImpl*>(param);
  t->set_active(true);
  t->run();
  t->set_active(false);
}


#ifdef WIN32

#include <windows.h>
#include <process.h>

void Delay(unsigned ms)
{
  Sleep(ms);
}

void Thread::start()
{
  _beginthread(ThreadMain,0,this);
}

class MutexImpl 
{ 
public:
  CRITICAL_SECTION cs;
  MutexImpl() { InitializeCriticalSection(&cs); }
  ~MutexImpl() { DeleteCriticalSection(&cs); }
};

Mutex::Mutex() : m_Mutex(new MutexImpl)
{}

Mutex::~Mutex() 
{ 
  delete m_Mutex;
}

void Mutex::enter() { EnterCriticalSection(&m_Mutex->cs); }
void Mutex::leave() { LeaveCriticalSection(&m_Mutex->cs); }


#endif // WIN32

