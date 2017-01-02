#include "Corridor.h"
#include "Map.h"

#include <vector>
using namespace std;

#include "image.h"
#include "cc.h"
using namespace CPP;

bool Corridor::isDestInside()
{
	return m_destInside;
}

bool Corridor::isSourceInside()
{
	return m_sourceInside;
}

bool Corridor::isDoorNumExist(map<int, MapCell>* doorsSerialToMapCell, int doorNum)
{
	map<int, MapCell>::iterator i;
	for(i=doorsSerialToMapCell->begin(); i!=doorsSerialToMapCell->end(); i++)
	{
		if((*i).second.m_doorNum == doorNum)
		{
			return true;
		}
	}
	return false;
}

int Corridor::initDoorSerialNumber(unsigned int doorRoomNum)
{
	map<int, MapCell>::iterator i;
	for(i=m_leftDoorsSerialToMapItem->begin(); i!=m_leftDoorsSerialToMapItem->end(); i++)
	{
		if((*i).second.m_doorNum == doorRoomNum)
		{
			return (*i).first;
		}
	}
	for(i=m_rightDoorsSerialToMapItem->begin(); i!=m_rightDoorsSerialToMapItem->end(); i++)
	{
		if((*i).second.m_doorNum == doorRoomNum)
		{
			return (*i).first;
		}
	}
	return -1;
}

void Corridor::buildPath(unsigned int sourceRoom, unsigned int destRoom) //real door nums
{
	m_sourceInside = isDoorNumExist(m_leftDoorsSerialToMapItem, sourceRoom) ||
					 isDoorNumExist(m_rightDoorsSerialToMapItem, sourceRoom);

	m_destInside = isDoorNumExist(m_leftDoorsSerialToMapItem, destRoom) ||
				   isDoorNumExist(m_rightDoorsSerialToMapItem, destRoom);

	if (m_sourceInside && m_destInside)
	{
		m_direction = (sourceRoom < destRoom) ? FORWARD : BACKWARD ;
		m_destDoor = initDoorSerialNumber(destRoom);
		m_sourceDoor = initDoorSerialNumber(sourceRoom);
		m_nextDoorNumLeft = findNextSerialDoorNum(m_sourceDoor, m_leftDoorsSerialToMapItem);
		m_nextDoorNumRight = findNextSerialDoorNum(m_sourceDoor, m_rightDoorsSerialToMapItem);
	}
	else if (!m_sourceInside && m_destInside)
	{
		m_direction = FORWARD;
		m_destDoor = initDoorSerialNumber(destRoom);	
		m_nextDoorNumLeft = findNextSerialDoorNum(0, m_leftDoorsSerialToMapItem);
		m_nextDoorNumRight = findNextSerialDoorNum(0, m_rightDoorsSerialToMapItem);
	}
	else if (m_sourceInside && !m_destInside)
	{
		m_direction = BACKWARD;
		m_sourceDoor = initDoorSerialNumber(sourceRoom);
		m_nextDoorNumLeft = findNextSerialDoorNum(m_sourceDoor, m_leftDoorsSerialToMapItem);
		m_nextDoorNumRight = findNextSerialDoorNum(m_sourceDoor, m_rightDoorsSerialToMapItem);
	}

	if(m_sourceInside)
	{
		m_findDoors = true;
	}
	else
	{
		m_findDoors = false;
	}
}


