#include "Aria.h"
#include "CFrameManager.h"
#include "sensor.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <math.h>


bool checkProportion(double firstWidth, double currWidth);
void CreateImage();
//Sensor* sensor = Sensor::create();

/* 
 * Action that drives the robot forward, but stops if obstacles are
 * detected by sonar. 
 */
class ActionGo : public ArAction
{
public:
  // constructor, sets myMaxSpeed and myStopDistance
  ActionGo(double maxSpeed, double stopDistance);
  // destructor. does not need to do anything
  virtual ~ActionGo(void) {};
  // called by the action resolver to obtain this action's requested behavior
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // store the robot pointer, and it's ArSonarDevice object, or deactivate this action if there is no sonar.
  virtual void setRobot(ArRobot *robot);
protected:
  // the sonar device object obtained from the robot by setRobot()
  ArRangeDevice *mySonar;


  /* Our current desired action: fire() modifies this object and returns
      to the action resolver a pointer to this object.
      This object is kept as a class member so that it persists after fire()
      returns (otherwise fire() would have to create a new object each invocation,
      but would never be able to delete that object).
  */
  ArActionDesired myDesired;

  double myMaxSpeed;
  double myStopDistance;
};

/* Action that turns the robot away from obstacles detected by the 
 * sonar. */
class ActionTurn : public ArAction
{
public:
  // constructor, sets the turnThreshold, and turnAmount
  ActionTurn(double turnThreshold, double turnAmount);
  // destructor, its just empty, we don't need to do anything
  virtual ~ActionTurn(void) {};
  // fire, this is what the resolver calls to figure out what this action wants
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // sets the robot pointer, also gets the sonar device, or deactivates this action if there is no sonar.
  virtual void setRobot(ArRobot *robot);
protected:
  // this is to hold the sonar device form the robot
  ArRangeDevice *mySonar;
  // what the action wants to do; used by the action resover after fire()
  ArActionDesired myDesired;
  // distance at which to start turning
  double myTurnThreshold;
  // amount to turn when turning is needed
  double myTurnAmount;
  // remember which turn direction we requested, to help keep turns smooth
  int myTurning; // -1 == left, 1 == right, 0 == none
};

/*
  Note the use of constructor chaining with 
  ArAction(actionName). Also note how it uses setNextArgument, which makes it so that 
  other parts of the program could find out what parameters this action has, and possibly modify them.
*/
ActionGo::ActionGo(double maxSpeed, double stopDistance) :
  ArAction("Go")
{
  mySonar = NULL;
  myMaxSpeed = maxSpeed;
  myStopDistance = stopDistance;
  setNextArgument(ArArg("maximum speed", &myMaxSpeed, "Maximum speed to go."));
  setNextArgument(ArArg("stop distance", &myStopDistance, "Distance at which to stop."));
}

/*
  Override ArAction::setRobot() to get the sonar device from the robot, or deactivate this action if it is missing.
  You must also call ArAction::setRobot() to properly store
  the ArRobot pointer in the ArAction base class.
*/
void ActionGo::setRobot(ArRobot *robot)
{
  ArAction::setRobot(robot);
  mySonar = robot->findRangeDevice("sonar");
  if (robot == NULL)
    {
      ArLog::log(ArLog::Terse, "actionExample: ActionGo: Warning: I found no sonar, deactivating.");
      deactivate();
    }
}

/*
  This fire is the whole point of the action.
  currentDesired is the combined desired action from other actions
  previously processed by the action resolver.  In this case, we're
  not interested in that, we will set our desired 
  forward velocity in the myDesired member, and return it.

  Note that myDesired must be a class member, since this method
  will return a pointer to myDesired to the caller. If we had
  declared the desired action as a local variable in this method,
  the pointer we returned would be invalid after this method
  returned.
*/
ArActionDesired *ActionGo::fire(ArActionDesired currentDesired)
{
  double range;
  double speed;
  CreateImage();
  // reset the actionDesired (must be done), to clear
  // its previous values.
  myDesired.reset();

  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }
  // get the range of the sonar
  range = mySonar->currentReadingPolar(-70, 70) - myRobot->getRobotRadius();
  // if the range is greater than the stop distance, find some speed to go
  if (range > myStopDistance)
  {
    // just an arbitrary speed based on the range
    speed = range * .3;
    // if that speed is greater than our max, cap it
    if (speed > myMaxSpeed)
      speed = myMaxSpeed;
    // now set the velocity
    myDesired.setVel(speed);
  }
  // the range was less than the stop distance, so request stop
  else
  {
    myDesired.setVel(0);
  }
  // return a pointer to the actionDesired to the resolver to make our request
  return &myDesired;
}

