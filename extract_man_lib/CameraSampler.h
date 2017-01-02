#ifndef H_CAMERA_SAMPLER
#define H_CAMERA_SAMPLER

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
#include "KeyboardThread.h"
#include "Sounds.h"
#include <time.h>
#include "pinpointing.h"
#include "FPS_timer.h"



/*
  This is a thread that maintains the camera. The methods allow recieving pictures from the camera.
*/
class CameraSampler : public Thread
{
public:

  CameraSampler();

  void update(Image& color_frame, Image& depth_frame);
  void update_color_frame(Image& color_frame);
  void update_depth_frame(Image& depth_frame);
  void wait_for_finished_initializing(); //ready to provide images
  virtual void run();

  ~CameraSampler(){};

private:
  void update(); //update internal pointers with frames from the camera


  Sensor* m_cam;
  Image m_color_frame, m_depth_frame;
  boost::shared_ptr<int> p;
  Mutex m_Mutex; //mutex for synchronizing
  FPS_timer m_timer; //timer for measuring frames per second
  volatile bool m_finished_init; //boolean for outputting that we finished initializing (creating the cam sensor object)
  bool m_recored_image;
  bool TakeSampleFromCamera;
  int frameNumber;
  int image_num;
};









#endif