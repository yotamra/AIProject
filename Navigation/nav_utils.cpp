#include "nav_utils.h"
#ifdef ROBOT
void play_wave(const char* filename, bool loop)
{
  DWORD flags=SND_SYNC;
  if (loop) flags|=SND_LOOP;
  //WCHAR st[100];
  //MultiByteToWideChar(0,0,filename,100,st,100);
  //PlaySound((LPCWSTR4)st,0,flags);
  PlaySoundA(filename,0,flags);
}

void play_wav_clip(const char* name)
{
  static unsigned last_play=0;
  unsigned cur=GetTickCount();
  if ((cur-last_play)>1500)
  {
    std::ostringstream os;
    os << "C:\\shared\\Navigation\\sounds\\voices\\" << name << ".wav";
    play_wave(os.str().c_str(),false);
  }
  last_play=cur;
}
#endif