/*
  This is the ActionTurn constructor, note the use of constructor chaining 
  with the ArAction. also note how it uses setNextArgument, which makes 
  it so that other things can see what parameters this action has, and 
  set them.  It also initializes the classes variables.
*/
ActionTurn::ActionTurn(double turnThreshold, double turnAmount) :
  ArAction("Turn")
{
  myTurnThreshold = turnThreshold;
  myTurnAmount = turnAmount;
  setNextArgument(ArArg("turn threshold (mm)", &myTurnThreshold, "The number of mm away from obstacle to begin turnning."));
  setNextArgument(ArArg("turn amount (deg)", &myTurnAmount, "The number of degress to turn if turning."));
  myTurning = 0;
}

/*
  Sets the myRobot pointer (all setRobot overloaded functions must do this),
  finds the sonar device from the robot, and if the sonar isn't there, 
  then it deactivates itself.
*/
void ActionTurn::setRobot(ArRobot *robot)
{
  ArAction::setRobot(robot);
  mySonar = robot->findRangeDevice("sonar");
  if (mySonar == NULL)
  {
    ArLog::log(ArLog::Terse, "actionExample: ActionTurn: Warning: I found no sonar, deactivating.");
    deactivate(); 
  }
}

/*
  This is the guts of the Turn action.
*/
ArActionDesired *ActionTurn::fire(ArActionDesired currentDesired)
{
  double leftRange, rightRange;
  // reset the actionDesired (must be done)
  myDesired.reset();
  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }
  // Get the left readings and right readings off of the sonar
  leftRange = (mySonar->currentReadingPolar(0, 100) - 
        myRobot->getRobotRadius());
  rightRange = (mySonar->currentReadingPolar(-100, 0) - 
        myRobot->getRobotRadius());
  // if neither left nor right range is within the turn threshold,
  // reset the turning variable and don't turn
  if (leftRange > myTurnThreshold && rightRange > myTurnThreshold)
  {
    myTurning = 0;
    myDesired.setDeltaHeading(0);
  }
  // if we're already turning some direction, keep turning that direction
  else if (myTurning)
  {
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // if we're not turning already, but need to, and left is closer, turn right
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else if (leftRange < rightRange)
  {
    myTurning = -1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // if we're not turning already, but need to, and right is closer, turn left
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else 
  {
    myTurning = 1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // return a pointer to the actionDesired, so resolver knows what to do
  return &myDesired;
}

ArTime start;

int run(int argc, char** argv)
{
  Aria::init();
  start.setToNow();

  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector con(&argParser, &robot);
  ArSonarDevice sonar;

  // Create instances of the actions defined above, plus ArActionStallRecover, 
  // a predefined action from Aria.
  ActionGo go(500, 350);
  ActionTurn turn(400, 10);
  ArActionStallRecover recover;

    
  // Parse all command-line arguments
  if(!Aria::parseArgs())
  {
    Aria::logOptions();
    return 1;
  }
  
  // Connect to the robot
  if(!con.connectRobot())
  {
    ArLog::log(ArLog::Terse, "actionExample: Could not connect to robot! Exiting.");
    return 2;
  }

  // Add the range device to the robot. You should add all the range 
  // devices and such before you add actions
  robot.addRangeDevice(&sonar);

 
  // Add our actions in order. The second argument is the priority, 
  // with higher priority actions going first, and possibly pre-empting lower
  // priority actions.
  robot.addAction(&recover, 100);
  robot.addAction(&go, 50);
  robot.addAction(&turn, 49);

  // Enable the motors, disable amigobot sounds
  robot.enableMotors();

  // Run the robot processing cycle.
  // 'true' means to return if it loses connection,
  // after which we exit the program.
  robot.run(false);
  
  Aria::exit(0);
}

void createRegions()
{
	int NUM_OF_PICTURES = 87;
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\micha\\" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\micha\\" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);
		int x=0,y=0;
		bool isMan=false;
		SMOSVMClassifier cls;
		if (!cls.load("classifier.xml")) //failed loading file
		{
			throw string("Classifier.xml not found");
		} 
		ClsEvaluator te(cls);
		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}
	NUM_OF_PICTURES = 63;
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\itai\\" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\itai\\" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);
		int x=0,y=0;
		bool isMan=false;
		SMOSVMClassifier cls;
		if (!cls.load("classifier.xml")) //failed loading file
		{
			throw string("Classifier.xml not found");
		} 
		ClsEvaluator te(cls);
		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}
	NUM_OF_PICTURES = 38;
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\tal\\" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\tal\\" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);
		int x=0,y=0;
		bool isMan=false;
		SMOSVMClassifier cls;
		if (!cls.load("classifier.xml")) //failed loading file
		{
			throw string("Classifier.xml not found");
		} 
		ClsEvaluator te(cls);
		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}
	NUM_OF_PICTURES = 259;
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\tal and micha\\" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\tal and micha\\" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);
		int x=0,y=0;
		bool isMan=false;
		SMOSVMClassifier cls;
		if (!cls.load("classifier.xml")) //failed loading file
		{
			throw string("Classifier.xml not found");
		} 
		ClsEvaluator te(cls);
		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}
	NUM_OF_PICTURES = 2778;
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\InDoor\\" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\InDoor\\" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);
		int x=0,y=0;
		bool isMan=false;
		SMOSVMClassifier cls;
		if (!cls.load("classifier.xml")) //failed loading file
		{
			throw string("Classifier.xml not found");
		} 
		ClsEvaluator te(cls);
		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}
}

