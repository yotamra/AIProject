// RobotTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Navigator.h"

#include <threads.h>
#include "Sensor.h"
#include "image.h"
using namespace CPP;
#include <iostream>


#define WINDOW_HEIGHT 500
#define WINDOW_WIDTH 1200
#define DISPLAY_EVRY 10

void paintMap(Map* mapManager,Image& calcedImage,Image& depthImage)
{
	MapCell* currMapRow;
	list<Coord>* track = mapManager->m_track;
	Matrix<MapCell>* mapMat = mapManager->m_map;
	Coord* currPosition = mapManager->m_currPosition;

	 /* create an image to represent the map */
	IplImage *mapImg = cvCreateImage(cvSize(WINDOW_WIDTH, WINDOW_HEIGHT), IPL_DEPTH_8U, 3);

	// get the number of cells in the map
	int mapWidth = mapMat->get_width();
	int mapHeight = mapMat->get_height();

	// calc the size of the cells in the display
	int cellWidth = WINDOW_WIDTH / mapWidth;
	int cellHeight = WINDOW_HEIGHT / mapHeight;
   	
	CvFont font;
	double hScale=0.25;
	double vScale=0.25;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);


	// draw the matrix
	// ---------------

	// draw the vertical line
	for (int i = 0;i < mapWidth+1;++i)
	{
		cvLine(mapImg,                         /* the dest image */
		   cvPoint(i*cellWidth,0),             /* start point */
		   cvPoint(i*cellWidth,WINDOW_HEIGHT),            /* end point */
		   cvScalar(0, 80, 0, 0),      /* the color; green */
		   1, 8, 0);                    /* thickness, line type, shift */
	}

	// draw the horizontal line
	for (int i = 0;i < mapHeight+1;++i)
	{
		cvLine(mapImg,                         /* the dest image */
		   cvPoint(0,i*cellHeight),             /* start point */
		   cvPoint(WINDOW_WIDTH,i*cellHeight),            /* end point */
		   cvScalar(0, 80, 0, 0),      /* the color; green */
		   1, 8, 0);                    /* thickness, line type, shift */
	}

	// draw the track

	int i = 0;
	for (list<Coord>::iterator currPoint = track->begin();
		currPoint != track->end();
		++currPoint)
	{
		int currX = currPoint->first;
		int currY = currPoint->second;
		
		
		cvRectangle(mapImg,                    /* the dest image */
				cvPoint(currY*cellWidth,currX*cellHeight),        /* top left point */
				cvPoint((currY+1)*cellWidth,(currX+1)*cellHeight),       /* bottom right point */
				cvScalar(0, 0, 255, 255), /* the color; blue */
				CV_FILLED, 8, 0);               /* thickness, line type, shift */
	}

	// draw the walls and doors
	for (unsigned int i = 0;i < mapHeight;++i)
	{
		currMapRow = mapMat->get_row(i);
		for (unsigned int j = 0;j < mapWidth;++j)
		{
			// check if this is a wall cell
			if (currMapRow->m_cellType == WALL)
			{
				 cvRectangle(mapImg,                    /* the dest image */
				cvPoint(j*cellWidth,i*cellHeight),        /* top left point */
				cvPoint((j+1)*cellWidth,(i+1)*cellHeight),       /* bottom right point */
				cvScalar(0, 255, 0, 0), /* the color; blue */
				CV_FILLED, 8, 0);               /* thickness, line type, shift */
			}
			// paint the door
			else if (currMapRow->m_cellType == DOOR)
			{
				char strDoorNum[100];
				itoa(currMapRow->m_doorNum,strDoorNum,10);
				// if we found the door paint it in diffrent color
				if (currMapRow->m_isFound)
				{
					cvRectangle(mapImg,                    /* the dest image */
								cvPoint(j*cellWidth,i*cellHeight),        /* top left point */
								cvPoint((j+1)*cellWidth,(i+1)*cellHeight),       /* bottom right point */
								cvScalar(0, 255, 255, 255), /* the color; blue */
								CV_FILLED, 8, 0);               /* thickness, line type, shift */
				}
				else
				{
					cvRectangle(mapImg,                    /* the dest image */
								cvPoint(j*cellWidth,i*cellHeight),        /* top left point */
								cvPoint((j+1)*cellWidth,(i+1)*cellHeight),       /* bottom right point */
								cvScalar(255, 255, 255, 255), /* the color; blue */
								CV_FILLED, 8, 0);               /* thickness, line type, shift */
				}

				// paint the number of the door
				cvPutText(mapImg,strDoorNum,cvPoint(j*cellWidth,(i+1)*cellHeight),&font,cvScalar(0, 20, 30, 0));
			}
			++currMapRow;
		}
	}

	// draw the current position
	//if (isFound)
	//{
	//	cvRectangle(mapImg,                    /* the dest image */
	//				cvPoint(currPosition->second*cellWidth,currPosition->first*cellHeight),        /* top left point */
	//				cvPoint((currPosition->second+1)*cellWidth,(currPosition->first+1)*cellHeight),       /* bottom right point */
	//				cvScalar(50,0, 150, 0), /* the color; blue */
	//				CV_FILLED, 8, 0);               /* thickness, line type, shift */
	//}
	//else
	//{
		cvRectangle(mapImg,                    /* the dest image */
					cvPoint(currPosition->second*cellWidth,currPosition->first*cellHeight),        /* top left point */
					cvPoint((currPosition->second+1)*cellWidth,(currPosition->first+1)*cellHeight),       /* bottom right point */
					cvScalar(50, 0, 100, 50), /* the color; blue */
					CV_FILLED, 8, 0);               /* thickness, line type, shift */
	//}
	
	/* display the map */
	cvNamedWindow("map", CV_WINDOW_AUTOSIZE);
	cvShowImage("map", mapImg);

	// display the result image
	cvNamedWindow("result", CV_WINDOW_AUTOSIZE);
	cvShowImage("result", ((CPPImage*)calcedImage.get())->get());

	/* display the depth image */
	/*cvNamedWindow("depth", CV_WINDOW_AUTOSIZE);
	cvShowImage("depth", ((CPPImage*)depthImage.get())->get());*/

	cvWaitKey(100);

	//cvDestroyWindow("img");
	cvReleaseImage(&mapImg);
	//cout << "continue to next door" << endl;
}
//
//void play_wave(const char* filename, bool loop)
//{
//  DWORD flags=SND_ASYNC;
//  if (loop) flags|=SND_LOOP;
//  PlaySoundA(filename,0,flags);
//}
//
//void play_wav_clip(const char* name)
//{
//  static unsigned last_play=0;
//  unsigned cur=GetTickCount();
//  if ((cur-last_play)>1500)
//  {
//    std::ostringstream os;
//    os << "C:\\shared\\sounds\\voices\\" << name << ".wav";
//    play_wave(os.str().c_str(),false);
//  }
//  last_play=cur;
//}


