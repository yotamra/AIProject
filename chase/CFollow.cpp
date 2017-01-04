#include "CFollow.h"

#undef Work_With_FallDetector
using namespace CPP; 
using namespace ML;

// opens a log file for the following

ofstream flog("follow.log");

CFollow::CFollow()
{
  m_DEBUG = DEBUG0;
  frameNumber = 0;
}

CFollow::~CFollow()
{
  close_wavs_and_play_new_wav("goodbye");
  m_Robot->stop();
  m_Sampler.terminate(); 
  Sleep(1000);
  delete m_Robot;
}


void CFollow::Init()
{
  play_wav_clip("chase menu");
  std::cout << "Chasing...\n";
  
#ifndef NO_ROBOT
  m_Robot = RobotInterface::create(4, 38400);
#endif

  // Keyboard
  m_Kbd.start();
  Sleep(100);
  
// Camera
  m_Sampler.start(); 
  m_Sampler.wait_for_finished_initializing();
  //  Sleep(5000);  // handle sampler crashing

#ifdef Work_With_FallDetector
  //thread that detects if the person followed falls
  m_pFall_Detector = new FallDetector(&m_Sampler); 
#endif // Work_With_FallDetector

  m_bPaused=false;

  Image rgb_frame(new CPPImage(640, 480, 3));
  m_Sampler.wait_for_finished_initializing();   // barrier for camera thread
}


void CFollow::KeyRecording()
{
  std::cout << "recording after kdb" << std::endl;
  unsigned tc=GetTickCount();
  unsigned long long secs = time(NULL);
  if (m_bPersonFound && NULL != m_iTarget)
  {
    close_wavs_and_play_new_wav("taking picture");
    //need to create the folder before!!
    string path = "C:\\temp\\pics\\";
    std::ostringstream os;	
    os << path << secs << "_" << tc << "_person" << ".png";
    //cout << os.str()<<std::endl;
	m_iTarget->save(os.str());
     
   {
     std::ostringstream os;
     os << path << secs << "_" << tc << "_frame" << ".png";
     //cout << os.str()<<std::endl;
     m_iFrame->save(os.str());
   }

   {
     std::ostringstream os;
     os << path << secs << "_" << tc << "_rgb_frame" << ".png";
     //cout << os.str()<<std::endl;
     m_iRgb_Frame->save(os.str());
   }

  }

}

void CFollow::KeyInit()
{
  cout << "===============================================================" << endl;
  cout << "Starting initialization" << endl;
  cout << "===============================================================" << endl;
  // init tracked person histogram
  bool result = CFrameManager::initPersonHist(m_iRgb_Frame,m_iFrame, m_Te,m_x,m_y,m_iTarget);
  cout << "*********** res " << result  << "x,y = " <<m_x <<"," <<m_y<< endl;
  if (result)
  {
    m_bPersonFound = true;
    play_wav_clip("initialization successful",400);
    m_Kbd.stopInit();
    cout << "init success *************************************************" << endl;
    m_FollowParameters.state = STATE_INIT;
    m_FollowParameters.searchDirection = STAY;
    m_FollowParameters.counterPersonFound = 0;
    m_FollowParameters.counterPersonNotFound = 0;
  }
  else
  {
    cout << "failed perform init *************************************************" << endl;
    play_wav_clip("initialization failed");
  }
}

void CFollow::KeyPinpointing()
{
  cout << "===============================================================" << endl;
  cout << "Starting Pinpointing" << endl;
  cout << "===============================================================" << endl;
  current_position_comparing_optimal(m_iRgb_Frame,m_iFrame);
  m_Kbd.stop_pinpointing();
}

void CFollow::KeyPaused()
{
    if (!m_bPaused) 
    {
      std::cout << "Paused\n";
#ifndef NO_ROBOT
      m_Robot->stop();
#endif
	}
#ifdef Work_With_FallDetector
	m_pFall_Detector->pause();
#endif // Work_With_FallDetector
    m_bPaused=true;
    Sleep(100);
}

void CFollow::ReportError(ROBOT_ERROR error)
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