bool evaluate1(Image image, double threshold)
{
	int w = image->get_width(), h = image->get_height();
	int count = 0;
	int largest_streak = 0;
	int streak = 0;
	for (int y = 0; y < h; ++y)
	{
		const uchar* row = image->get_row(y);
		bool last = false;
		int runs = 0;
		for (int x = 0; x < w; ++x)
		{
			bool cur = (row[x]>128);
			if (cur && !last) ++runs;
			last = cur;
		}
		if (runs == 2)
		{
			++count;
			++streak;
			largest_streak = Max(streak, largest_streak);
		}
		else
		{
			streak = 0;
		}
	}
	return (((double(count) / h) > threshold) | ((double(largest_streak)) / h > threshold));
}

bool evaluate2(Image image, double threshold)
{
#define THRESHOLD_COUNT 0.33
#define THRESHOLD_LARGEST 0.13
	int w = image->get_width(), h = image->get_height();
	int count = 0;
	int largest_streak = 0;
	int streak = 0;
	for (int y = 0; y < h; ++y)
	{
		const uchar* row = image->get_row(y);
		bool last = false;
		int runs = 0;
		int runsWidth = 0;
		int currWidth = 0;
		for (int x = 0; x < w; ++x)
		{
			bool cur = (row[x]>128);
			if (cur)
			{
				currWidth++;
			}
			if (!cur && last && currWidth >= 6 && currWidth < 66)
			{
				++runsWidth;
			}
			if (!cur && last)
			{
				++runs;
			}
			if (!cur)
			{
				currWidth = 0;
			}
			last = cur;
		}
		if (runs < 4 && runsWidth == 2)
		{
			++count;
			++streak;
			largest_streak = Max(streak, largest_streak);
		}
		else
			streak = 0;
	}
	bool isManFound = ((double(count) / h)>threshold) && ((double(largest_streak)) / h > THRESHOLD_LARGEST);
	return isManFound;
}

bool evaluate3(Image image, double threshold)
{
#define THRESHOLD_COUNT 0.38
#define THRESHOLD_LARGEST 0.10
	int w = image->get_width(), h = image->get_height();
	int count = 0;
	int largest_streak = 0;
	int streak = 0;
	for (int y = 0; y < h; ++y)
	{
		const uchar* row = image->get_row(y);
		bool last = false;
		int runs = 0;
		int runsWidth = 0;
		int currWidth = 0;
		int firstWidth = 0;
		bool isValidRow = true;
		for (int x = 0; x < w; ++x)
		{
			bool cur = (row[x]>128);
			if (cur)
			{
				currWidth++;

			}
			if (!cur && last && currWidth >= 7 && currWidth < 66)
			{
				if (0 == runsWidth)
				{
					firstWidth = currWidth;
				}
				else if (runsWidth == 1 && !checkProportion(firstWidth, currWidth))
				{
					isValidRow = false;
					break;
				}
				else if (runsWidth >= 2)
				{
					isValidRow = false;
					break;
				}
				//get here when it first legs, or 2 legs and proportional
				++runsWidth;
			}
			if (!cur && last)
			{
				++runs;
			}
			if (!cur)
			{
				currWidth = 0;
			}
			last = cur;
		}
		if (runsWidth == 2 && isValidRow)
		{
			++count;
			++streak;
			largest_streak = Max(streak, largest_streak);
		}
		else
			streak = 0;
	}
	bool isManFound = ((double(count) / h)>threshold) && ((double(largest_streak)) / h > THRESHOLD_LARGEST);
	return isManFound;
}