void close_all_waves()
{
  PlaySound(0,0,0);
}

class KeyboardThread : public Thread
{
  bool m_Paused;
  bool m_Exit;
  bool m_Record;
  bool m_inInit;
public:
  int m_srcRoom;
  int m_dstRoom;

  KeyboardThread() 
    : m_Paused(true),
      m_Record(false),
      m_Exit(false),
	  m_inInit(true),
	  m_srcRoom(0),
	  m_dstRoom(0)
  {}

  bool finishedInit()const { return m_inInit;}
  bool paused() const { return m_Paused; }
  bool recording() const { return m_Record; }
  bool exit() const{ return m_Exit;}
  virtual void run()
  {
    char ch=0;
    bool done=false;
	bool srcRoomInit = true;
	double factor = 100;
	int prevDigit = -1;
    while (!done)
    {
      if (GetKeyState(VK_ESCAPE) < 0) done=true;
      if (GetKeyState(VK_BACK) < 0) done=true;
      
		if (m_inInit)
		{
			// get room number
			int digit = -1;
			if (GetKeyState('0')<0) 
			{
				digit = 0;
			}
			else if (GetKeyState('1')<0) 
			{
				digit = 1;
			}
			else if (GetKeyState('2')<0) 
			{
				digit = 2;
			}
			else if (GetKeyState('3')<0) 
			{
				digit = 3;
			}
			else if (GetKeyState('4')<0) 
			{
				digit = 4;
			}
			else if (GetKeyState('5')<0) 
			{
				digit = 5;
			}
			else if (GetKeyState('6')<0) 
			{
				digit = 6;
			}
			else if (GetKeyState('7')<0) 
			{
				digit = 7;
			}
			else if (GetKeyState('8')<0) 
			{
				digit = 8;
			}
			else if (GetKeyState('9')<0) 
			{
				digit = 9;
			}

			if (digit >= 0)
			{
				cout << "digit = " << digit << endl;
				if (digit == prevDigit)
				{
					prevDigit = -1;
				}
				else
				{
					prevDigit = digit;
					if (srcRoomInit)
					{
						char buf[5];
						itoa(digit,buf,10);
						play_wav_clip(buf);

						m_srcRoom = m_srcRoom + digit*factor;

						factor = factor / 10;
						if (factor < 1)
						{
              play_wav_clip("i-know");
							cout << "src room = " << m_srcRoom << endl;
							srcRoomInit = false;
							factor = 100;
						}
					}
					else
					{
						char buf[5];
						itoa(digit,buf,10);
						play_wav_clip(buf);

						m_dstRoom = m_dstRoom + digit*factor;

						factor = factor / 10;
						if (factor < 1)
						{
							cout << "dst room = " << m_dstRoom << endl;

							Sleep(200);
              
              play_wav_clip("im-so-ready");

							m_inInit = false;
						}
					}
				}
			}
		}
		else
		{
		  if (GetKeyState('1')<0) // stop moving
		  {
			m_Paused=true;
			cout << "Paused" << endl;
			play_wav_clip("well-talk-about-it-soon");
		  }
		  if (GetKeyState('2')<0) // start navigating
		  { 
			cout << "Start Navigating!!" << endl;
			if (m_Paused) play_wav_clip("woohoo");
			
			m_Paused=false;
			m_Record=false;
		  }
		  if (GetKeyState('3')<0) // start recording
		  {
			m_Record=true;
			play_wav_clip("you-got-it-1");
		  }
		   if (GetKeyState('4')<0) // start recording
		  {
			m_Exit=true;
			play_wav_clip("shut-up-1");
		  }
		}
      Sleep(100);
    }
  }
};