void paintResults(Image* colorImage,unsigned int leftCorridor,
			     unsigned int rightCorridor,DirectionEnum direction)
{
	// paint the coridor boundaries
	CvScalar leftColor;
	leftColor.val[0] = 0;
	leftColor.val[1] = 255;
	CvScalar rightColor;
	rightColor.val[0] = 0;
	rightColor.val[1] = 255;
	unsigned int w = ((CPPImage*)colorImage->get())->get_width();
	unsigned int h = ((CPPImage*)colorImage->get())->get_height();
	
	// paint left and right side
	for (unsigned int i = 0; i < h; ++i)
	{
		cvSet2D(((CPPImage*)colorImage->get())->get(),i,leftCorridor, leftColor);
		cvSet2D(((CPPImage*)colorImage->get())->get(),i,rightCorridor, rightColor);
	}

	// paint the direction
	IplImage *img = ((CPPImage*)colorImage->get())->get();
	if (direction == STRAIGHT)
	{
		CvPoint  curve1[]={290,65,  350,65,  320,30};
		CvPoint* curveArr[1]={curve1};
		int      nCurvePts[1]={3};
		int      nCurves=1;
		int      isCurveClosed=1;
		int      lineWidth=1;

		cvFillPoly(img,curveArr,nCurvePts,nCurves,cvScalar(0, 0, 255, 0));
		//cvPolyLine(img,curveArr,nCurvePts,nCurves,isCurveClosed,cvScalar(0, 0, 255, 0),lineWidth);
	}
	else if (direction == RIGHT)
	{
		CvPoint  curve1[]={320,30,  320,100,  350,65};
		CvPoint* curveArr[1]={curve1};
		int      nCurvePts[1]={3};
		int      nCurves=1;
		int      isCurveClosed=1;
		int      lineWidth=1;

		cvFillPoly(img,curveArr,nCurvePts,nCurves,cvScalar(0, 0, 255, 0));
		//cvPolyLine(img,curveArr,nCurvePts,nCurves,isCurveClosed,cvScalar(0, 0, 255, 0),lineWidth);
	}
	else if (direction == LEFT)
	{
		CvPoint  curve1[]={320,30,  320,100,  290,65};
		CvPoint* curveArr[1]={curve1};
		int      nCurvePts[1]={3};
		int      nCurves=1;
		int      isCurveClosed=1;
		int      lineWidth=1;

		cvFillPoly(img,curveArr,nCurvePts,nCurves,cvScalar(0, 0, 255, 0));
		//cvPolyLine(img,curveArr,nCurvePts,nCurves,isCurveClosed,cvScalar(0, 0, 255, 0),lineWidth);
	}
	
	//// paint floor line.
	//CvScalar color;
	//color.val[0] = 255;
	//for (unsigned int j = 0; j < w; ++j)
	//{
	//	cvSet2D(((CPPImage*)colorImage->get())->get(),floorLine,j, color);
	//}
}
void paintDoor(Door& door,Image* image)
{
	unsigned int h = ((CPPImage*)image->get())->get_height();

	// DEBUG!!
	CvScalar leftColor;
	leftColor.val[1] = 255;
	leftColor.val[0] = 255;

	// paint left and right side
	for (unsigned int i = 0; i < h; ++i)
	{
		if (door.second != 0)
			cvSet2D(((CPPImage*)image->get())->get(),i,door.second, leftColor);
	}
}

