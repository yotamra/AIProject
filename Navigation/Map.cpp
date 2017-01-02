#include "Map.h"
#include <stdio.h>
#include <queue>
#include <string>
using namespace std;

void Map::init(string inputMapPath)
{
	//Build map.
	FILE* inputMap= fopen(inputMapPath.c_str(),"r");
	int width=0, height=0; //map size

	int line = fscanf_s(inputMap, "%d,%d\n",&width, &height); //get map size

	m_map = new Matrix<MapCell>(width, height, MapCell());
	
	initCorridor(&m_leftCorr, inputMap);
	initHall(inputMap);
	initCorridor(&m_rightCorr, inputMap);

	m_currPosition = NULL; //not yet initialized.
	m_source = Door(0,0); //not yet initialized.
	m_destination = Door(0,0); //not yet initialized.
	m_track = NULL; //not yet initialized.

	fclose(inputMap);
}

void Map::initCorridor(Corridor** corridor, FILE* inputMap)
{
	map<int, MapCell>* leftDoorSerialNumToCoord = new map<int, MapCell>();
	map<int, MapCell>* rightDoorSerialNumToCoord =  new map<int, MapCell>();

	int x=0, y=0; //cell coord
	int type=0; //cell type: wall=1, door=2
	int doorNum=0; //door real number
	int serialNum=0; //door serial number
	int doorSide=0; //door side: left=0, right=1

	int lastSerialNum = -1;
	int line = fscanf_s(inputMap, "%d,%d,%d",&x, &y, &type);

	while(type != 0 && line != EOF)
	{
		if (type == 1) //wall
		{
			(*m_map)(y,x).m_cellType = WALL;
			fscanf_s(inputMap, "\n");
		}
		else //door
		{
			fscanf_s(inputMap, ",%d,%d,%d\n", &doorNum, &serialNum, &doorSide);
			MapCell item = MapCell(doorNum, serialNum, Door(x,y),doorSide == 0);
			(*m_map)(y,x) = item;
			
			// add the door to the doors list (to convert dest and source door numbers to serial numbers)
			m_doorNumToMapCell[doorNum] = item;

			// add the door to the corridor door map.
			if (item.m_isLeft) //left
			{
				(*leftDoorSerialNumToCoord)[serialNum] = item;
			}
			else //right
			{
				(*rightDoorSerialNumToCoord)[serialNum] = item;
			}
		
			if (serialNum > lastSerialNum)
			{
				lastSerialNum = serialNum;
			}
		}
		line = fscanf_s(inputMap, "%d,%d,%d",&x, &y, &type);
	}
	(*corridor) = new Corridor(this,leftDoorSerialNumToCoord,rightDoorSerialNumToCoord,lastSerialNum);
}

void Map::initHall(FILE* inputMap)
{
	int x=0, y=0; //cell coord
	int type=0; //cell type: wall=1, door=2

	int line = fscanf_s(inputMap, "%d,%d,%d",&x, &y, &type);

	while(type != 0 && line != EOF)
	{
		(*m_map)(y,x).m_cellType = WALL;
		fscanf_s(inputMap, "\n");
		line = fscanf_s(inputMap, "%d,%d,%d",&x, &y, &type);
	}
	m_hall = new Hall(this);

}

//void Map::init(string inputMapPath)
//{
	//Build map.
	/*FILE* inputMap= fopen(inputMapPath.c_str(),"r");
	int width=0, height=0, x=0, y=0;
	double type=0;
	int serialNum = 0;
	int line = fscanf_s(inputMap, "%d,%d\n",&width, &height);
	map = new Matrix<MapCell>(width, height, MapCell(EMPTY));
	destDoor = doorNum;

	while(line != EOF)
	{
		line = fscanf_s(inputMap, "%d,%d,%lf",&x, &y, &type);

		if (type < 5)
		{
			(*map)(y,x) = MapCell(type);
			fscanf_s(inputMap, "\n");
		}
		// it's a door
		else
		{
			fscanf_s(inputMap, ",%d\n",&serialNum);
			MapCell item = MapCell(type, serialNum, Door(x,y));
			
			// add the door to the map and to the doors list
			(*map)(y,x) = item;
			doorNumToMapItem[(*map)(y,x).m_doorNum] = item;

			// check if this is the destination door
			if(((*map)(y,x).m_doorNum == doorNum))
			{
				destination = new Coord(x,y);
			}
		}

		//((*map)(y,x))=WALL;
	}

	//init source and destination.
	currPosition = new Coord(source);
	track = NULL;

	m_nextDoorNumLeft = 211;
	m_nextDoorNumRight = 208;

	fclose(inputMap);
	*/
