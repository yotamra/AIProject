#ifndef H_FALL_DETECTOR
#define H_FALL_DETECTOR

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
#include "CameraSampler.h"
#include "boost/circular_buffer.hpp"

struct History
{
  Image depth_image;
  Image person;
  int person_x;
  int person_y;
  int center_x;
  int center_y;
  unsigned long long secs;
  unsigned int micro;
  History(Image& depth_image, Image& person, int person_x,int person_y, int center_x, int center_y, unsigned long long secs, unsigned int micro) :
    depth_image(depth_image), person(person), person_x(person_x), person_y(person_y), center_x(center_x), center_y(center_y), secs(secs), micro(micro)
  {
  }

  History& operator=(const History& rhs)
  {
    if (this != &rhs)
    {
        this->depth_image = rhs.depth_image;
        this->person = rhs.person;
        this->person_x = rhs.person_x;
        this->person_y = rhs.person_y;
        this->center_x = rhs.center_x;
        this->center_y = rhs.center_y;
        this->secs = rhs.secs;
        this->micro=rhs.micro;
    }

    return *this;
  }
};

class FallDetector
{
public:
  FallDetector(CameraSampler* cam); //constructor that gets the camera sampler thread to get images from
  /*
  The main thread uses this method to provide the person.
  */
  void provide_person(Image& person, bool person_in_frame, int x, int y, int speed1, int speed2); //function used by main thread to provide the data found by it
  void run();
  void pause(); //stops tracking fall
  void resume(); //resumes tracking fall
  void set_person_in_frame_status(bool person_in_frame);

private:
    /*
    updates the fields of the FallDetector object.
    */
    void update_data();
    /*
    check if there is a fall.
    */
    bool detect_fall();
    /*
    saves the depth pictures in the last frames.
    */
    void print_person_location(boost::circular_buffer<History> history);
    /*
    calls print_person_location
    */
    void print_person_location(Image& depth_image, Image& person, int person_x,int person_y, int center_x, int center_y, bool history = false, unsigned long long history_secs = 0, unsigned long long history_micro = 0);
    /*
    clears history information irrelavant afeter falling.
    */
    void clear_buffers();

  Mutex m_Mutex; //mutex for synchronizing
  Image m_person; //person image from main thread
  FPS_timer m_timer; //timer for measuring frames per second
  CameraSampler* m_cam; //camera sampler for depth images
  volatile bool m_person_in_frame; //variable for if there is a person in the frame currently
  int m_person_x, m_person_y ; //person location
  int m_color_border; //border between depth of person and depth of background
  bool m_paused;
  bool m_Provide_Person_Flag;
  bool m_recently_found_person;
  std::ofstream m_log;//the log file for the fall detection
  boost::circular_buffer<Vec2D> m_centers_buffer; //the center of mass in the last depth frames
  boost::circular_buffer<History> m_history;//the last update data usages
  volatile int m_num_lost_person; //number of times the person was not found
  volatile int m_speed1, m_speed2; //last provided speeds
};




#endif