void Corridor::findDoorInSide(Image& grayImage, Door& door, bool isLeft) //unsigned int floorLine //unsigned int leftCorridor,unsigned int rightCorridor,
{
	unsigned int w = ((CPPImage*)grayImage.get())->get()->width;
	unsigned int h = ((CPPImage*)grayImage.get())->get()->height;

	door.first = ILLEGAL_COORD;
	door.second = ILLEGAL_COORD;

	if (!m_findDoors)
	{
		return;
	}

	CPPMatrix<double> matrix(7,7);
	//CPPMatrix<double> matrix(1,3);
	matrix.fill(0);

	//matrix.set(0,0,1);
	//matrix.set(0,2,-1);

	
if(isLeft)
	{
		matrix.set(0,6,-3);
		matrix.set(1,5,-2);
		matrix.set(2,4,-1);
		matrix.set(4,2,1);
		matrix.set(5,1,2);
		matrix.set(6,0,3);
		
		matrix.set(0,5,-1.5);
		matrix.set(1,6,-1.5);
		matrix.set(1,4,-0.5);
		matrix.set(2,5,-0.5);

		matrix.set(4,1,0.5);
		matrix.set(5,2,0.5);
		matrix.set(5,0,1.5);
		matrix.set(6,1,1.5);
		/*matrix.set(0,4,-2);
		matrix.set(1,3,-1);
		matrix.set(3,1,1);
		matrix.set(4,0,2);*/
	}
	else
	{
		matrix.set(0,0,3);
		matrix.set(1,1,2);
		matrix.set(2,2,1);
		matrix.set(4,4,-1);
		matrix.set(5,5,-2);
		matrix.set(6,6,-3);

		matrix.set(0,1,1.5);
		matrix.set(1,0,1.5);
		matrix.set(2,1,0.5);
		matrix.set(1,2,0.5);

		matrix.set(5,4,-0.5);
		matrix.set(5,4,-0.5);
		matrix.set(5,6,-1.5);
		matrix.set(6,5,-1.5);
		/*matrix.set(0,0,2);
		matrix.set(1,1,1);
		matrix.set(3,3,-1);
		matrix.set(4,4,-2);*/
	}
	
	cc_list* doors = new cc_list();
	Image calcedImage = filter(grayImage, matrix);
	
	//cvNamedWindow("gray", CV_WINDOW_AUTOSIZE);
	//cvShowImage("gray", ((CPPImage*)calcedImage.get())->get());

	//double sum = 0;
	//int count = 0;
	//byte* row;
	//for (int i = h-1; i >= 0; --i)
	//{
	//	row = ((CPPImage*)calcedImage.get())->get_row(i);
	//	for (unsigned int j = 0; j < w; ++j)
	//	{
	//		//black pixel
	//		if(*row != 0)
	//		{
	//			sum += *row;
	//			++count;
	//		}
	//	}
	//}

	//double avg = sum/count;

	//cout << " avg = " << avg << " max = " << max << endl;
	calcedImage =  threshold(calcedImage, 110);

	//CPP::Kernel f(new CPPKernel(1,150)); //150
	//Image res = dilate(calcedImage,f);
	Image res = calcedImage;
	/* display the map */
	//cvNamedWindow("map1", CV_WINDOW_AUTOSIZE);
	//cvShowImage("map1", ((CPPImage*)calcedImage.get())->get());

	//cvNamedWindow("res", CV_WINDOW_AUTOSIZE);
	//cvShowImage("res", ((CPPImage*)res.get())->get());

	//cvWaitKey(0);

	cc::analyze(&calcedImage, doors, cc::config(false));
	remove_small_cc(*doors, 800);
	if (doors->size() == 0)
	{
		return;
	}

	cc::cc_list::iterator i;
	if(isLeft)
	{
		door.second = 700;
		for(i = doors->begin(); i!=doors->end(); i++)
		{
			//printf("height = %d left = %d\n", (*i).rect.height(), (*i).rect.l);
			if ( ((*i).rect.l < door.second) && ((*i).rect.height() > 200))//&& ((*i).rect.l < 110) )
			{
				//cout << "door depth " << doorDepthDiff << endl;
				door.second = (*i).rect.r;
			}
		}
	}
	else
	{
		door.second = 0;
		for(i = doors->begin(); i!=doors->end(); i++)
		{
			//printf("height = %d right = %d\n", (*i).rect.height(), (*i).rect.r);
			if ( ((*i).rect.r > door.second) && ((*i).rect.height() > 200))// && ((*i).rect.r > (w-110)) )
			{
				//cout << "door depth " << doorDepthDiff << endl;
				door.second = (*i).rect.l;
			}
		}
	}

	delete doors;
	if ( (door.second == 0) || (door.second == 700) )
	{
		door.second = ILLEGAL_COORD;
	}
}


void Corridor::findDoors(CPP::Image* colorImage,unsigned int leftCorridor,unsigned int rightCorridor,
									Door& leftDoor,Door& rightDoor)
{
	leftDoor.second = ILLEGAL_COORD;
	if (leftCorridor > 10)
	{
		Rect leftRect(0,0,leftCorridor,310);
		Image leftImage = crop(*colorImage, leftRect);
		
		findDoorInSide(leftImage, leftDoor, true);
	}

	rightDoor.second = ILLEGAL_COORD;
	if (rightCorridor < 630)
	{
		Rect rightRect(rightCorridor,0,640,310);
		Image rightImage = crop(*colorImage, rightRect);
		
		findDoorInSide(rightImage, rightDoor, false);
		if (rightDoor.second != ILLEGAL_COORD)
		{
			rightDoor.second += rightCorridor-1;
		}
	}
}

