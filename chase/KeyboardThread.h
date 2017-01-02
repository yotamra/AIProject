#ifndef KeyboardThread_h__
#define KeyboardThread_h__

#include "../libsensor/sensor.h"
#include <iomanip>
#include <cmdline.h>
#include <files.h>
#include <smosvm.h>
#include <feature_manager.h>
#include <profiler.h>
#include <statistics.h>
#include <threads.h>
#include <conio.h>
#include "moments.h"
#include "../robctrl/robintr.h"
#include "clseval.h"
#include "Sounds.h"

//Most of these includes might be useless

class KeyboardThread : public Thread
{
  bool m_Paused;//the robot is in pause, stands and waits for resume.
  bool m_Record; //the robot records color and depth images and saves them for later use.
  bool m_Initialize;//the robot is trying to initialize the person to follow.
  bool m_pinpoint; //the robot is helping the user adjust the camera
public:
  /*
  In the beginning, the robot is on "paused" mode.
  */
  KeyboardThread() 
    : m_Paused(true),
      m_Record(false),
	  m_Initialize(false),
    m_pinpoint(false)
  {}

  bool paused() const { return m_Paused; }
  bool recording() const { return m_Record; }
  bool init() const { return m_Initialize; }
  bool pinpointing() const { return m_pinpoint;}

  void stopInit() { m_Initialize = false;m_Paused=false; }
  void stop_pinpointing() { m_pinpoint = false; }

  virtual void run();
  
};

#endif