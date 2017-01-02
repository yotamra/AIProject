#ifndef avi_h__
#define avi_h__

#include "image.h"

class AVICallback
{
public:
  virtual void process_frame(CPP::Image frame) = 0;
  virtual void set_frame_rate(int fps) {}
};

bool extract_avi_frames(const char* szFileName, AVICallback* cb);


class AVIWriter
{
  class AVIWriterImpl* m_Impl;
public:
  AVIWriter(const char* filename, int w, int h, int bpp, int frame_rate);
  ~AVIWriter();
  void write_frame(CPP::Image frame);
};

#endif // avi_h__