//void findDoors(CPP::Image* colorImage,unsigned int floorLine,
//									unsigned int leftCorridor,unsigned int rightCorridor,
//									Door& leftDoor,Door& rightDoor)
//{
//	byte* row;
//
//	leftDoor.first = ILLEGAL_COORD;
//	leftDoor.second = ILLEGAL_COORD;
//	rightDoor.first = ILLEGAL_COORD;
//	rightDoor.second = ILLEGAL_COORD;
//	// discard the left side of the colorImage because it contains black columns
//	int w = ((CPPImage*)colorImage->get())->get_width();
//	int h = ((CPPImage*)colorImage->get())->get_height();
//
//	vector<float> colSum(w);
//	vector<int> colPixelCount(w);
//	for (unsigned int j = 0; j < w; ++j)
//	{
//		colSum[j] = 0;
//		colPixelCount[j]=0;
//	}
//	
//	// discard the floor
//	//for (unsigned int i = 0; i < floorLine; ++i)
//	for (unsigned int i = 170; i < 270; ++i)
//	{
//		row = ((CPPImage*)colorImage->get())->get_row(i);
//		for (unsigned int j = 0; j < w; ++j)
//		{
//			if(*row !=0)
//			{
//				colSum[j] += (*row);
//				colPixelCount[j]++;
//			}
//			++row;
//		}
//	}
//	//calc average.
//	for (unsigned int j = 0; j < w; ++j)
//	{
//		if(colPixelCount[j] != 0)
//		{
//			colSum[j] = colSum[j]/(colPixelCount[j]);
//		}
//	}
//
//	// find the most left ,between the left side of 
//	// pic to the left side of the coridor
//	//int max = leftCorridor-CORRIDOR_OFFSET;
//	//if (max < 0)
//	//{
//	//	max = 0;
//	//}
//
//	//if (max + DOOR_OFFSET > w)
//	//{
//	//	max = w-DOOR_OFFSET;
//	//}
//	unsigned int left = 150;
//	int max = Min(leftCorridor, left);
//
//	for (unsigned int j = 0; j < max;++j)
//	{
//		if(colSum[j]==0 || colSum[j+DOOR_OFFSET]==0)
//		{
//			continue;
//		}
//		double ratio = colSum[j+DOOR_OFFSET]/colSum[j];
//		printf("left ratio: %lf\n", ratio);
//		// pass from the door to it's right side
//		if ((ratio > MIN_DOOR_RATIO) && (ratio < MAX_DOOR_RATIO))
//		{
//			leftDoor = Door(floorLine,j);
//								 
//			break;
//		}
//	}
//
//	//unsigned int min = rightCorridor + CORRIDOR_OFFSET;
//	//if (min > w)
//	//{
//	//	min = w-DOOR_OFFSET;
//	//}
//	unsigned int right = 490;
//	int min = Max(rightCorridor, right);
//
//	// find the most right ,between the right side of 
//	// pic to the right side of the coridor
//	for (unsigned int j = min; j < w-DOOR_OFFSET;++j)
//	{
//		if(colSum[j]==0 || colSum[j+DOOR_OFFSET]==0)
//		{
//			continue;
//		}
//		double ratio = colSum[j]/colSum[j+DOOR_OFFSET];
//		printf("right ratio: %lf\n", ratio);
//		// pass from the door to it's right side
//		if ((ratio > MIN_DOOR_RATIO) && (ratio < MAX_DOOR_RATIO)) 
//		{
//			rightDoor = Door(floorLine,j);
//			break;
//		}
//	}
//}




