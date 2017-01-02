#ifndef MAP_ITEM_H
#define MAP_ITEM_H
#include "Common.h"

#define UNVALID -1

typedef enum CellType
{
	EMPTY,
	WALL,
	DOOR,
	TRACK
}CellType;

class MapCell
{

public:
	CellType m_cellType;

	// data relevant only to doors
	bool m_isFound;
	int m_doorNum;
	int m_serialNum;
	Door m_doorCoord;
	bool m_isLeft;

	MapCell() : m_isFound(false), m_cellType(EMPTY), m_doorNum(UNVALID), m_serialNum(UNVALID), m_doorCoord(Door(0,0)),m_isLeft(false){}
	/*MapCell(int cellType): m_isFound(false) 
	{
		m_cellType = (CellType)((unsigned int)cellType);
	}*/

	MapCell(int doorNum, int serialNum,Door coord,bool isLeft): m_cellType(DOOR), m_doorNum(doorNum), 
																   m_serialNum(serialNum), m_doorCoord(coord),
																   m_isFound(false),m_isLeft(isLeft){}
};

#endif