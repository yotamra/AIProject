#ifndef H_ROBOT_INTERFACE
#define H_ROBOT_INTERFACE


// Abstact robot control interface
class RobotInterface
{
public:
	virtual bool drive(short leftVelocity, short rightVelocity){ return false; }
	virtual bool stop(){return false;}
	virtual ~RobotInterface(){}
  // Create an instance of the interface's implementation
	static RobotInterface* create(int com_port, int speed);
};

#endif // H_ROBOT_INTERFACE