//unsigned int findFloor(CPP::Image* image)
//{
//	byte* row;
//	// discard the left side of the image because it contains black columns
//	int w = ((CPPImage*)image->get())->get_width()-10;
//	int h = ((CPPImage*)image->get())->get_height();
//
//	int rowBlackCount = 0;
//
//	for (int i = h-1; i >= 0; --i)
//	{
//		row = ((CPPImage*)image->get())->get_row(i);
//		rowBlackCount = 0;
//		for (unsigned int j = 0; j < w; ++j)
//		{
//			//black pixel
//			if(*row == 0)
//			{
//				rowBlackCount++;
//			}
//			++row;
//		}
//		if(rowBlackCount > BLACK_PIXEL_IN_FLOOR_LINE)
//		{
//			return i; 
//		}
//	}
//	return h-1;
//
//}

bool findCorridorCenter(CPP::Image* image, unsigned int& left, unsigned int& right)
{
	byte* pixel;
	unsigned int w = ((CPPImage*)image->get())->get_width();
	unsigned int h = ((CPPImage*)image->get())->get_height();

	int blackPixelsInColumn=0;
	bool isLeftSideFound = false;
	
	vector<int> colPixelCount(w);
	for (unsigned int i = 0; i < w; ++i)
	{
		blackPixelsInColumn = 0;
		for (unsigned int j = TOP_CORRIDOR; j < BOTTOM_CORRIDOR; ++j)
		{
			pixel = ((CPPImage*)image->get())->get_row(j,i);
			if(*pixel == 0)
			{
				blackPixelsInColumn++;
			}
		}
		colPixelCount[i]=blackPixelsInColumn;
	}

	bool foundStart = false;
	int currCorridorWidth = 0;
	for (unsigned int i = 0; i < w; ++i)
	{
		if (colPixelCount[i] > CORRIDOR_HEIGHT)
		{
			if (foundStart)
			{
				++currCorridorWidth;
			}
			else
			{
				left = i;
				foundStart = true;
			}
		}
		else
		{
			if (currCorridorWidth > CORRIDOR_MIN_WIDTH)
			{
				right = i;
				return true;
			}

			foundStart = false;
			currCorridorWidth = 0;
		}
	}
	if (currCorridorWidth > CORRIDOR_MIN_WIDTH)
	{
		right = 639;
		return true;
	}
  printf("find center: corr width %d\n", currCorridorWidth);
	right = 0;
	left = w-1;
	printf("couldnt find corridor center");
	return false;
	/*for (unsigned int i = 0; i < w; ++i)
	{
		blackPixelsInColumn = 0;
		for (unsigned int j = TOP_CORRIDOR; j < BOTTOM_CORRIDOR; ++j)
		{
			pixel = ((CPPImage*)image->get())->get_row(j,i);
			if(*pixel == 0)
			{
				blackPixelsInColumn++;
			}
		}
		if((blackPixelsInColumn > CORRIDOR_HEIGHT))
		{
			left = i;
			break;
		}
	}

	for (unsigned int i = w-10; i > left+1; --i)
	{
		blackPixelsInColumn = 0;
		for (unsigned int j = TOP_CORRIDOR; j < BOTTOM_CORRIDOR; ++j)
		{
			pixel = ((CPPImage*)image->get())->get_row(j,i);
			if(*pixel == 0)
			{
				blackPixelsInColumn++;
			}
		}
		if((blackPixelsInColumn > CORRIDOR_HEIGHT))
		{
			right = i;
			return;
		}
	}
	right = w-1;*/
}

