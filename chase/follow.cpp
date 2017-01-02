#include "../libsensor/sensor.h"
#include <iomanip>
#include <cmdline.h>
#include <files.h>
#include <smosvm.h>
#include <feature_manager.h>
#include <profiler.h>
#include <statistics.h>
#include <threads.h>
#include <conio.h>
#include "moments.h"
#include "../robctrl/robintr.h"
#include "clseval.h"
#include "KeyboardThread.h"
#include "Sounds.h"
#include <time.h>
#include "pinpointing.h"
#include "CameraSampler.h"
#include "FPS_timer.h"
#include "FallDetector.h"

#define SEARCH_TARGET_COUNTER		35
#define LOSING_TARGET_COUNTER		(SLOW_COUNTER+10)
#define SLOW_COUNTER				10

#define FRAME_WIDTH					640
#define FRAME_HEIGHT				480

#define TURN_SPEED					416
#define MIN_SPEED					256
#define MAX_SPEED					512

//we are sorry. this magic number are define from previous code.
//have a nice white night at debuging it.
#define MAGIC_COEFFICIENT			0.7
#define MAGIC_SPEED					300

#define PERSON_FOUND_COUNTER		3
#define SPEED_DECREASE_COEFFICIENT  5
#define SPEED_COEFFICIENT			0.6

#define ROTATE_SLEEP_TIME			800 //determinate the angel of the rotation. 45 degree.

using namespace CPP; 
using namespace ML;


typedef enum
{
	STATE_FOLLOW,
	STATE_SEARCH,
	STATE_INIT
} ROBOT_STATE;

typedef enum
{
	ILLEGEAL_STATE,
	ILLEGEAL_SUBSTATE,
	TARGET_NOT_FOUND
} ROBOT_ERROR;

typedef enum
{
	SUB_STATE_SEARCH_ENTRANCE,
	SUB_STATE_SEARCH_CONTINUSE,
	SUB_STATE_FOLLOW_TARGET,
	SUB_STATE_FOLLOW_LOSING
} ROBOT_SUB_STATE;

typedef enum
{
	LEFT_TURN = 1,
	STAY = 0,
	RIGHT_TURN = -1
} ROBOT_TURN;

typedef enum
{
	NO_DEBUG = -1,
	DEBUG0 = 0,
	DEBUG1 = 1,
	DEBUG2 = 2
} ROBOT_DEBUG;






ROBOT_DEBUG DEBUG = DEBUG0;
void calculate_features(Image image, sample& x);

//opens a log file for the following
std::ofstream flog("follow.log");

//a position of the target to follow
struct TargetPosition
{
	Vec2D position;
	TargetPosition(const Vec2D& cg = Vec2D()) : position(cg) {}
};

typedef std::list<TargetPosition> tgt_seq;

typedef std::list<Image> image_seq;

class FollowParameters
{
public:
	ROBOT_STATE state;
	ROBOT_SUB_STATE subState;
	ROBOT_TURN searchDirection;
	tgt_seq targets;
	double targetsMedian;
	short speedLeft;
	short speedRight;
	int counterPersonFound;
	int counterPersonNotFound;
};

RobotInterface* robot;
FollowParameters followParameters;

void reportError(ROBOT_ERROR error);
void changeState();
void searchState();
void followState();
void followTarget();
void losingTarget();



