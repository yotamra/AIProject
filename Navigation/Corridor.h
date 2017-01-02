#ifndef CORRIDOR_H
#define CORRIDOR_H

#include "MapCell.h"
#include "MapPart.h"
#include "nav_utils.h"
#include "image.h"
#include <map>

using namespace std;

//#define FRAMES_BETWEEN_DOORS 5
#define CORRIDOR_CENTER 320
#define CORRISOR_CENTER_OFFSET 5
#define DIST_BETWEEN_DOORS 70
	
#define ILLEGAL_COORD 8000
#define DOOR_OFFSET 15
#define MIN_DOOR_RATIO 1.7//2
#define MAX_DOOR_RATIO 4//2.8
#define TOP_CORRIDOR 120
#define BOTTOM_CORRIDOR 370
#define CORRIDOR_HEIGHT 210
#define CORRIDOR_MIN_WIDTH 60
#define BLACK_PIXEL_IN_FLOOR_LINE 30
#define CORRIDOR_WIDTH_IN_HALL 320

enum CorrDirection
{
	FORWARD,
	BACKWARD,
	UNINITIALIZED
};

class Map;

class Corridor : public MapPart
{
public :
	Corridor(Map* mapInstance, map<int, MapCell>* leftDoorsSerialToCoord, 
			 map<int, MapCell>* rightDoorsSerialToCoord,int lastSerialNum);
	void buildPath(unsigned int sourceRoom, unsigned int destRoom); //room numbers
	bool isDestInside();
	bool isSourceInside();
	NavigateResultEnum navigate(CPP::Image* colorImage,CPP::Image* image,CPP::Image* resImage,NavInfo& navInfo);

private:

	int findNextSerialDoorNum(int prevNum, map<int, MapCell>* doorsSerialToCoord);
	bool isDoorNumExist(map<int, MapCell>* doorsSerialToMapCell, int doorNum);
	int initDoorSerialNumber(unsigned int doorRoomNum);
	bool goToNextDoor(CPP::Image* colorImage,map<int, MapCell>* corrDoorsSerialToMapItem,
					  map<int, MapCell>* secondCorrDoorsSerialToMapItem,
					  int& corrNextDoorNum, int& secondCorrNextDoorNum);

	void findDoorInSide(CPP::Image& grayImage, Door& door, bool isLeft);
	void findDoors(CPP::Image* colorImage,unsigned int leftCorridor,unsigned int rightCorridor,
									Door& leftDoor,Door& rightDoor);

	CorrDirection m_direction;

	bool m_findDoors;

	unsigned int m_prevLeftDoor;
	unsigned int m_prevRightDoor;

	unsigned int m_leftDoorCount;
	unsigned int m_rightDoorCount;

	bool m_isLeftFound;
	bool m_isRightFound;

	bool m_destInside;
	bool m_sourceInside;

	int m_destDoor; //serial number of destination door.
	int m_sourceDoor; //serial number of source door. 

	// movment in corridor
	//bool goToNextLeftDoor();
	//bool goToNextRightDoor();

	//serial numbers
	int m_nextDoorNumLeft;
	int m_nextDoorNumRight;
	
	//int m_leftDoorFrameCounter;
	//int m_rightDoorFrameCounter;
	//Door m_prevRightDoor;
	
	//map door serial number to door coord. 
	int m_lastSerialNum;
	map<int, MapCell>* m_leftDoorsSerialToMapItem;
	map<int, MapCell>* m_rightDoorsSerialToMapItem;	
};

#endif