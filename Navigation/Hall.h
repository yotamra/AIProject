#ifndef HALL_H
#define HALL_H

#include "MapPart.h"

using namespace CPP;

//for count black pixels algo
#define TOP_BLACK_PIXEL_STRIP_SIZE 180// for summing up all black pixels on the top of the image.
#define MIN_BLACK 10000
#define MAX_BLACK 100000

//for find corridor algo
#define CORRIDOR_TO_FIND_HEIGHT 170
#define CORRIDOR_TO_FIND_WIDTH 120
#define TOP_CORRIDOR 120
#define BOTTOM_CORRIDOR 370
#define TURN_RIGHT_CORR_MIN 540
#define TURN_RIGHT_CORR_MAX 620
#define TURN_LEFT_CORR_MIN 20
#define TURN_LEFT_CORR_MAX 100


enum Direction
{
	LEFT_TURN,
	RIGHT_TURN,
	UNINITIALIZED_DIRECTION
};

class Hall : public MapPart
{
public :
	Hall(Map* mapInstance);
	void buildPath(unsigned int sourceRoom, unsigned int destRoom); //room numbers
	NavigateResultEnum navigate(CPP::Image* colorImage,CPP::Image* depthImage,CPP::Image* resImage, NavInfo& navInfo);

private:

	Direction m_turnDirection;
	double m_turnFactor;

	double countBlackPixelsTop(Image& depthImage);
	bool findCorridorLimits(CPP::Image* image, unsigned int& left, unsigned int& right);

};

#endif