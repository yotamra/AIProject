#ifndef NAV_UTILS
#define NAV_UTILS

#include <threads.h>
//#include "Sensor.h"
#include "image.h"
using namespace CPP;
//#include "Windows.h"

//#include <stdio.h>
//#include <list>
//#include <ostream>
//#include <tchar.h>



#ifdef ROBOT
void play_wave(const char* filename, bool loop);

void play_wav_clip(const char* name);
#endif

#endif