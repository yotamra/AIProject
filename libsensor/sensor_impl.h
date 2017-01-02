#ifndef sensor_impl_h__
#define sensor_impl_h__

#include "sensor.h"
#include <XnCppWrapper.h>
#include <iostream>
#include <fstream>

// assaf hilla
#include "CFrameManager.h"

using namespace CPP;

//forward declaration of the function in the end of this file
Image extract_man(Image frame, TargetEvaluator* te, int& off_x, int& off_y);

class SensorImpl : public Sensor
{
  typedef std::vector<int> ivec;
  XnStatus           m_Status;
  xn::Context        m_Context;
  xn::DepthGenerator m_Depth;
  xn::ImageGenerator m_RGBGen;
  Image              m_Image;
  Matrix<ushort>     m_Matrix;
  ivec               m_Histogram;

public:
  SensorImpl()
    : m_Image(new CPPImage(640,480,1)),
      m_Matrix(640,480),
      m_Histogram(65536)
  {
    int i = 0;
    cout <<"at creator" << i <<endl; i++;
    m_Status=m_Context.Init();
    cout <<"at creator" << i <<endl; i++;
    m_Status=m_Depth.Create(m_Context);
    cout <<"at creator" << i <<endl; i++;
    m_Status=m_RGBGen.Create(m_Context);
    cout <<"at creator" << i <<endl; i++;
    XnMapOutputMode outputMode;
    cout <<"at creator" << i <<endl; i++;
    outputMode.nXRes = 640;
    cout <<"at creator" << i <<endl; i++;
    outputMode.nYRes = 480;
    cout <<"at creator" << i <<endl; i++;
    outputMode.nFPS = 30;
    cout <<"at creator" << i <<endl; i++; //this is line 7
    m_Status=m_Depth.SetMapOutputMode(outputMode);
    cout <<"at creator" << i <<endl; i++;
    m_RGBGen.SetMapOutputMode(outputMode);
    cout <<"at creator" << i <<endl; i++;
    m_RGBGen.SetPixelFormat(XN_PIXEL_FORMAT_RGB24);
    cout <<"at creator" << i <<endl; i++;
    m_Status=m_Context.StartGeneratingAll();
    cout <<"at creator" << i <<endl; i++;
  }

  ~SensorImpl()
  {
    m_Context.Shutdown();
  }

  bool update(Image& rgb, Image& depth)
  {
    //depth=update();
    update(depth);
    const XnUInt8* buffer=m_RGBGen.GetImageMap();
    if (rgb->get_width()!=640 || rgb->get_height()!=480 || rgb->get_channels()!=3)
    {
      rgb=Image(new CPPImage(640,480,3));
    }
    byte* dst=rgb->get_row(0);
    int sz=640*480*3;
    std::copy(buffer,buffer+sz,dst);
    return true;
  }

  void update(Image& depth)
  {
    m_Status=XN_STATUS_OK+1;

    while (true)
    {
      m_Status=m_Context.WaitOneUpdateAll(m_Depth);
      if (m_Status == XN_STATUS_OK) break;
      Sleep(10);
    }
    const XnDepthPixel* pDepthMap = m_Depth.GetDepthMap();
    ushort* mptr=&(m_Matrix(0,0));
    int N=640*480;
    m_Histogram.assign(65536,0);
    XnDepthPixel mn=65535,mx=0;
    for(int i=0;i<N;++i,++pDepthMap)
    {
      XnDepthPixel p=*pDepthMap;
      *mptr++ = p;
      m_Histogram[p]++;
      mn=Min(mn,p);
      mx=Max(mx,p);
    }
    //std::ofstream fout("matrix.m");
    //fout << "A=[";
    double mult=255.0/(mx-mn);
    for(unsigned y=0;y<480;++y)
    {
      //byte* row=m_Image->get_row(y);
      byte* row=depth->get_row(y);
      for(unsigned x=0;x<640;++x)
      {
        //fout << m_Matrix(x,y) << " ";
        row[x]=byte((m_Matrix(x,y)-mn)*mult);
      }
      //if (y<479) fout << "; ";
    }
    //fout << "];";
  }

  Image extract_man(Image colorFrame,Image depthFrame, TargetEvaluator* te, int& off_x, int& off_y,bool& personFound)
  {
    return CFrameManager::extract_man(colorFrame,depthFrame,te,off_x,off_y,personFound);
  }
};

#endif // sensor_impl_h__