//}

void Map::paintPath()
{
	queue<Coord> queue;
	bool found=false;
	Matrix<int> visited(m_map->get_width(), m_map->get_height(), false);
	Matrix<Coord> fathers(m_map->get_width(), m_map->get_height(), Coord(-1,-1));

	//add current position to the queue.
	queue.push(*m_currPosition);
	visited(m_currPosition->second, m_currPosition->first) = true;
	fathers(m_currPosition->second, m_currPosition->first) = *m_currPosition;

	while(!queue.empty() && !found)
	{
		Coord curr = queue.front();
		queue.pop();
		//Insert the neighbors of the current element to the queue and update their father.
		if((curr.second!=0) && 
		   (((*m_map)(curr.second-1, curr.first)).m_cellType != WALL) && !visited(curr.second-1, curr.first))
		{
			//left neighbor.
			queue.push(Coord(curr.first, curr.second-1));
			visited(curr.second-1, curr.first) = true;
			fathers(curr.second-1, curr.first) = curr;
			if (Coord(curr.first, curr.second-1) == m_destination)
			{
				found = true;
			}
		}
		if((curr.second!=(m_map->get_width()-1)) && 
		   (((*m_map)(curr.second+1, curr.first)).m_cellType != WALL) && !visited(curr.second+1, curr.first))
		{
			//right neighbor.
			queue.push(Coord(curr.first, curr.second+1));
			visited(curr.second+1, curr.first) = true;
			fathers(curr.second+1, curr.first) = curr;
			if (Coord(curr.first, curr.second+1) == m_destination)
			{
				found = true;
			}
		}
		if((curr.first!=(m_map->get_height()-1)) && 
		   (((*m_map)(curr.second, curr.first+1)).m_cellType != WALL) && !visited(curr.second, curr.first+1))
		{
			//lower neighbor.
			queue.push(Coord(curr.first+1, curr.second));
			visited(curr.second, curr.first+1) = true;
			fathers(curr.second, curr.first+1) = curr;
			if (Coord(curr.first+1, curr.second) == m_destination)
			{
				found = true;
			}
		}
		if((curr.first!=0) && 
		   (((*m_map)(curr.second, curr.first-1)).m_cellType != WALL) && !visited(curr.second, curr.first-1))
		{
			//upper neighbor.
			queue.push(Coord(curr.first-1, curr.second));
			visited(curr.second, curr.first-1) = true;
			fathers(curr.second, curr.first-1) = curr;
			if (Coord(curr.first-1, curr.second) == m_destination)
			{
				found = true;
			}
		}
		if((curr.first!=0 && curr.second!=0) && 
		   (((*m_map)(curr.second-1, curr.first-1)).m_cellType != WALL) && !visited(curr.second-1, curr.first-1))
		{
			//left-upper corner neighbor.
			queue.push(Coord(curr.first-1, curr.second-1));
			visited(curr.second-1, curr.first-1) = true;
			fathers(curr.second-1, curr.first-1) = curr;
			if (Coord(curr.first-1, curr.second-1) == m_destination)
			{
				found = true;
			}
		}
		if(curr.first!=0 && (curr.second!=(m_map->get_width()-1)) && 
		   (((*m_map)(curr.second+1, curr.first-1)).m_cellType != WALL) && !visited(curr.second+1, curr.first-1))
		{
			//right-upper corner neighbor.
			queue.push(Coord(curr.first-1, curr.second+1));
			visited(curr.second+1, curr.first-1) = true;
			fathers(curr.second+1, curr.first-1) = curr;
			if (Coord(curr.first-1, curr.second+1) == m_destination)
			{
				found = true;
			}
		}
		if(curr.first!=(m_map->get_height()-1) && (curr.second!=0) && 
		   (((*m_map)(curr.second-1, curr.first+1)).m_cellType != WALL) && !visited(curr.second-1, curr.first+1))
		{
			//left-lower corner neighbor.
			queue.push(Coord(curr.first+1, curr.second-1));
			visited(curr.second-1, curr.first+1) = true;
			fathers(curr.second-1, curr.first+1) = curr;
			if (Coord(curr.first+1, curr.second-1) == m_destination)
			{
				found = true;
			}
		}
		if(curr.first!=(m_map->get_height()-1) && (curr.second!=(m_map->get_width()-1)) && 
		   (((*m_map)(curr.second+1, curr.first+1)).m_cellType != WALL) && !visited(curr.second+1, curr.first+1))
		{
			//right-lower corner neighbor.
			queue.push(Coord(curr.first+1, curr.second+1));
			visited(curr.second+1, curr.first+1) = true;
			fathers(curr.second+1, curr.first+1) = curr;
			if (Coord(curr.first+1, curr.second+1) == m_destination)
			{
				found = true;
			}
		}
	}
	printf("found = %d\n", found);
	
	
	//calc shortest path from current position to destination.
	if(!found)
	{
		//current position and destionation coords are not connected. 
		m_track = NULL;
	}
	else
	{
		//delete previous track.
		if (m_track!=NULL)
		{
			free(m_track);
		}
		//create the new track acording the BFS results.
		m_track = new list<Coord>();
		m_track->push_front(m_destination);
		Coord curr = m_destination;
		while(curr != *m_currPosition)
		{
			m_track->push_front(fathers(curr.second, curr.first));
			curr = fathers(curr.second, curr.first);
		}
	}
}

