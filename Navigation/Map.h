#ifndef MAP_H
#define MAP_H

#include "prims.h"
#include <list>

#include "MapPart.h"
#include "MapCell.h"
#include "nav_utils.h"
#include "Corridor.h"
#include "Hall.h"
#include <map>

using namespace std;

class Map
{
public:
	void init(string inputMapPath);

	// when the obstacleis identifier 
	// paint the obstacle on the map
	// cal the track (BFS)
	// remove the obstacle
	// if track is not found (current postion and destionation is no connected) track=NULL.
	vector<MapPart*> Map::calcTrack(unsigned int sourceRoom, unsigned int destRoom);
	void setXCoord(unsigned int x);
	void setYCoord(unsigned int y);
	void foundDoor(Door door);
	//bool isDestDoor(int doorNum);

	Matrix<MapCell>* m_map;
	list<Coord>* m_track;
	Coord* m_currPosition;
private:
	void paintPath();
	void initCorridor(Corridor** corr, FILE* inputMap);
	void initHall(FILE* inputMap);

	Corridor* m_leftCorr;
	Corridor* m_rightCorr;
	Hall* m_hall;

	Door m_source;
	Door m_destination;

	map<int, MapCell> m_doorNumToMapCell;
		
};

#endif