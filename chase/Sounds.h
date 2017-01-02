#ifndef Sounds_h__
#define Sounds_h__

#include <iomanip> 
#include <conio.h>

/*
Plays asynchronously the wav file from the path in "filename". If loop is true, the file is played
repeatedly until "close_all_waves" is called.
*/
void play_wave(const char* filename, bool loop);

/*
Closes all wav playing that were called before.
*/
void close_all_wavs();

/*
Plays once a wav file in "C:\\shared\\sounds\\voices\\" by the file's name specified without .wav extention.
In case there was a previous wav playing when the method is called, the old wav closed.
*/
void close_wavs_and_play_new_wav(const char* name);

/*
Plays once a wav file in "C:\\shared\\sounds\\voices\\" by the file's name specified without .wav extention.
In case there was a previous wav playing in the last time_interval (default is 1.5 secs), 
the new wav file won't be played.
*/
void play_wav_clip(const char* name, int time_interval=1500);

#endif