//#define Work_With_FallDetector
void chase()
{
  play_wav_clip("chase menu");
  std::cout << "Chasing...\n";
  robot=RobotInterface::create(4,38400);

  // TODO - remove cls?
  SMOSVMClassifier cls;
  if (!cls.load("classifier.xml")) //failed loading file
  {
    close_wavs_and_play_new_wav("xml fail");
    throw string("Classifier.xml not found");
  } 
  ClsEvaluator te(cls);

  KeyboardThread kbd;
  kbd.start();
  Sleep(100);
  CameraSampler sampler; //thread that takes the actual camera pictures (both color and depth)
  sampler.start(); 
  Sleep(5000);  //handle sampler crashing
#ifdef Work_With_FallDetector
  FallDetector fall_detector(&sampler);   //thread that detects if the person followed falls
  fall_detector.start(); 
#endif // Work_With_FallDetector
  bool paused=false;
  image_seq images;
  Image rgb_frame(new CPPImage(640,480,3));
  FPS_timer timer("Main");
  sampler.wait_for_finished_initializing();//barrier for camera thread
  unsigned long long fall_secs = time(NULL);
  while (kbd.is_active()) //run until told to stop
  { 
    timer.sample(); //will print FPS every few loops

    Image frame;
    sampler.update(rgb_frame,frame);
    frame=convert_to_gray(frame);
    int x,y;
    bool personFound;
    Image target=CFrameManager::extract_man(rgb_frame,frame,&te,x,y,personFound); //target is now the person if personFound==true
	if (personFound)
	{
		followParameters.counterPersonNotFound = 0;
		followParameters.counterPersonFound++;
	}
	else
	{
		followParameters.counterPersonNotFound++;
		followParameters.counterPersonFound = 0;
	}
	if (DEBUG >= DEBUG0)
	{
		cout << "\r                                                                 \r";
		cout << "Found= " << followParameters.counterPersonFound << " Not found= " << followParameters.counterPersonNotFound;
	}
	
    if (kbd.recording())
    {
	  std::cout << "recording after kdb" << std::endl;
      unsigned tc=GetTickCount();
      unsigned long long secs = time(NULL);
      if (personFound && NULL != target)
      {
        close_wavs_and_play_new_wav("taking picture");
		//need to create the folder befor!!
		string path = "C:\\temp\\pics\\";
        std::ostringstream os;	
        os << path << secs << "_" << tc << "_person" << ".png";
		//cout << os.str()<<std::endl;
        target->save(os.str());

        {
          std::ostringstream os;
          os << path << secs << "_" << tc << "_frame" << ".png";
		  //cout << os.str()<<std::endl;
          frame->save(os.str());
        }
        {
          std::ostringstream os;
          os << path << secs << "_" << tc << "_rgb_frame" << ".png";
		  //cout << os.str()<<std::endl;
          rgb_frame->save(os.str());
        }
      }
    }

    if (kbd.init())
    { 
      cout << "===============================================================" << endl;
      cout << "Starting initialization" << endl;
      cout << "===============================================================" << endl;
      //init tracked person histogram.
      bool result = CFrameManager::initPersonHist(rgb_frame,frame, &te,x,y,target);
      cout << "*********** res " << result  << "x,y = " <<x <<"," <<y<< endl;
      if (result)
      {
        personFound = true;
        play_wav_clip("initialization successful",400);
        kbd.stopInit();
        cout << "init success *************************************************" << endl;
	     followParameters.state = STATE_INIT;
		 followParameters.searchDirection = STAY;
		 followParameters.counterPersonFound = 0;
		 followParameters.counterPersonNotFound = 0;
      }
      else
      {
        cout << "failed perform init *************************************************" << endl;
        play_wav_clip("initialization failed");
      }
    }
    if (kbd.pinpointing())
    {
      cout << "===============================================================" << endl;
      cout << "Starting Pinpointing" << endl;
      cout << "===============================================================" << endl;

      current_position_comparing_optimal(rgb_frame,frame);

      kbd.stop_pinpointing();

    }

    if (kbd.paused())
    {
      if (!paused) 
      {
        std::cout << "Paused\n";
        robot->stop();
      }
#ifdef Work_With_FallDetector
      fall_detector.pause();
#endif // Work_With_FallDetector
      paused=true;
      Sleep(100);
      continue;
    }

    paused=false;

#ifdef Work_With_FallDetector
    fall_detector.resume();
#endif Work_With_FallDetector

	if (personFound  && target != NULL)//(cls.classify(s)))
	{
		Vec2D cg = calculate_centroid(target);
		cg.x += x;
		cg.y += y;
		followParameters.targets.push_back(TargetPosition(cg));
		images.push_back(target);
	}
	else
	{
		followParameters.targets.clear();
		images.clear();
	}

	changeState();
	//fall detection code calling
#ifdef Work_With_FallDetector
	fall_detector.provide_person(target, personFound, x, y, followParameters.speedLeft, followParameters.speedRight);
#endif // Work_With_FallDetector
	if (SUB_STATE_SEARCH_CONTINUSE == followParameters.subState &&
		STATE_SEARCH == followParameters.state &&
		followParameters.counterPersonNotFound < SEARCH_TARGET_COUNTER
		)
		// self search
	{
		//DO NOTHING
	}
	else
	{
		if (DEBUG >= DEBUG2)
		{
			flog << "Drive: " << followParameters.speedLeft << " " << followParameters.speedRight << std::endl;
		}
		short speedLeft  = SPEED_COEFFICIENT * followParameters.speedLeft;
		short speedRight = SPEED_COEFFICIENT * followParameters.speedRight;
		robot->drive(speedLeft, speedRight);
		if (STATE_SEARCH == followParameters.state)
		{
			Sleep(ROTATE_SLEEP_TIME);
			robot->stop();
			followParameters.counterPersonNotFound = 0;
		}
	}
	
	
  }

  close_wavs_and_play_new_wav("goodbye");
  robot->stop();
#ifdef Work_With_FallDetector
  fall_detector.terminate();
#endif // Work_With_FallDetector
  sampler.terminate(); 
  Sleep(1000);
  delete robot;
}

