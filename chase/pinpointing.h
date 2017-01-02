#ifndef pinpointing_h__
#define pinpointing_h__

#include "math.h"
#include "InputConstants.h"
#include <iostream>
#include "CFrameManager.h"
#include "CEdgeDetector.h"
#include "cc.h"
#include "image.h"
#include "hsv.h"
#include <map>
using namespace std;
using namespace CPP;

/*
Finds the hight of the lowest pixel that differs the feet from the floor.
Returns a positive number if it is higher than wanted,
negative if lawer than wanted
and zero if perfectly pinpointed.
The abs of the return value represents how far it is high or low than wanted.
*/
void current_position_comparing_optimal(Image& colorImage,Image& depthImage);

#endif