DirectionEnum calcDirection(int left,int right,double& offsetFromCenter)
{
	DirectionEnum direction;
	// decide witch direction to move
	double coridorCenter = (left+right)/2;
	if ((coridorCenter >= CORRIDOR_CENTER - CORRISOR_CENTER_OFFSET) && 
		(coridorCenter <= CORRIDOR_CENTER + CORRISOR_CENTER_OFFSET))
	{
		direction = STRAIGHT;
    offsetFromCenter = 0;
	}
	else if (coridorCenter > CORRIDOR_CENTER)
	{
		direction = RIGHT;
		offsetFromCenter = coridorCenter - CORRIDOR_CENTER;
	}
	else
	{
		direction = LEFT;
		offsetFromCenter = CORRIDOR_CENTER - coridorCenter;
	}
  cout << " offsetFromCenter = " << offsetFromCenter << endl;
	return direction;
}
void findDirection(CPP::Image* image,CPP::Image* resImage,DirectionEnum& direction,
					double& offsetFromCenter)
{
  offsetFromCenter = 0;
	int left, right;
	byte* pixel;
	unsigned int w = ((CPPImage*)image->get())->get_width();
	unsigned int h = ((CPPImage*)image->get())->get_height();

	bool foundStart = false;
	bool foundCorridor = false;
	int currCorridorWidth = 0;
	for (unsigned int i = 20; i < 70; ++i)
	{
		for (unsigned int j = 0; j < w; ++j)
		{
			pixel = ((CPPImage*)image->get())->get_row(i,j);
			if (foundStart)
			{
				if (*pixel == 0)
				{
					++currCorridorWidth;
				}
				else
				{
					if (currCorridorWidth > 180)
					{
						right = j-4;
						if (right < 0)
						{
							right = 0;
						}
						foundCorridor = true;
						break;
					}
					else
					{
						foundStart = false;
						currCorridorWidth = 0;
					}
				}
			}
			else
			{
				if (*pixel == 0)
				{
					left = j;
					foundStart = true;
				}
			}
		}
		foundStart = false;
		if (foundCorridor)
		{
			break;
		}
	}

	if (foundCorridor)
	{
		// check if this is not a turn to a side corridor
		// ------------------------
		
		double rightHeight = 0,rightNeighHeight = 1;
		bool performRightSum = true,performRightNeighSum = true;
		for (unsigned int i = 20; i < 70; ++i)
		{
			// sum column of right corridor side
			pixel = ((CPPImage*)image->get())->get_row(i,right);
			//if (performRightSum)
			//{
				if (*pixel == 0)
				{
					++rightHeight;
				}
			/*	else
				{
					performRightSum = false;
				}
			}*/

			// sum column of right neighbor 
				int neigh = right-10;
				if (neigh < 0)
				{
					neigh = 0;
				}
			pixel = ((CPPImage*)image->get())->get_row(i,neigh);
			//if (performRightNeighSum)
			//{
				if (*pixel == 0)
				{
					++rightNeighHeight;
				}
			/*	else
				{
					performRightNeighSum = false;
				}
			}*/
		}

		// check that the height are not differ
		//cout << "ratio " << rightHeight/rightNeighHeight << " " << rightNeighHeight/rightHeight << endl;
		//if ((rightHeight/rightNeighHeight > 0.6) ||(rightNeighHeight /rightHeight > 0.6) )
		if (rightHeight/rightNeighHeight > 0.6)
		{
			//cout << "ratio above normal" << endl;
			direction = STRAIGHT;
		}
		else
		{
			direction = calcDirection(left,right,offsetFromCenter);
			
		}
		// paint the corridor and the direction of the drive
		paintResults(resImage,left,
					 right,direction);
	}
	else
	{
		direction = STRAIGHT;
		printf("new algorithm: couldnt find corridor center");
	}
	
}

int Corridor::findNextSerialDoorNum(int prevNum, map<int, MapCell>* doorsSerialToCoord)
{
	int serialNum = -1;
	if (m_direction == FORWARD)
	{
		serialNum = prevNum+1;
		while ((serialNum <= m_lastSerialNum) && 
			   (doorsSerialToCoord->find(serialNum) == doorsSerialToCoord->end() ))
		{
			serialNum++;
		}
	}
	else
	{
		serialNum = prevNum-1;
		while ((serialNum >= 1) && 
			   (doorsSerialToCoord->find(serialNum) == doorsSerialToCoord->end() ))
		{
			serialNum--;
		}
	}
	return serialNum;
}