void CFollow::DriveControl()
{
   // self search
   if (SUB_STATE_SEARCH_CONTINUSE == m_FollowParameters.subState &&
       STATE_SEARCH == m_FollowParameters.state &&
	   m_FollowParameters.counterPersonNotFound < SEARCH_TARGET_COUNTER)
       {
          // DO NOTHING
       }
       else
       {
           if (m_DEBUG >= DEBUG2)
           {
			   flog << "Drive: " << m_FollowParameters.speedLeft << " " << m_FollowParameters.speedRight << std::endl;
           }
		   short speedLeft = SPEED_COEFFICIENT * m_FollowParameters.speedLeft;
		   short speedRight = SPEED_COEFFICIENT * m_FollowParameters.speedRight;
		   m_Robot->drive(speedLeft, speedRight);

		   if (STATE_SEARCH == m_FollowParameters.state)
           {
               Sleep(ROTATE_SLEEP_TIME);
			   m_Robot->stop();
	           m_FollowParameters.counterPersonNotFound = 0;
           }
       }
}


void CFollow::ChangeState()
{
	switch (m_FollowParameters.state)
	{
		case STATE_INIT:
			m_FollowParameters.state = STATE_SEARCH;
			m_FollowParameters.subState = SUB_STATE_SEARCH_ENTRANCE;
			if (m_DEBUG >= DEBUG0)
			{
				cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
			}
			if (m_DEBUG >= DEBUG1)
			{
				flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
			}
			break;
		case STATE_SEARCH:
			switch (m_FollowParameters.subState)
			{
				case SUB_STATE_SEARCH_ENTRANCE:
					m_FollowParameters.subState = SUB_STATE_SEARCH_CONTINUSE;
					if (m_DEBUG >= DEBUG0)
					{
						cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_CONTINUSE" << endl;
					}
					if (m_DEBUG >= DEBUG1)
					{
						flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_CONTINUSE" << endl;
					}
					break;
				case SUB_STATE_SEARCH_CONTINUSE:
					if (m_FollowParameters.counterPersonFound >= PERSON_FOUND_COUNTER)
					{
						play_wav_clip("found you");
						m_FollowParameters.state = STATE_FOLLOW;
						m_FollowParameters.subState = SUB_STATE_FOLLOW_TARGET;
						m_FollowParameters.counterPersonFound = 0;
						m_FollowParameters.counterPersonNotFound = 0;
						if (m_DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
						if (m_DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
					}
					break;
				default:
					ReportError(ILLEGEAL_SUBSTATE);
					break;
			}
			break;
		case STATE_FOLLOW:
			switch (m_FollowParameters.subState)
			{
				case SUB_STATE_FOLLOW_TARGET:
					if (m_FollowParameters.counterPersonNotFound > 0)
					{
						m_FollowParameters.subState = SUB_STATE_FOLLOW_LOSING;
						m_FollowParameters.counterPersonFound = 0;
						if (m_DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_LOSING" << endl;
						}
						if (m_DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_LOSING" << endl;
						}
					}
					break;
				case SUB_STATE_FOLLOW_LOSING:
					if (m_FollowParameters.counterPersonFound > 0)
					{
						m_FollowParameters.subState = SUB_STATE_FOLLOW_TARGET;
						m_FollowParameters.counterPersonNotFound = 0;
						if (m_DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
						if (m_DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_FOLLOW -> SUB_STATE_FOLLOW_TARGET" << endl;
						}
					}
					else if (m_FollowParameters.counterPersonNotFound >= LOSING_TARGET_COUNTER)
					{
						play_wav_clip("target lost");
						//play_wav_clip("where-are-we-going");
						
						m_FollowParameters.state = STATE_SEARCH;
						m_FollowParameters.subState = SUB_STATE_SEARCH_ENTRANCE;
						m_FollowParameters.counterPersonFound = 0;
						m_FollowParameters.counterPersonNotFound = 0;
						if (m_DEBUG >= DEBUG0)
						{
							cout << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
						}
						if (m_DEBUG >= DEBUG1)
						{
							flog << endl << "STATE_SEARCH -> SUB_STATE_SEARCH_ENTRANCE" << endl;
						}
					}
					break;
				default:
					ReportError(ILLEGEAL_SUBSTATE);
					break;
			}
			break;
		default:
			ReportError(ILLEGEAL_STATE);
			break;
	}
	switch (m_FollowParameters.state)
	{
		case STATE_FOLLOW:
			FollowState();
			break;
		case STATE_SEARCH:
			SearchState();
			break;
		default:
			ReportError(ILLEGEAL_STATE);
			break;
	}
}



void CFollow::SearchState()
{
	if (m_DEBUG >= DEBUG2)
	{
		flog << m_FollowParameters.subState << endl;
	}
	if (SUB_STATE_SEARCH_ENTRANCE == m_FollowParameters.subState)
	{
		m_FollowParameters.counterPersonNotFound = 1;
		//store the postions of all the targets in the current frame
		std::vector<double> targetPositions;
		//itatrate all the targets and store their postions.
		for (tgt_seq::iterator it = m_FollowParameters.targets.begin(); it != m_FollowParameters.targets.end(); ++it)
		{
			const Vec2D& position = it->position;
			if (m_DEBUG >= DEBUG2)
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
			m_FollowParameters.targetsMedian = targetPositions[targetPositions.size() / 2];
		}

		if (m_FollowParameters.targetsMedian <= (FRAME_WIDTH / 2.0))
		{
			m_FollowParameters.searchDirection = LEFT_TURN;
		}
		else
		{
			m_FollowParameters.searchDirection = RIGHT_TURN;
		}

	}
	else if (m_FollowParameters.counterPersonNotFound >= SEARCH_TARGET_COUNTER)
	{
		int turn = m_FollowParameters.searchDirection;
		m_FollowParameters.speedLeft = (-TURN_SPEED)	*	turn;
		m_FollowParameters.speedRight = TURN_SPEED	*	turn;
		if (m_DEBUG >= DEBUG2)
		{
			flog << "speedLeft = " << m_FollowParameters.speedLeft << ", speedRight = " << m_FollowParameters.speedRight << endl;
		}
	}
	else
	{
		m_FollowParameters.speedLeft = 0;
		m_FollowParameters.speedRight = 0;
	}

	if (m_DEBUG >= DEBUG1)
	{
		flog << "targetMedian = "			<< m_FollowParameters.targetsMedian				<< endl;
		flog << "searchDirection = "		<< m_FollowParameters.searchDirection			<< endl;
		flog << "counterPersonNotFound = "	<< m_FollowParameters.counterPersonNotFound		<< endl;
		flog << "counterPersonFound = "		<< m_FollowParameters.counterPersonFound		<< endl;
	}
	return;
}


void CFollow::FollowState()
{
	// Following a person:
	
	//store the postions of all the targets in the current frame
	std::vector<double> targetPositions;

	//itatrate all the targets and store their postions.
	for (tgt_seq::iterator it = m_FollowParameters.targets.begin(); it != m_FollowParameters.targets.end(); ++it)
	{
		const Vec2D& position = it->position;
		if (m_DEBUG >= DEBUG2)
		{
			flog << "fp=" << int(position.x) << ' ';
		}
		targetPositions.push_back(position.x);

	}
	if (!targetPositions.empty())
	{
		if (m_DEBUG >= DEBUG2)
		{
			flog << endl;
		}
		//sorting the targets by postion
		std::sort(targetPositions.begin(), targetPositions.end());
		m_FollowParameters.targetsMedian = targetPositions[targetPositions.size() / 2];
	}
	else if (m_FollowParameters.subState == SUB_STATE_FOLLOW_LOSING)
	{
		// DO NOTHING - dont change median, save it for the case target will be lost compleitly in the future 
		// so we know to start search n the right direction (and for velocity calculations)
	}
	else
	{
		ReportError(TARGET_NOT_FOUND);
	}
	int speedy = MAGIC_SPEED;
	int speedx = int((m_FollowParameters.targetsMedian - (FRAME_WIDTH / 2.0)) * MAGIC_COEFFICIENT);
	m_FollowParameters.speedLeft = speedy + speedx;
	m_FollowParameters.speedRight = speedy - speedx;

	//in case left or right speed exceeds legal speed - set limit speed:
	if (m_FollowParameters.speedLeft<MIN_SPEED)		m_FollowParameters.speedLeft = MIN_SPEED;
	if (m_FollowParameters.speedRight<MIN_SPEED)	m_FollowParameters.speedRight = MIN_SPEED;
	if (m_FollowParameters.speedLeft>MAX_SPEED)		m_FollowParameters.speedLeft = MAX_SPEED;
	if (m_FollowParameters.speedRight>MAX_SPEED)	m_FollowParameters.speedRight = MAX_SPEED;
	
	switch (m_FollowParameters.subState)
	{
		case SUB_STATE_FOLLOW_TARGET:
			FollowTarget();
			break;
		case SUB_STATE_FOLLOW_LOSING:
			LosingTarget();
			break;
		default:
			ReportError(ILLEGEAL_SUBSTATE);
			break;
	}
}

void CFollow::FollowTarget()
{
   return;
}

bool CFollow::CheckKeyboardStatus()
// Check the keyboard status and returns true if the 'Pause' key is pressed
{
	bool bKeyPasuedFlag = false;

	// Key Recording
	if (m_Kbd.recording())
	{
		KeyRecording();
	}

	// Key Init
	if (m_Kbd.init())
	{
		KeyInit();
	}

	// Key Pinpointing
	if (m_Kbd.pinpointing())
	{
		KeyPinpointing();
	}

	// Key Paused
	if (m_Kbd.paused())
	{
		KeyPaused();
		bKeyPasuedFlag = true;
	}

	return (bKeyPasuedFlag);
}

void CFollow::GeometricCenterCalc(Image target)
// Calculate the geometric center of the person
{
	if ((m_bPersonFound == true) && (target != NULL))
	{
		Vec2D cg = calculate_centroid(target);
		cg.x += m_x;
		cg.y += m_y;
		m_FollowParameters.targets.push_back(TargetPosition(cg));
	}
	else
	{
		m_FollowParameters.targets.clear();
	}
}


void CFollow::LosingTarget()
{
   int speedDecrease = (m_FollowParameters.counterPersonNotFound - SLOW_COUNTER) * SPEED_DECREASE_COEFFICIENT;
   if (speedDecrease >= abs(m_FollowParameters.speedLeft) || speedDecrease >= abs(m_FollowParameters.speedRight))
   {
	   m_FollowParameters.speedLeft = 0;
	   m_FollowParameters.speedRight = 0;
   }
   else
   {
	   m_FollowParameters.speedLeft = (m_FollowParameters.speedLeft >= 0) ? (m_FollowParameters.speedLeft - speedDecrease) : (m_FollowParameters.speedLeft + speedDecrease);
	   m_FollowParameters.speedRight = (m_FollowParameters.speedRight >= 0) ? (m_FollowParameters.speedRight - speedDecrease) : (m_FollowParameters.speedRight + speedDecrease);
   }
}

void CFollow::RunningAndFollow()
{
  bool TakeSampleFromCamera = false;
  //run until told to stop
  cvNamedWindow("RGBImage:", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("EDepthImage:", CV_WINDOW_AUTOSIZE);
  while (m_Kbd.is_active())
  { 
	// It will print FPS every few loops
    m_Timer.sample();    

	m_Sampler.update(m_iRgb_Frame, m_iFrame);
	m_iFrame = convert_to_gray(m_iFrame);
	//CFrameManager::displayImage(m_iFrame);
	cvShowImage("RGBImage:", ((CPPImage*)m_iRgb_Frame.get())->get());
	cvShowImage("EDepthImage:", ((CPPImage*)m_iFrame.get())->get());
	cvWaitKey(20);
    // target is now the person if personFound==true
    Image target=CFrameManager::extract_man(m_iRgb_Frame,m_iFrame,m_Te,m_x,m_y,m_bPersonFound);
    
	if (m_bPersonFound)
    {
		m_FollowParameters.counterPersonNotFound = 0;
		m_FollowParameters.counterPersonFound++;
    }
    else
    {
		m_FollowParameters.counterPersonNotFound++;
		m_FollowParameters.counterPersonFound = 0;
    }
    
	// Check the keyboard status and returns true if the 'Pause' key is pressed
	if (CheckKeyboardStatus() == true)
	{
		continue;
	}

    m_bPaused=false; 

#ifdef Work_With_FallDetector
	m_pFall_Detector->resume();
#endif Work_With_FallDetector

	// Calculate the geometric center of the person
	GeometricCenterCalc(target);

    ChangeState();

    // fall detection code calling
#ifdef Work_With_FallDetector
	m_pFall_Detector->set_person_in_frame_status(m_bPersonFound);
	if (target != NULL)
	{
		if (m_bPersonFound == true)
		{
			m_pFall_Detector->provide_person(target, m_bPersonFound, m_x, m_y, m_FollowParameters.speedLeft, m_FollowParameters.speedRight);
		}
		m_pFall_Detector->run();
	}	
	//m_pFall_Detector->provide_person(target, m_bPersonFound, m_x, m_y, m_FollowParameters.speedLeft, m_FollowParameters.speedRight);
#endif // Work_With_FallDetector

	// Set the velocities to the robot drive
#ifndef NO_ROBOT
	DriveControl();
#endif

  } // end while()

}
