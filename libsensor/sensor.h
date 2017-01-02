#ifndef sensor_h__
#define sensor_h__

#include <image.h>

// assaf hilla
#include "CFrameManager.h"

//class TargetEvaluator
//{
//public:
//  virtual ~TargetEvaluator() {}
//  virtual double evaluate(CPP::Image image) = 0;
//};


class Sensor
{
public:
  virtual ~Sensor() {}
  virtual bool update(CPP::Image& rgb, CPP::Image& depth) = 0;
  virtual void update(Image & depth) = 0;
  virtual CPP::Image extract_man(CPP::Image colorFrame, CPP::Image depthFrame, TargetEvaluator* te, int& off_x, int& off_y,bool& personFound) = 0;

  static Sensor* create();
};



#endif // sensor_h__
