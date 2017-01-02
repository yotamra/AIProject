#include "KeyboardThread.h"

void KeyboardThread::run()
  {
    char ch=0;
    bool done=false;
    //Checks for keyboard instructions until an escape button is pressed.
    while (!done)
    {
      if (GetKeyState(VK_ESCAPE) < 0) done=true;
      if (GetKeyState(VK_BACK) < 0) done=true;
      if (GetKeyState('1')<0) 
      {
        m_Paused=true;
        m_Initialize=false;
        close_wavs_and_play_new_wav("paused");
      }
      if (GetKeyState('2')<0) 
      { 
        if (m_Paused) close_wavs_and_play_new_wav("resuming");
        m_Paused=false;
        m_Record=false;
      }
      if (GetKeyState('9')<0) 
      {
		std::cout << "recording" << std::endl;
        m_Record=true;
        close_wavs_and_play_new_wav("recording");
      }
	  if (GetKeyState('0')<0)
	  {
        std::cout << "Initialize" << std::endl;
		    m_Initialize=true;
        m_Paused=true;
		    //no sound here as it colides with the other voice messages of initialization
	  }

    if (GetKeyState('5')<0)
	  {
        std::cout << "Pinpointing pressed" << endl;
		    m_pinpoint = true;        
	  }

      Sleep(100);
    }
  }