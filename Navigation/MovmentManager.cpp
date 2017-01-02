#include "MovmentManager.h"
#include <math.h>

#define DEG_TO_RAD(d) (((d)*PI)/180)
MovmentManager::MovmentManager(Map* map)
{
	m_map = map;
  m_prevDirection = NONE;
  m_prevFactor = 100;
#ifdef ROBOT
	m_robot=RobotInterface::create(1,38400);
#endif
}


void MovmentManager::stopRobot()
{
	m_robot->drive(0,0);
}

void MovmentManager::moveRobotInCorr(DirectionEnum direction, double factor)
{
  //factor*=2;
  cout << "direction = " << direction << " prevDirection = " << m_prevDirection << " factor = " << factor << " prev factor = " << m_prevFactor << endl;
  if ((direction == m_prevDirection) &&
      (abs(m_prevFactor - factor) < 0.01))
  {
    return;
  }
  double slow = 315;
  
  //factor *=4;
  m_prevFactor = factor;
  m_prevDirection = direction;
	double speed = (slow*factor) + slow;
  /*4*/
  printf("factor = %lf\n",factor);
  printf("speed = %lf\n", speed);
  if (speed > 512)
  {
    speed = 511;
  }
  if (direction == STRAIGHT)
	{
		m_robot->drive(slow,slow);
	}
	else if (direction == RIGHT)
	{
    printf("right\n");
		m_robot->drive(speed,slow);
	}
	else if (direction == LEFT)
	{
    printf("left\n");
		m_robot->drive(slow,speed);
	}
}

void MovmentManager::turnRobot(DirectionEnum direction)
{
	this->moveRobot(STRAIGHT,0);
	Sleep(5200);
	if (direction == RIGHT)
	{
		m_robot->drive(510,-510);
    Sleep(800);
    m_robot->drive(0,0);
    m_robot->drive(510,-510);
    Sleep(800);
    m_robot->drive(0,0);
    m_robot->drive(510,-510);
    Sleep(200);
    m_robot->drive(0,0);
    m_robot->drive(510,-510);
	}
	else if (direction == LEFT)
	{
		m_robot->drive(-510,510);
    Sleep(800);
    m_robot->drive(0,0);
    m_robot->drive(-510,510);
    Sleep(800);
    m_robot->drive(0,0);
    m_robot->drive(-510,510);
    Sleep(200);
    m_robot->drive(0,0);
    m_robot->drive(-510,510);
	}

	this->stopRobot();
}

void MovmentManager::moveRobot(DirectionEnum direction, double factor)
{
  cout << "direction = " << direction << " prevDirection = " << m_prevDirection << " factor = " << factor << " prev factor = " << m_prevFactor << endl;
  //if ((direction == m_prevDirection) &&
  //    (abs(m_prevFactor - factor) < 0.03))
  //{
  //  return;
  //}
  //
  ////factor *=10;
  //m_prevFactor = factor;
  //m_prevDirection = direction;
  
	double speed = (256*factor) + 256;

  printf("speed = %lf\n", speed);
  if (speed > 512)
  {
    speed = 511;
  }
	if (direction == STRAIGHT)
	{
		m_robot->drive(256,256);
	}
	else if (direction == RIGHT)
	{
		m_robot->drive(speed,256);
	}
	else if (direction == LEFT)
	{
		m_robot->drive(256,speed);
	}
}