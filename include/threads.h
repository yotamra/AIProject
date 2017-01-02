#ifndef threads_h__
#define threads_h__

class Thread
{
protected:
  bool    m_Terminate;
  bool    m_Active;
public:
  virtual ~Thread() {}
  virtual void run() = 0;
  void start();
  void terminate()            { m_Terminate=true; }
  bool is_active() const      { return m_Active; }
  bool is_terminating() const { return m_Terminate; }
};

void Delay(unsigned ms);

class Mutex
{
  class MutexImpl* m_Mutex;
public:
  Mutex();
  ~Mutex();
  void enter();
  void leave();
};

class Monitor
{
  Mutex& m_Mutex;
public:
  Monitor(Mutex& m) : m_Mutex(m) { m_Mutex.enter(); }
  ~Monitor() { m_Mutex.leave(); }
};

#define MONITOR(x) Monitor l_Monitor_##__LINE__##(x)
#define SYNCHRONIZED MONITOR(m_Mutex)


#endif // threads_h__