int _tmain(int argc, _TCHAR* argv[])
{
  play_wav_clip("go-go");

	Sleep(100);

	KeyboardThread kbd;
	kbd.start();
	Sleep(100);

	// load the curr depth image
	// load the curr depth image
	Navigator navigator;
	navigator.init("C:\\Shared\\Navigation\\Maps\\Floor4_BEST.txt");
	
	while (kbd.finishedInit())
		Sleep(100);

	// TODO : init robot
	cout << "src = " << kbd.m_srcRoom << " dst = " << kbd.m_dstRoom << endl;


	navigator.buildTrack(kbd.m_srcRoom,kbd.m_dstRoom);//(455, 419);//417,436
	Map* mapManager = navigator.getMap();
	int count = 0;
	// init the camera
	Sensor* cam=Sensor::create();
   Image rgb_frame(new CPPImage(640,480,3));
  
   while (true)
  {
    Image frame(new CPPImage(640,480,3));
    cam->update(rgb_frame,frame);

	 // save result
	  unsigned tc=GetTickCount();
    std::ostringstream os;
    os << "C:\\Shared\\Results\\rgb_frame_" << tc << ".png";
    ((CPPImage*)rgb_frame.get())->save(os.str());
    
    std::ostringstream os1;
    os1 << "C:\\Shared\\Results\\frame_" << tc << ".png";
    ((CPPImage*)frame.get())->save(os1.str());

	  // run algo and save images.
    if (!kbd.paused())
	  {
      // run algo and save images.
		  Image calcedImage;
		  bool isFound = navigator.navigate(&rgb_frame,&frame,&calcedImage);
		  
      /*++count;
      if (count % 4 == 0)
      {
        count = 0;*/
		    paintMap(mapManager,calcedImage,frame);
      //}
        if (isFound)
		    {
          while (!(kbd.exit()))
	        {
            Sleep(400);
	        }
			    return 0;
		    }
    }

	  if (kbd.paused())
	  {
      navigator.stop();
	  }
    if (kbd.exit())
	  {
      navigator.stop();
      return 0;
	  }
  }
	return 0;
}