vector<MapPart*> Map::calcTrack(unsigned int sourceRoom, unsigned int destRoom)
{
	//init source and dest coords (for paintPath).
	m_source = m_doorNumToMapCell[sourceRoom].m_doorCoord;
	m_destination = m_doorNumToMapCell[destRoom].m_doorCoord;

	vector<MapPart*> track;

	// calc track for all parts
	m_leftCorr->buildPath(sourceRoom,destRoom);
	m_rightCorr->buildPath(sourceRoom,destRoom);
	m_hall->buildPath(sourceRoom,destRoom);

	// build the list of MapParts

	// find source
	MapPart* sourceCorridor;
	if (m_leftCorr->isSourceInside())
	{
		sourceCorridor = m_leftCorr;
		track.push_back(m_leftCorr);
	}
	else if (m_rightCorr->isSourceInside())
	{
		sourceCorridor = m_rightCorr;
		track.push_back(m_rightCorr);
	}

	this->foundDoor(m_source);

	// find dest
	MapPart* destCorridor;
	if (m_leftCorr->isDestInside())
	{
		destCorridor = m_leftCorr;
	}
	else if (m_rightCorr->isDestInside())
	{
		destCorridor = m_rightCorr;
	}

	// check if the src and dest in the same corridor
	if (destCorridor != sourceCorridor)
	{
		// add the hall
		track.push_back(m_hall);

		// add the dest
		track.push_back(destCorridor);
	}
	
	bool isLeft = m_doorNumToMapCell[sourceRoom].m_isLeft;
	if (sourceCorridor == m_leftCorr)
	{
		isLeft = !isLeft;
	}

	if (isLeft)
	{
		m_currPosition = new Coord(m_source.first+5,m_source.second);
	}
	else
	{
		m_currPosition = new Coord(m_source.first-5,m_source.second);
	}


	// paint on the map the track
	this->paintPath();


	return track;
}

void Map::setXCoord(unsigned int x)
{
	m_currPosition->first = x;
}

void Map::setYCoord(unsigned int y)
{
	m_currPosition->second = y;
}
void Map::foundDoor(Door door)
{
	(*m_map)(door.second,door.first).m_isFound = true;
}
//bool Map::updatePosition(Coord newPosition)
//{
//	*currPosition = newPosition;
//
//	return (*currPosition == *destination);
//}

//void Map::getMap(Matrix<MapCell>*& m, Coord& dest, Coord& curr, list<Coord>*& shortestTrack)
//{
//	m = m_map;
//	//dest = *destination;
//	curr = *m_currPosition;
//	shortestTrack = m_track;
//}
//void Map::getMap(Coord& dest,
//				Coord& curr,
//				list<Coord>*& shortestTrack)
//{
//	//dest = *destination;
//	curr = *m_currPosition;
//	shortestTrack = m_track;
//}

//for debug
//void Map::displayMap()
//{
//	Matrix<MapCell> m(m_map->get_width(), m_map->get_height(), MapCell());
//
//	for(int i=0; i<m_map->get_width(); i++)
//	{
//		for(int j=0; j<m_map->get_height(); j++)
//		{
//			m(i,j) = (*m_map)(i,j);
//			
//		}
//	}
//
//	if(track != NULL)
//	{
//		list<Coord>::iterator i;
//		for(i = track->begin(); i != track->end(); ++i)
//		{
//			Coord curr = *i;
//			(m(curr.second,curr.first)).m_cellType = TRACK;
//		}
//	}
//	else
//	{
//		printf("track not found!\n");
//	}
//
//	for (int i=0; i < m_map->get_height(); i++)
//	{
//		for(int j=0; j < m_map->get_width(); j++)
//		{
//			printf("%d ", m(j,i));
//		}
//		printf("\n");
//	}
//}
//bool Map::isDestDoor(int doorNum)
//{
//	return (m_destDoorNum == doorNum);
//}