int main()
{
	bool isGood = true;
	//row - threshold change
	//colom // 0-TPR, 1-FPR
	//evaluate function algorithm number
	double matrix[11][2][3];
	for (int k = 0; k < 2; k++)
	{
		int NUM_OF_PICTURES;
		if (isGood)
		{
			NUM_OF_PICTURES = 254; //4705; 254;
		}
		else
		{
			NUM_OF_PICTURES = 4705; //4705; 254;
		}
		double threshold = 0;
		for (int index = 0; index < 11; index++)
		{		
			double truePicture[3] = {0};
			for (int i = 1; i <= NUM_OF_PICTURES; i++)
			{
				std::stringstream rgbSTR;
				std::stringstream depthSTR;
				if (isGood)
				{
					rgbSTR << "C:\\Dropbox\\ROS\\photoTemp\\good\\r (" << i << ").jpg";
				}
				else
				{
				
					rgbSTR << "C:\\Dropbox\\ROS\\photoTemp\\bad\\inDoor" << i << ".jpg";
				}
				// rgbSTR << "c:\\temp\\verygood\\r (" << i << ").jpg";
				//cout << rgbSTR.str() << endl;
				CPP::Image& depthImage = Image(new CPPImage(rgbSTR.str()));
				if (depthImage == NULL)
				{
					continue;
				}
				depthImage = convert_to_gray(depthImage);
				if (evaluate1(depthImage, threshold))
				{
					truePicture[0]++;
				}
				if (evaluate2(depthImage, threshold))
				{
					truePicture[1]++;
				}
				if (evaluate3(depthImage, threshold))
				{
					truePicture[2]++;
				}
			}
			if (isGood)
			{
				for (int l = 0; l < 3; l++)
				{
					matrix[index][0][l] = truePicture[l] / NUM_OF_PICTURES;
					cout << "Algo=" << l + 1 << " threshold=" << threshold << " TPR=" << matrix[index][0][l] << endl;
				}	
			}
			else
			{
				for (int l = 0; l < 3; l++)
				{
					matrix[index][1][l] = truePicture[l] / NUM_OF_PICTURES;
					cout << "Algo=" << l + 1 << " threshold=" << threshold << " FPR=" << matrix[index][1][l] << endl;
				}
			}
			threshold += 0.1;
		}
		cout << endl;
		isGood = false;
	}
	ofstream myfile;
	myfile.open("C:\\Dropbox\\ROS\\photoTemp\\evaluate.csv");
	for (int l = 0; l < 4; l++)
	{
		myfile << "Algo" << l + 1 << endl;
		myfile << "FPR,TPR" << endl;
		for (int i = 0; i < 11; i++)
		{
			myfile << matrix[i][1][l] << "," << matrix[i][0][l] << endl;
		}
	}
	myfile.close();
	system("pause");
}

int oldMain2()
{
	int NUM_OF_PICTURES = 258; //4705; 258;
	for (int i = 1; i <= NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "c:\\temp\\good\\r (" << i << ").jpg";
		// rgbSTR << "c:\\temp\\verygood\\r (" << i << ").jpg";
		// rgbSTR << "c:\\temp\\bad\\inDoor"<<i<<".jpg";
		cout << rgbSTR.str() << endl;
		CPP::Image& depthImage = Image(new CPPImage(rgbSTR.str()));
		if(depthImage == NULL)
		{
			continue;
		}
		depthImage = convert_to_gray(depthImage);
		//myevaluate(depthImage,i);
	}
	return 0;
}

int oldmain()
{
	int NUM_OF_PICTURES = 382;
	int x=0,y=0;
	bool isMan=false;
	SMOSVMClassifier cls;
	if (!cls.load("classifier.xml")) //failed loading file
	{
		throw string("Classifier.xml not found");
	} 
	ClsEvaluator te(cls);
	for (int i = 0; i < NUM_OF_PICTURES; i++)
	{
		std::stringstream rgbSTR;
		std::stringstream depthSTR;
		rgbSTR << "C:\\temp\\TalTesting\\TalTesting" << i << "rgb.jpg";
		depthSTR << "C:\\temp\\TalTesting\\TalTesting" << i << "depth.jpg";
		CPP::Image& rgbImage = Image(new CPPImage(rgbSTR.str()));
		CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
		depthImage = convert_to_gray(depthImage);

		Image target=CFrameManager::extract_man(rgbImage,depthImage,&te,x,y,isMan);
	}

	system("pause");
	return 0;
}

void CreateImage()
{
	static int i = 0;

	std::stringstream rgbSTR;
	std::stringstream depthSTR;
	rgbSTR << "C:\\temp\\TalTesting" << i << "rgb.jpg";
	depthSTR << "C:\\temp\\TalTesting" << i << "depth.jpg";
	
	CPP::Image& rgbImage = Image(new CPPImage(640,480,3));
	CPP::Image& depthImage = Image(new CPPImage(640,480,1));
	//sensor->update(rgbImage,depthImage);
	CPP::CPPImage* imageRGB = rgbImage.get();
	CPP::CPPImage* imageDepth = depthImage.get();
	imageRGB->save(rgbSTR.str());
	imageDepth->save(depthSTR.str());
	CPPImage* r =	rgbImage.get();
	CPPImage* d =	depthImage.get();
	r->~CPPImage();
	d->~CPPImage();

	i++;
}