Corridor::Corridor(Map* mapInstance, map<int, MapCell>* leftDoorsSerialToMapCell,
				   map<int, MapCell>* rightDoorsSerialToMapCell,int lastSerialNum) : MapPart(mapInstance)
{
	//m_leftDoorFrameCounter = FRAMES_BETWEEN_DOORS;
	//m_rightDoorFrameCounter = FRAMES_BETWEEN_DOORS;

	m_destInside = false;
	m_sourceInside = false;

	m_destDoor = -1;
	m_sourceDoor = -1; 

	m_lastSerialNum = lastSerialNum;
	m_leftDoorsSerialToMapItem = leftDoorsSerialToMapCell;
	m_rightDoorsSerialToMapItem = rightDoorsSerialToMapCell;


	m_nextDoorNumLeft = -1;
	m_nextDoorNumRight = -1;

	m_prevLeftDoor = 639;
	m_prevRightDoor = 0;
	m_leftDoorCount = 0;
	m_rightDoorCount = 0;
	m_isLeftFound = false;
	m_isRightFound = false;
}

void paintFoundDoorNum(Image* colorImage,int foundDoorNum)
{
	IplImage *img = ((CPPImage*)colorImage->get())->get();

	// paint the number of found door
	if (foundDoorNum != -1)
	{
		char strDoorNum[100];
		itoa(foundDoorNum,strDoorNum,10);
	
		CvFont font;
		double hScale=3.25;
		double vScale=3.25;
		int    lineWidth=3;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

		// paint the number of the door
		cvPutText(img,strDoorNum,cvPoint(210,240),&font,cvScalar(0, 20, 200, 0));
	}
}
bool Corridor::goToNextDoor(Image* colorImage,map<int, MapCell>* corrDoorsSerialToMapItem,
							map<int, MapCell>* secondCorrDoorsSerialToMapItem,
							int& corrNextDoorNum, int& secondCorrNextDoorNum)
{	
	printf("found door num %d\n", corrNextDoorNum);
	paintFoundDoorNum(colorImage,(*corrDoorsSerialToMapItem)[corrNextDoorNum].m_doorNum);

	// update the map with the new position.
	Door doorPosition = (*corrDoorsSerialToMapItem)[corrNextDoorNum].m_doorCoord;
	m_mapManager->foundDoor(doorPosition);
	m_mapManager->setYCoord(doorPosition.second);
	
	// check if we got to destination
	if (m_destDoor == corrNextDoorNum && m_destInside)
	{
		return true;
	}

	// advance next door in the second corridor (in case a door was skipped).
	secondCorrNextDoorNum = findNextSerialDoorNum(corrNextDoorNum, secondCorrDoorsSerialToMapItem);

	// advance to next door in the corridor.
	corrNextDoorNum = findNextSerialDoorNum(corrNextDoorNum, corrDoorsSerialToMapItem);

	//dest door was not found yet...
	return false;
}

