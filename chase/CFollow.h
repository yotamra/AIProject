#ifndef CFOLLOW
#define CFOLLOW

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

#define Work_With_FallDetector

// Defines:

#define SEARCH_TARGET_COUNTER				35
#define LOSING_TARGET_COUNTER				(SLOW_COUNTER+10)
#define SLOW_COUNTER						10

#define FRAME_WIDTH							640
#define FRAME_HEIGHT						480

#define TURN_SPEED							416
#define MIN_SPEED							256
#define MAX_SPEED							512

#define MAGIC_COEFFICIENT					0.7
#define MAGIC_SPEED							300

#define PERSON_FOUND_COUNTER				3
#define SPEED_DECREASE_COEFFICIENT 			5
#define SPEED_COEFFICIENT					0.6

#define ROTATE_SLEEP_TIME				    800  //determinate the angel of the rotation. 45 degree.


// Enums:

typedef enum
{
	STATE_FOLLOW,
	STATE_SEARCH,
	STATE_INIT
} ROBOT_STATE;

typedef enum
{
	SUB_STATE_SEARCH_ENTRANCE,
	SUB_STATE_SEARCH_CONTINUSE,
	SUB_STATE_FOLLOW_TARGET,
	SUB_STATE_FOLLOW_LOSING
} ROBOT_SUB_STATE;

typedef enum
{
	ILLEGEAL_STATE,
	ILLEGEAL_SUBSTATE,
	TARGET_NOT_FOUND
} ROBOT_ERROR;

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


// Structs:

//a position of the target to follow
struct TargetPosition
{
	Vec2D position;
	TargetPosition(const Vec2D& cg = Vec2D()) : position(cg) {}
};

// Typedef:
typedef std::list<TargetPosition> tgt_seq;
typedef std::list<Image> image_seq;

struct FollowParameters
{
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


class CFollow
{
// Functions Member:
public:
   CFollow(); 
   ~CFollow();
   void Init();
   void RunningAndFollow();

// Methods:
private:
    void ReportError(ROBOT_ERROR);
    void ChangeState();
    void SearchState();
    void FollowState();
    void FollowTarget();
    void LosingTarget();
    void KeyRecording();
    void KeyInit();
    void KeyPinpointing();
	void KeyPaused();
	void DriveControl();
	bool CheckKeyboardStatus();
	void GeometricCenterCalc(Image target);


// Variables:
private: 
    ROBOT_DEBUG m_DEBUG;
    FollowParameters m_FollowParameters;
    RobotInterface* m_Robot;
    SMOSVMClassifier m_Cls;
    KeyboardThread m_Kbd;
    CameraSampler m_Sampler; // Thread that takes the actual camera pictures (both color and depth)
	Image m_iRgb_Frame;      // Image rgb_frame(new CPPImage(640,480,3));
    Image m_iFrame;
	Image m_iTarget;
    bool m_bPersonFound;
	bool m_bPaused;
    int m_x, m_y;
	ClsEvaluator* m_Te;
	FPS_timer m_Timer;
	int frameNumber;
#ifdef Work_With_FallDetector
	FallDetector* m_pFall_Detector;
#endif // Work_With_FallDetector
    // unsigned long long fall_secs = time(NULL);
};

#endif // CFOLLOW