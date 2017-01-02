#ifndef H_FPS_TIMER
#define H_FPS_TIMER

#include <time.h>
#include <iostream>

/*
This is a timer used in order to print the frames per second.
Use this by calling the sample function inside the loop.
*/
class FPS_timer
{
public:
  FPS_timer(string name)
  {
    m_secs = time(NULL);
    m_counter =0;
    m_name = name;
  }
  FPS_timer()
  {
    m_secs = time(NULL);
    m_counter =0;
    m_name = "Default";
  }

  /*
  prints the last FPS each 5 seconds.
  */
  void sample()
  {
    unsigned long long curr_secs = time(NULL);
    if (curr_secs > (m_secs + 4))
    {
      /*if ( (curr_secs - 1) > m_secs)
      {
      cout << "way too slow!" << endl;
      }*/
      m_secs = curr_secs;
      //cout << m_name << " FPS is: " << m_counter/ (float) 5 << endl;
      m_counter = 0;
    }
    else
    {
      ++m_counter;
    }
  }
private:
  unsigned long long m_secs;
  int m_counter;
  string m_name;
};


#endif