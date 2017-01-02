#include "Navigator.h"
#include "nav_utils.h"
#include "hsv.h"

using namespace CPP;

//Image convertToH(Image& colorImage)
//{
//	int w = ((CPPImage*)colorImage.get())->get_width();
//	int h = ((CPPImage*)colorImage.get())->get_height();
//	
//	Image newImage(new CPPImage(w,h,1));
//	byte* row;
//	byte* newRow;
//			rgb src;
//	for (unsigned int i = 0; i < h; ++i)
//	{
//		row = colorImage->get_row(i);
//		newRow = ((CPPImage*)newImage.get())->get_row(i);
//		for (unsigned int j = 0; j < w; ++j)
//		{
//
//			src.set(row);
//			*newRow = hsl(src).l*255;
//
//			row += 3;
//			newRow++;
//		}
//	}
//	return newImage;
//}

void Navigator::init(string inputMapPath)
{
	m_map = new Map();
	m_movmentManager = new MovmentManager(m_map);

	m_map->init(inputMapPath);
}
void Navigator::buildTrack(unsigned int sourceRoom, unsigned int destRoom)
{
	m_currMapPartIndex = 0;
	m_partsInPath = m_map->calcTrack(sourceRoom,destRoom);
}

int count1 = 1;
bool Navigator::navigate(CPP::Image* colorImage,Image* image,Image* calcedImage)
{
	Image depthImage = convert_to_gray(*image);
	Image hImage = convert_to_gray(*colorImage);
//
//m_movmentManager->moveRobotInCorr(RIGHT,0.05);
//	*calcedImage = hImage;
//  return false;

	if (m_currMapPartIndex >= m_partsInPath.size())
	{
		cout << "error!!!" << endl;
		return true;
	}
	// get the current MapPart
	MapPart* currPart = m_partsInPath[m_currMapPartIndex];

	*calcedImage = *colorImage;

	// navigate
	NavInfo navInfo;
	NavigateResultEnum result = currPart->navigate(&hImage,&depthImage,calcedImage,navInfo);
	
	// advance to next part
	bool isArrived = false;
	if (result == EXIT_PART)
	{
		printf("exit part\n");
		++m_currMapPartIndex;
	}
  else if (result == CONTINUE_CORR)
  {
    printf("~~~~~~~CONTINUE_CORR~~~~~\n");
    if (navInfo.factor < 0.1)
  {
    navInfo.factor = 0.1;
  }
    // move the robot	
		#ifdef ROBOT
		m_movmentManager->moveRobot(navInfo.direction,navInfo.factor);
		#endif
  }
	else if (result == CONTINUE)
	{
    printf("~~~~~~~CONTINUE~~~~~\n");
		// move the robot	
		#ifdef ROBOT
    if (navInfo.factor < 0.05)
  {
    navInfo.factor = 0.05;
  }
			m_movmentManager->moveRobot(navInfo.direction,navInfo.factor);
		#endif
	}
	else if (result == FOUND_OBSTACLE)
	{
  		// stop the robot	
		#ifdef ROBOT
			m_movmentManager->stopRobot();
		#endif

			printf("!!!!FOUND_OBSTACLE!!!!!\n");
	}
	// handle arrived to dest
	else
	{
    DirectionEnum dir = RIGHT;
    if (result == FOUND_DEST_LEFT)
    {
      dir = LEFT;
    }

		// turning
		m_movmentManager->turnRobot(dir);

		play_wav_clip("open-the-door");
		isArrived = true;
    printf("arrived\n");
	}

	return isArrived;
}

void Navigator::stop()
{
  m_movmentManager->stopRobot();
}
Map* Navigator::getMap()
{
	return m_map;
}
// display support
//void Navigator::getMap(Matrix<MapCell>*& map,
//				Coord& destination,
//				Coord& currPosition,
//				list<Coord>*& track)
//{
//	m_map->getMap(map,destination,currPosition,track);
//}
