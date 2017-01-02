#include "Sounds.h"
#include "image.h"


void play_wave(const char* filename, bool loop)
{
  DWORD flags=SND_ASYNC;
  if (loop) flags|=SND_LOOP;
  PlaySoundA(filename,0,flags);
}

void close_all_wavs()
{
  PlaySound(0,0,0);
}

void close_wavs_and_play_new_wav(const char* name)
{
  close_all_wavs();
  std::ostringstream os;
  os << "sounds\\voices\\" << name << ".wav";
  play_wave(os.str().c_str(),false);
}

void play_wav_clip(const char* name, int time_interval)
{
  static unsigned last_play=0;
  unsigned cur=GetTickCount();//gets the number of milliseconds elpsed since the system was started.
  if ((cur-last_play)>(unsigned)time_interval)
  {
    std::ostringstream os;
    os << "sounds\\voices\\" << name << ".wav";
    play_wave(os.str().c_str(),false);
    last_play=cur;
  }
}