void reportError(ROBOT_ERROR error)
{
	switch (error)
	{
		case ILLEGEAL_STATE:
			cout << "ERROR ILLEAGAL STATE -  I WANT TO DIE" << endl;
			flog << "ERROR ILLEAGAL STATE -  I WANT TO DIE" << endl;
			break;
		case ILLEGEAL_SUBSTATE:
			cout << "ERROR ILLEAGAL SUBSTATE -  I WANT TO DIE" << endl;
			flog << "ERROR ILLEAGAL SUBSTATE -  I WANT TO DIE" << endl;
			break;
		case TARGET_NOT_FOUND:
			cout << "ERROR TARGET NOT FOUND AT SUBSTATE FOLLOW TARGET- I WANT TO DIE" << endl;
			flog << "ERROR TARGET NOT FOUND AT SUBSTATE FOLLOW TARGET- I WANT TO DIE" << endl;
			break;
		default:
			cout << "ERROR IN WRONG TATE ====" << endl;
			flog << "ERROR IN WRONG TATE ====" << endl;
			break;
	}
	system("pause");
	exit(85);
}

void changeState()
{
	switch (followParameters.state)
	{
		case STATE_INIT:
			followParameters.state = STATE_SEARCH;
			followParameters.subState = SUB_STATE_SEARCH_ENTRANCE;
			if (DEBUG >= DEBUG0)
			{
				cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
			}
			if (DEBUG >= DEBUG1)
			{
				flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
			}
			break;
		case STATE_SEARCH:
			switch (followParameters.subState)
			{
				case SUB_STATE_SEARCH_ENTRANCE:
					followParameters.subState = SUB_STATE_SEARCH_CONTINUSE;
					if (DEBUG >= DEBUG0)
					{
						cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_CONTINUSE" << endl;
					}
					if (DEBUG >= DEBUG1)
					{
						flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_CONTINUSE" << endl;
					}
					break;
				case SUB_STATE_SEARCH_CONTINUSE:
					if (followParameters.counterPersonFound >= PERSON_FOUND_COUNTER)
					{
						play_wav_clip("found you");
						followParameters.state = STATE_FOLLOW;
						followParameters.subState = SUB_STATE_FOLLOW_TARGET;
						followParameters.counterPersonFound = 0;
						followParameters.counterPersonNotFound = 0;
						if (DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
						if (DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
					}
					break;
				default:
					reportError(ILLEGEAL_SUBSTATE);
					break;
			}
			break;
		case STATE_FOLLOW:
			switch (followParameters.subState)
			{
				case SUB_STATE_FOLLOW_TARGET:
					if (followParameters.counterPersonNotFound > 0)
					{
						followParameters.subState = SUB_STATE_FOLLOW_LOSING;
						followParameters.counterPersonFound = 0;
						if (DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_LOSING" << endl;
						}
						if (DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_LOSING" << endl;
						}
					}
					break;
				case SUB_STATE_FOLLOW_LOSING:
					if (followParameters.counterPersonFound > 0)
					{
						followParameters.subState = SUB_STATE_FOLLOW_TARGET;
						followParameters.counterPersonNotFound = 0;
						if (DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
						if (DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
					}
					else if (followParameters.counterPersonNotFound >= LOSING_TARGET_COUNTER)
					{
						play_wav_clip("target lost");
						followParameters.state = STATE_SEARCH;
						followParameters.subState = SUB_STATE_SEARCH_ENTRANCE;
						followParameters.counterPersonFound = 0;
						followParameters.counterPersonNotFound = 0;
						if (DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
						}
						if (DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
						}
					}
					break;
				default:
					reportError(ILLEGEAL_SUBSTATE);
					break;
			}
			break;
		default:
			reportError(ILLEGEAL_STATE);
			break;
	}
	switch (followParameters.state)
	{
		case STATE_FOLLOW:
			followState();
			break;
		case STATE_SEARCH:
			searchState();
			break;
		default:
			reportError(ILLEGEAL_STATE);
			break;
	}
}

void searchState()
{
	if (DEBUG >= DEBUG2)
	{
		flog << followParameters.subState << endl;
	}
	if (SUB_STATE_SEARCH_ENTRANCE == followParameters.subState)
	{
		followParameters.counterPersonNotFound = 1;
		//store the postions of all the targets in the current frame
		std::vector<double> targetPositions;
		//itatrate all the targets and store their postions.
		for (tgt_seq::iterator it = followParameters.targets.begin(); it != followParameters.targets.end(); ++it)
		{
			const Vec2D& position = it->position;
			if (DEBUG >= DEBUG2)
			{
				flog << "fp=" << int(position.x) << ' ';
			}
			targetPositions.push_back(position.x);
			flog << endl;
		}
		//sorting the targets by postion
		std::sort(targetPositions.begin(), targetPositions.end());


		if (!targetPositions.empty())
		{
			followParameters.targetsMedian = targetPositions[targetPositions.size() / 2];
		}

		if (followParameters.targetsMedian <= (FRAME_WIDTH/2.0))
		{
			followParameters.searchDirection = LEFT_TURN;
		}
		else
		{
			followParameters.searchDirection = RIGHT_TURN;
		}

	}
	else if (followParameters.counterPersonNotFound >= SEARCH_TARGET_COUNTER)
	{
		int turn = followParameters.searchDirection;
		followParameters.speedLeft  = (-TURN_SPEED)	*	turn;
		followParameters.speedRight =   TURN_SPEED	*	turn;
		if (DEBUG >= DEBUG2)
		{
			flog << "speedLeft = " << followParameters.speedLeft << ", speedRight = " << followParameters.speedRight << endl;
		}
	}
	else
	{
		followParameters.speedLeft	= 0;
		followParameters.speedRight = 0;
	}

	if (DEBUG >= DEBUG1)
	{
		flog << "targetMedian = "			<< followParameters.targetsMedian			<< endl;
		flog << "searchDirection = "		<< followParameters.searchDirection			<< endl;
		flog << "counterPersonNotFound = "	<< followParameters.counterPersonNotFound	<< endl;
		flog << "counterPersonFound = "		<< followParameters.counterPersonFound		<< endl;
	}
	return;
}

void followState()
{
	//store the postions of all the targets in the current frame
	std::vector<double> targetPositions;
	//itatrate all the targets and store their postions.
	for (tgt_seq::iterator it = followParameters.targets.begin(); it != followParameters.targets.end(); ++it)
	{
		const Vec2D& position = it->position;
		if (DEBUG >= DEBUG2)
		{
			flog << "fp=" << int(position.x) << ' ';
		}
		targetPositions.push_back(position.x);

	}
	if (!targetPositions.empty())
	{
		if (DEBUG >= DEBUG2)
		{
			flog << endl;
		}
		//sorting the targets by postion
		std::sort(targetPositions.begin(), targetPositions.end());
		followParameters.targetsMedian = targetPositions[targetPositions.size() / 2];
	}
	else if (followParameters.subState == SUB_STATE_FOLLOW_LOSING)
	{
		// DO NOTHING - dont change median, save it for the case target will be lost compleitly in the future 
		// so we know to start search n the right direction (and for velocity calculations)
	}
	else
	{
		reportError(TARGET_NOT_FOUND);
	}
	int speedy = MAGIC_SPEED;
	int speedx = int((followParameters.targetsMedian - (FRAME_WIDTH / 2.0)) * MAGIC_COEFFICIENT);
	followParameters.speedLeft = speedy + speedx;
	followParameters.speedRight = speedy - speedx;
	//in case left or right speed exceeds legal speed - set limit speed.
	if (followParameters.speedLeft<MIN_SPEED)	followParameters.speedLeft	= MIN_SPEED;
	if (followParameters.speedRight<MIN_SPEED)	followParameters.speedRight = MIN_SPEED;
	if (followParameters.speedLeft>MAX_SPEED)	followParameters.speedLeft	= MAX_SPEED;
	if (followParameters.speedRight>MAX_SPEED)	followParameters.speedRight = MAX_SPEED;
	
	switch (followParameters.subState)
	{
		case SUB_STATE_FOLLOW_TARGET:
			followTarget();
			break;
		case SUB_STATE_FOLLOW_LOSING:
			losingTarget();
			break;
		default:
			reportError(ILLEGEAL_SUBSTATE);
			break;
	}
}

void followTarget()
{
	return;
}

void losingTarget()
{
	//TODO need to think about negative speed
	int speedDecrease = (followParameters.counterPersonNotFound - SLOW_COUNTER) * SPEED_DECREASE_COEFFICIENT;
	if (speedDecrease >= abs(followParameters.speedLeft) || speedDecrease >= abs(followParameters.speedRight))
	{
		followParameters.speedLeft = 0;
		followParameters.speedRight = 0;
	}
	else
	{
		followParameters.speedLeft = (followParameters.speedLeft >= 0) ? (followParameters.speedLeft - speedDecrease) : (followParameters.speedLeft + speedDecrease);
		followParameters.speedRight = (followParameters.speedRight >= 0) ? (followParameters.speedRight - speedDecrease) : (followParameters.speedRight + speedDecrease);
	}
}