#include "Map.h"
#include "MapPart.h"

#include <iostream>
using namespace std; 
#include "../robctrl/robintr.h"



class MovmentManager
{
public:
	MovmentManager(Map* map);

	void turnRobot(DirectionEnum direction);
	void moveRobot(DirectionEnum direction, double factor);
  void moveRobotInCorr(DirectionEnum direction, double factor);

	void stopRobot();

private:
	// out params : newAngle,delta,distance
	void calcNextMove(Coord& currPosition,double angle,Coord& nextPosition,
					  double& newAngle,double& delta,double& distance);
  DirectionEnum m_prevDirection;
  double m_prevFactor;
	Map* m_map;
	RobotInterface* m_robot;
};