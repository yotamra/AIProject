#include "Hall.h"
#include <vector>
using namespace std;

Hall::Hall(Map* mapInstance) : MapPart(mapInstance)
{
	m_turnDirection = UNINITIALIZED_DIRECTION;
	m_turnFactor = 0;
}

void Hall::buildPath(unsigned int sourceRoom, unsigned int destRoom)
{
	if (sourceRoom < destRoom)
	{
		m_turnFactor = 2.3;
		m_turnDirection = LEFT_TURN;
	}
	else
	{
		m_turnFactor = 2.5;//2.3
		m_turnDirection = RIGHT_TURN;
	}
}

NavigateResultEnum Hall::navigate(CPP::Image* colorImage,CPP::Image* depthImage,CPP::Image* resImage, NavInfo& navInfo)
{
	unsigned int rightCorr, leftCorr;
	bool isCorridorFound = findCorridorLimits(depthImage, leftCorr, rightCorr);
	int corrWidth = rightCorr-leftCorr;
	
	//turn right
	if(m_turnDirection == RIGHT_TURN)
	{
    cout << "corrWidth = " << corrWidth << " rightCorr = " << rightCorr << endl;
		if (isCorridorFound && (corrWidth>130 && corrWidth<200) && rightCorr>TURN_RIGHT_CORR_MIN && rightCorr<TURN_RIGHT_CORR_MAX)
		{
			printf("********** corridor found ************\n");
			return EXIT_PART;
		}
		else
		{
			navInfo.factor = countBlackPixelsTop(*depthImage);
			navInfo.direction = RIGHT;
			return CONTINUE;
		}
	}
	//TODO:: turn left
	else
	{
		if (isCorridorFound && (corrWidth>130 && corrWidth<200) && leftCorr>TURN_LEFT_CORR_MIN && leftCorr<TURN_LEFT_CORR_MAX)
		{
			printf("********** corridor found ************\n");
			return EXIT_PART;
		}
		else
		{
			navInfo.factor = countBlackPixelsTop(*depthImage);
			navInfo.direction = LEFT;
			return CONTINUE;
		}
	}
	
}

bool Hall::findCorridorLimits(CPP::Image* image, unsigned int& left, unsigned int& right)
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
		if (colPixelCount[i] > CORRIDOR_TO_FIND_HEIGHT)
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
			if (currCorridorWidth > CORRIDOR_TO_FIND_WIDTH)
			{
				right = i;
				return true;
			}

			foundStart = false;
			currCorridorWidth = 0;
		}
	}

	left = 0;
	right = w-1;
	return false;
}


double Hall::countBlackPixelsTop(Image& depthImage)
{
	unsigned int w = ((CPPImage*)depthImage.get())->get_width();
	int h = ((CPPImage*)depthImage.get())->get_height();
	byte* row;

	double numOfBlack=0;
	
	//count black pixels.
	for (unsigned int i = 0; i < TOP_BLACK_PIXEL_STRIP_SIZE; ++i)
	{
		row = ((CPPImage*)depthImage.get())->get_row(i);
		for (unsigned int j = 0; j < w; ++j)
		{
			if(*row ==0)
			{
				numOfBlack++; 
			}
			++row;
		}
	}

	double res = numOfBlack;

	double black = numOfBlack;

	if(black > MAX_BLACK)
	{
		black = MAX_BLACK;
	}
	else if(black < MIN_BLACK)
	{
		black = MIN_BLACK;
	}

	double factor = 1-((black - MIN_BLACK)/(MAX_BLACK-MIN_BLACK));

	printf("factor = %lf   ", factor);
	printf("speed = %lf\n", ((factor/m_turnFactor)*256)+256);

	return (factor/m_turnFactor);
}