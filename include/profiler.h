#ifndef H_PROFILER
#define H_PROFILER

#include <windows.h>

struct Profiler
{
  LARGE_INTEGER start;
  Profiler()
  {
    QueryPerformanceCounter(&start);
  }
 
  double get() const
  {
    LARGE_INTEGER li,freq;
    QueryPerformanceCounter(&li);
    li.QuadPart-=start.QuadPart;
    li.QuadPart*=1000;
    QueryPerformanceFrequency(&freq);
    li.QuadPart/=freq.QuadPart;
    return li.QuadPart*0.001;
  }

  void print(std::ostream& os, const char* name) const
  {
    LARGE_INTEGER li,freq;
    QueryPerformanceCounter(&li);
    li.QuadPart-=start.QuadPart;
    li.QuadPart*=1000;
    QueryPerformanceFrequency(&freq);
    li.QuadPart/=freq.QuadPart;
    os << li.QuadPart << " ms  for " << name << std::endl;
  }
};

class SectionProfiler : public Profiler
{
  const char* m_Name;
public:
  SectionProfiler(const char* name) : m_Name(name) {}
  ~SectionProfiler()
  {
    print(std::cout,m_Name);
  }
};

#define PROFILER(x) SectionProfiler l_Prof_##__LINE__ (#x)


#endif // H_PROFILER