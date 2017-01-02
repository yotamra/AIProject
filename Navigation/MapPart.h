#ifndef MAP_PART_H
#define MAP_PART_H

#include "image.h"

class Map;

enum DirectionEnum
{
	STRAIGHT,
	LEFT,
	RIGHT,
  NONE
};

enum NavigateResultEnum
{
	FOUND_DEST_LEFT,
	FOUND_DEST_RIGHT,
	EXIT_PART,
	CONTINUE,
  CONTINUE_CORR,
  FOUND_OBSTACLE
};

typedef struct NavInfo
{
	double factor;
	DirectionEnum direction; // right/left/straight
} NavInfo;
class MapPart
{
protected :
	Map* m_mapManager;
public :
	MapPart(Map* mapManager)
	{
		m_mapManager = mapManager;
	}
	virtual void buildPath(unsigned int sourceRoom, unsigned int destRoom) = 0;
	virtual NavigateResultEnum navigate(CPP::Image* colorImage,CPP::Image* depthImage,CPP::Image* resImage, NavInfo& navInfo) = 0;
};

#endif