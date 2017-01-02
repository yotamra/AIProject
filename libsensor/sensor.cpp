#include "sensor_impl.h"

//some other libraries to link when using sensor.cpp
//#pragma comment(lib,"OpenNI.lib")
//#pragma comment(lib,"cv210.lib")
//#pragma comment(lib,"cxcore210.lib")
//#pragma comment(lib,"highgui210.lib")

Sensor* Sensor::create()
{
  return new SensorImpl;
}