void paintArrived(CPP::Image* resImage)
{
	IplImage *img = ((CPPImage*)resImage->get())->get();
	string str("Arrived");
	
	CvFont font;
	double hScale=3.25;
	double vScale=3.25;
	int    lineWidth=3;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

	// paint the number of the door
	cvPutText(img,str.c_str(),cvPoint(150,100),&font,cvScalar(0, 20, 200, 0));
}
NavigateResultEnum Corridor::navigate(CPP::Image* colorImage,CPP::Image* image,CPP::Image* resImage,NavInfo& navInfo)
{

	//printf("navigate in corridor\n");

	unsigned int leftCorridor, rightCorridor;
	bool isCorrFound = findCorridorCenter(image, leftCorridor, rightCorridor);
	if (!isCorrFound) 
	{
		play_wav_clip("horn");
		return FOUND_OBSTACLE;
	}

	//printf("corridor center: %d \n", (rightCorridor+leftCorridor)/2);
	unsigned int corridorCenter = (rightCorridor+leftCorridor)/2;
	if ((abs((double)(corridorCenter-320)) < 50) && (!m_findDoors))
	{
		printf("start find doors\n");
		m_findDoors = true;
	}

	// find the floor
	//unsigned int floorLine = findFloor(image);

	// find the doors
	Door leftDoor,rightDoor;

	findDoors(colorImage, leftCorridor, rightCorridor,leftDoor,rightDoor);

	//if (m_direction == BACKWARD)
	//{
	//	Door temp = leftDoor;
	//	leftDoor = rightDoor;
	//	rightDoor = temp;
	//}

	// count doors
	bool isFoundLeft = false;
	bool isFoundRight = false;
	if (leftDoor.second != ILLEGAL_COORD)
	{
		paintDoor(leftDoor,resImage);
		if ((leftDoor.second < m_prevLeftDoor) || (leftDoor.second - m_prevLeftDoor < 40))
		{
			m_leftDoorCount++;
			m_prevLeftDoor = leftDoor.second;

			if(m_prevLeftDoor < 80 && m_leftDoorCount > 6 && !m_isLeftFound)
			{
				play_wav_clip("Left");
				//printf("$$$$$$$$$$$$$$$$ left door is found $$$$$$$$$$$$$$$$$$$$$$$$$$\n");
				m_isLeftFound = true;
				if (m_direction == BACKWARD)
				{
					isFoundLeft = this->goToNextDoor(resImage,m_rightDoorsSerialToMapItem, m_leftDoorsSerialToMapItem, m_nextDoorNumRight, m_nextDoorNumLeft);
				}
				else
				{
					isFoundLeft = this->goToNextDoor(resImage,m_leftDoorsSerialToMapItem, m_rightDoorsSerialToMapItem, m_nextDoorNumLeft, m_nextDoorNumRight);
				}
			}
		}
		else
		{
			//printf("&&&&&&&&& init left door &&&&&&&&&&\n");
			m_isLeftFound = false;
			m_leftDoorCount = 1;
			m_prevLeftDoor = leftDoor.second;
		}
	}
	
	if (rightDoor.second != ILLEGAL_COORD)
	{
		paintDoor(rightDoor,resImage);
		cout << "right door pos " << rightDoor.second << endl;
		if ((rightDoor.second > m_prevRightDoor) || (m_prevRightDoor - rightDoor.second < 40))
		{
			m_rightDoorCount++;
			m_prevRightDoor = rightDoor.second;

			if(m_prevRightDoor > 560 && m_rightDoorCount > 6 && !m_isRightFound) 
			{
				play_wav_clip("okay-7");
				//printf("$$$$$$$$$$$$$$$$ right door is found $$$$$$$$$$$$$$$$$$$$$$$$$$4\n");
				m_isRightFound = true;
				if(m_direction == BACKWARD)
				{
					isFoundRight = this->goToNextDoor(resImage,m_leftDoorsSerialToMapItem, m_rightDoorsSerialToMapItem, m_nextDoorNumLeft, m_nextDoorNumRight);
				}
				else
				{
					isFoundRight = this->goToNextDoor(resImage,m_rightDoorsSerialToMapItem, m_leftDoorsSerialToMapItem, m_nextDoorNumRight, m_nextDoorNumLeft);
				}
			}
		}
		else
		{
			//printf("&&&&&&&&& init right door &&&&&&&&&&\n");
			m_isRightFound = false;
			m_rightDoorCount = 1;
			m_prevRightDoor = rightDoor.second;
		}
	}

  if (isFoundRight || isFoundLeft)
  {
    paintArrived(resImage);
  }
	// found the destination door
	if (isFoundRight)
	{
			return FOUND_DEST_RIGHT;
	}
	else if (isFoundLeft)
	{
			return FOUND_DEST_LEFT;
	}
	

	// check if we exit the corridor to the hall
	NavigateResultEnum res = CONTINUE_CORR;
  printf("---------------- corr width = %d -----------\n",rightCorridor - leftCorridor);
	if (rightCorridor - leftCorridor > CORRIDOR_WIDTH_IN_HALL)
	{
		res = EXIT_PART;
	}


	if (m_findDoors)
	{
		findDirection(image,resImage,navInfo.direction,navInfo.factor);
	}
	else
	{
		navInfo.direction = calcDirection(leftCorridor,rightCorridor,navInfo.factor);
		cout << "navInfo.direction = " << navInfo.direction << endl;
	}
	
	navInfo.factor /= 300;

	return res;
}