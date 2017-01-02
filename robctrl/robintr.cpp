#include <iostream>
#include "robintr.h"
#include <Aria.h>
using namespace std;


/*
  This is a connection handler class, to demonstrate how to run code in
  response to events such as the program connecting an disconnecting
  from the robot.
*/
class ConnHandler
{
public:
  // Constructor
  ConnHandler(ArRobot *robot);
  // Destructor, its just empty
  ~ConnHandler(void) {}
  // to be called if the connection was made
  void connected(void);
  // to call if the connection failed
  void connFail(void);
  // to be called if the connection was lost
  void disconnected(void);
protected:
  // robot pointer
  ArRobot *myRobot;
  // the functor callbacks
  ArFunctorC<ConnHandler> myConnectedCB;
  ArFunctorC<ConnHandler> myConnFailCB;
  ArFunctorC<ConnHandler> myDisconnectedCB;
};

ConnHandler::ConnHandler(ArRobot *robot) :
  myConnectedCB(this, &ConnHandler::connected),  
  myConnFailCB(this, &ConnHandler::connFail),
  myDisconnectedCB(this, &ConnHandler::disconnected)

{
  myRobot = robot;
  myRobot->addConnectCB(&myConnectedCB, ArListPos::FIRST);
  myRobot->addFailedConnectCB(&myConnFailCB, ArListPos::FIRST);
  myRobot->addDisconnectNormallyCB(&myDisconnectedCB, ArListPos::FIRST);
  myRobot->addDisconnectOnErrorCB(&myDisconnectedCB, ArListPos::FIRST);
}

// just exit if the connection failed
void ConnHandler::connFail(void)
{
  printf("directMotionDemo connection handler: Failed to connect.\n");
  myRobot->stopRunning();
  Aria::exit(1);
  return;
}

// turn on motors, and off sonar, and off amigobot sounds, when connected
void ConnHandler::connected(void)
{
  printf("directMotionDemo connection handler: Connected\n");
  myRobot->comInt(ArCommands::SONAR, 0);
  myRobot->comInt(ArCommands::ENABLE, 1);
  myRobot->comInt(ArCommands::SOUNDTOG, 0);
}

// lost connection, so just exit
void ConnHandler::disconnected(void)
{
  printf("directMotionDemo connection handler: Lost connection, exiting program.\n");
  Aria::exit(0);
}


// Implementation of the control interface
class RobotInterface_Impl : public RobotInterface
{

public:
	RobotInterface_Impl(int com_port, int speed)
	{
		char comPort[6]={"COM"};
		char port[3];
		itoa(com_port,port,10);
		strcat(comPort,port);
		Aria::init();
		char *argv[3];
		int argc = 3;
		argv[0] = "chase.exe";
		argv[1] = "-rp";
		argv[2] = comPort;
  
		ArArgumentParser argParser(&argc, argv);
		argParser.loadDefaultArguments();
		robot = new ArRobot();
		con  = new ArRobotConnector(&argParser, robot);
		

		//// the connection handler from above
		 ch = new ConnHandler(robot);

		if(!con->connectRobot())
		{
			if(!argParser.checkHelpAndWarnUnparsed()) 
			{
				ArLog::log(ArLog::Terse, "Could not connect to robot, will not have parameter file os options displayed later may not include everything");
			}
			else
			{
				ArLog::log(ArLog::Terse,"Error, could not connect to robot.");
				Aria::logOptions();
 			}
			Aria::exit(1);
			exit(1);
		}
		if(!robot->isConnected())
		{
			 ArLog::log(ArLog::Terse,"Internal error: robot connector succeeded but ArRobot::isConnected() is false!");
		}

		//// Run the robot processing cycle in its own thread. Note that after starting this
		//// thread, we must lock and unlock the ArRobot object before and after
		//// accessing it.
		robot->runAsync(false);	
	}
	virtual ~RobotInterface_Impl() 
	{
		con->disconnectAll();
		delete con;
		delete robot;
	}

	bool drive(short leftVelocity, short rightVelocity)
	{
		robot->lock();
		robot->setVel2(leftVelocity, rightVelocity);
		robot->unlock();
		ArUtil::sleep(35);
		return true;
	}

	bool stop()
	{
		robot->lock();
		robot->stop();
		robot->unlock();
		return true;
	}
private:
	ArRobot* robot;
	ArRobotConnector* con;
	ConnHandler* ch;
};


RobotInterface* RobotInterface::create(int com_port, int speed)
{
  return new RobotInterface_Impl(com_port,speed);
}
