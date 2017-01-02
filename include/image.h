#ifndef H_IMAGEPP
#define H_IMAGEPP

#include <cxcore.h>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include "prims.h"

namespace CPP {

typedef std::pair<int,int> int_pair;

inline void calculate_range(double* hist, int_pair& range, double edge)
{
  double sum=0;
  for(range.first=0;range.first<256;++range.first)
  {
    sum+=hist[range.first];
    if (sum>=edge) break;
  }
  sum=0;
  for(range.second=255;range.second>0;--range.second)
  {
    sum+=hist[range.second];
    if (sum>=edge) break;
  }
}

inline unsigned char* get_row(IplImage* image, int row)
{
  return (unsigned char*)(image->imageData + row*image->widthStep);
}

class CPPImage
{
  IplImage* m_Image;
  bool      m_Failed;

  static CvSize size(unsigned w, unsigned h)
  {
    CvSize sz;
    sz.width=w;
    sz.height=h;
    return sz;
  }
public:
  CPPImage(unsigned w, unsigned h, int channels,int depth = IPL_DEPTH_8U)
    : m_Image(cvCreateImage(size(w,h),depth,channels)),
      m_Failed(false)
  {}

  CPPImage(const char* filename)
    : m_Image(cvLoadImage(filename)),
      m_Failed(false)
  {
    if (!m_Image) m_Failed=true;
  }

  CPPImage(const std::string& filename)
    : m_Image(cvLoadImage(filename.c_str())),
      m_Failed(false)
  {
    if (!m_Image) m_Failed=true;
  }

  CPPImage(IplImage* img)
    : m_Image(cvCloneImage(img)),
      m_Failed(false)
  {}

  ~CPPImage()
  {
    cvReleaseImage(&m_Image);
  }

  bool failed() const { return m_Failed; }

  CPPImage* clone() 
  { 
    unsigned y,w=get_width(),h=get_height(),c=get_channels();
    CPPImage* img=new CPPImage(w,h,c);
    unsigned pitch=w*c;
    for(y=0;y<h;++y)
    {
      const byte* row=get_row(y);
      byte* dst=img->get_row(y);
      std::copy(row,row+pitch,dst);
    }
    return img;
  }

  IplImage* get() { return m_Image; }

  void save(const std::string& filename)
  {
    cvSaveImage(filename.c_str(),m_Image);
  }

  void save(const char* filename)
  {
    cvSaveImage(filename,m_Image);
  }

  unsigned get_pitch()
  {
    return m_Image->widthStep;
  }

  inline unsigned char* get_row(int row, int x=0)
  {
    unsigned char* ptr=CPP::get_row(m_Image,row);
    ptr+=x*get_channels();
    return ptr;
  }

  inline const unsigned char* get_row(int row, int x=0) const
  {
    const unsigned char* ptr=CPP::get_row(m_Image,row);
    ptr+=x*get_channels();
    return ptr;
  }

  unsigned get_width()  const   { return m_Image->width; }
  unsigned get_height() const   { return m_Image->height; }
  unsigned get_channels() const { return m_Image->nChannels; }
  Rect     get_rect() const     { return Rect(0,0,get_width(),get_height()); }

  bool calculate_histogram(double* hist)
  {
    if (get_channels()!=1) return false;
    std::fill_n(hist,256,0);
    unsigned w=get_width(),h=get_height(),x,y;
    for(y=0;y<h;++y)
    {
      const unsigned char* row=get_row(y);
      for(x=0;x<w;++x)
        hist[row[x]]+=1.0;
    }
    return true;
  }
};

typedef boost::shared_ptr<CPPImage> Image;

inline Image load(const std::string& filename) { return Image(new CPPImage(filename)); }
inline Image load(const char*        filename) { return Image(new CPPImage(filename)); }
inline Image attach(IplImage* image) { return Image(new CPPImage(image)); }

class CPPKernel
{
  IplConvKernel* m_Kernel;
public:
  // Create a structuring element of size w,h
  // anchor defaults to center
  // Use mask to specify content.  If mask is not used, a Rectangular element is used
  CPPKernel(int w, int h, int anchor_x=-1, int anchor_y=-1, IplImage* mask=0)
    : m_Kernel(0)
  {
    if (anchor_x<0) anchor_x=w/2;
    if (anchor_y<0) anchor_y=h/2;
    std::vector<int> values(w*h,0);
    if (mask)
    {
      std::vector<int>::iterator it=values.begin();
      for(int y=0;y<h;++y)
      {
        unsigned char* ptr=get_row(mask,y);
        for(int x=0;x<w;++x,++it,++ptr) *it=*ptr;
      }
      cvCreateStructuringElementEx(w,h,anchor_x,anchor_y,CV_SHAPE_CUSTOM,&values[0]);
    }
    else
    {
      cvCreateStructuringElementEx(w,h,anchor_x,anchor_y,CV_SHAPE_RECT);
    }
  }

  ~CPPKernel()
  {
    cvReleaseStructuringElement(&m_Kernel);
  }

  IplConvKernel* get() { return m_Kernel; }
};

typedef boost::shared_ptr<CPPKernel> Kernel;

template<class T>
class CPPMatrix
{
  CvMat* m_Matrix;

  static int get_type(const char&) { return CV_8SC1; }
  static int get_type(const int&) { return CV_32SC1; }
  static int get_type(const float&) { return CV_32FC1; }
  static int get_type(const double&) { return CV_64FC1; }
public:
  CPPMatrix(int w, int h)
    : m_Matrix(cvCreateMat(h,w,get_type(T())))
  {}

  ~CPPMatrix()
  {
    cvReleaseMat(&m_Matrix);
  }

  const CvMat* get() const { return m_Matrix; }
  CvMat* get() { return m_Matrix; }

  void fill(const T& value)
  {
    cvFillImage(get(),value);
  }

  void set(int x, int y, const T& value)
  {
    cvmSet(get(),y,x,value);
  }
};

class BackgroundSubtract
{
  CvFGDStatModelParams m_ParamFGD;
  CvBGStatModel*       m_BGModel;
public:
  BackgroundSubtract(Image first_frame)
  {
    m_ParamFGD.Lc = CV_BGFG_FGD_LC;
    m_ParamFGD.N1c = CV_BGFG_FGD_N1C;
    m_ParamFGD.N2c = CV_BGFG_FGD_N2C;
    m_ParamFGD.Lcc = CV_BGFG_FGD_LCC;
    m_ParamFGD.N1cc = CV_BGFG_FGD_N1CC;
    m_ParamFGD.N2cc = CV_BGFG_FGD_N2CC;
    m_ParamFGD.delta = CV_BGFG_FGD_DELTA;
    m_ParamFGD.alpha1 = CV_BGFG_FGD_ALPHA_1;
    m_ParamFGD.alpha2 = CV_BGFG_FGD_ALPHA_2;
    m_ParamFGD.alpha3 = CV_BGFG_FGD_ALPHA_3;
    m_ParamFGD.T = CV_BGFG_FGD_T;
    m_ParamFGD.minArea = CV_BGFG_FGD_MINAREA;
    m_ParamFGD.is_obj_without_holes = 1;
    m_ParamFGD.perform_morphing = 1;
    m_BGModel=cvCreateFGDStatModel(first_frame->get(),&m_ParamFGD);
  }

  ~BackgroundSubtract()
  {
    cvReleaseBGStatModel(&m_BGModel);
  }

  Image update(Image frame)
  {
    cvUpdateBGStatModel(frame->get(),m_BGModel);
    IplImage* fg=cvCloneImage(m_BGModel->foreground);
    return Image(new CPPImage(fg));
  }
};


inline Image odd_rows(Image image)
{
  unsigned w=image->get_width(),h=image->get_height(),ch=image->get_channels();
  Image target(new CPPImage(w,h/2,ch));
  for(unsigned y=1;y<h;y+=2)
  {
    const unsigned char* src=image->get_row(y);
    unsigned char* dst=target->get_row(y/2);
    std::copy(src,src+w*ch,dst);
  }
  return target;
}

inline void paste(Image target, Image source, int ox, int oy)
{
  int w=target->get_width(),h=target->get_height(),channels=target->get_channels();
  int sw=source->get_width(),sh=source->get_height(),schannels=source->get_channels();
  if (schannels!=channels) throw std::string("Channels mismatch in paste");
  if ((ox+sw)>w || (oy+sh)>h) throw std::string("Image doesn't fit in paste");
  int pitch=w*channels;
  for(int y=oy;y<(oy+sh);++y)
  {
    unsigned char* row=target->get_row(y,ox);
    const unsigned char* src=source->get_row(y-oy);
    std::copy(src,src+sw*channels,row);
  }
}

inline void and(Image a, Image b) // a&=b
{
  unsigned w=a->get_width(),h=a->get_height(),ch=a->get_channels();
  if (w!=b->get_width() || h!=b->get_height() || ch!=b->get_channels())
    throw std::string("and: Images mismatch");
  unsigned pitch=w*ch;
  for(unsigned y=0;y<h;++y)
  {
    byte* row=a->get_row(y);
    const byte* src=b->get_row(y);
    for(unsigned x=0;x<pitch;++x)
      row[x]&=src[x];
  }
}

inline Image or(Image a, Image b)
{
  unsigned w=a->get_width(),h=a->get_height(),ch=a->get_channels();
  if (w!=b->get_width() || h!=b->get_height() || ch!=b->get_channels())
    throw std::string("and: Images mismatch");
  Image c(new CPPImage(w,h,ch));
  cvOr(a->get(),b->get(),c->get());
  return c;
}

inline Image xor(Image a, Image b)
{
  unsigned w=a->get_width(),h=a->get_height(),ch=a->get_channels();
  if (w!=b->get_width() || h!=b->get_height() || ch!=b->get_channels())
    throw std::string("and: Images mismatch");
  Image c(new CPPImage(w,h,ch));
  cvXor(a->get(),b->get(),c->get());
  return c;
}

inline void fill(Image image, unsigned char value)
{
  int w=image->get_width(),h=image->get_height(),channels=image->get_channels();
  int pitch=w*channels;
  for(int y=0;y<h;++y)
  {
    unsigned char* row=image->get_row(y);
    std::fill_n(row,pitch,value);
  }
}

inline Image flip_vertical(Image image)
{
  int w=image->get_width(),h=image->get_height(),n=image->get_channels();
  int pitch=w*n;
  Image target(new CPPImage(w,h,n));
  for(int y=0;y<=h/2;++y)
  {
    int y2=h-y-1;
    if (y2==y) continue;
    const unsigned char* row1=image->get_row(y);
    const unsigned char* row2=image->get_row(y2);
    unsigned char* dst1=target->get_row(y);
    unsigned char* dst2=target->get_row(y2);
    for(int x=0;x<pitch;++x)
    {
      dst1[x]=row2[x];
      dst2[x]=row1[x];
    }
  }
  return target;
}

inline Image rotate_90(Image image, bool clockwise)
{
  int w=image->get_width(),h=image->get_height(),n=image->get_channels();
  Image target(new CPPImage(h,w,n));
  for(int y=0;y<h;++y)
  {
    const unsigned char* row=image->get_row(y);
    for(int x=0;x<w;++x)
    {
      unsigned char* dst=target->get_row(x,y);
      std::copy(row,row+n,dst);
      row+=n;
    }
  }
  return target;
}

inline Image median_filter(Image image)
{
  Image target(image->clone());
  cvSmooth(image->get(),target->get(),CV_MEDIAN);
  return target;
}

inline Image invert(Image image)
{
  Image target(image->clone());
  fill(target,255);
  return xor(target,image);
}

// Output is set to 255 is 3x3 sum is >= mass
inline Image mass_filter(Image image, int mass)
{
  unsigned w=image->get_width(),h=image->get_height(),ch=image->get_channels();
  if (ch!=1)
    throw std::string("mass_filter works on single channel images");
  Image target(new CPPImage(w,h,ch));
  fill(target,0);
  for(unsigned y=1;y<(h-1);++y)
  {
    const byte* rows[3];
    rows[0]=image->get_row(y-1);
    rows[1]=image->get_row(y);
    rows[2]=image->get_row(y+1);
    byte* dst=target->get_row(y);
    for(unsigned x=1;x<(w-1);++x)
    {
      int sum=int(rows[0][x-1])+int(rows[0][x])+int(rows[0][x+1])+
              int(rows[1][x-1])+int(rows[1][x])+int(rows[1][x+1])+
              int(rows[2][x-1])+int(rows[2][x])+int(rows[2][x+1]);
      dst[x]=(sum>mass?255:0);
    }
  }
  return target;
}

inline Image sobel(Image image, int xorder, int yorder, int apperture=3)
{
  Image target(image->clone());
  cvSobel(image->get(),target->get(),xorder,yorder,apperture);
  return target;
}

inline Image laplace(Image& image, int apperture=3)
{
	/*Image dst;
	CPPImage* img = ((CPPImage*)image.get());
	int kernel[] = {0,-1,0,
					-1,4,-1,
					0,-1,0};
	CvMat* filter = cvCreateMatHeader(3,3,CV_8UC1);
	cvSetData(filter,kernel,3*8);
	cvFilter2D(img->get(),img->get(),filter);
	return image;*/

	  CPPImage* img = ((CPPImage*)image.get());
  IplImage* im2 = cvCreateImage(cvGetSize(img->get()),
		IPL_DEPTH_32F, 1);
  cvLaplace(img->get(),im2,apperture);

	// //32 bit floating point image
	//IplImage *image2 = cvCreateImage(cvSize(im2->width,im2->height), IPL_DEPTH_8U, 1);

	////Convert image depth to image2 depth
	//cvConvertScale(im2,image2);

 // return Image(new CPPImage(image2));
  return Image(new CPPImage(im2));
	
}

inline Image scale_to(Image image, int w, int h, int interpolation=1)
{
  if (w<1) w=1;
  if (h<1) h=1;
  Image target(new CPPImage(w,h,image->get_channels()));
  cvResize(image->get(),target->get(),interpolation);
  return target;
}

inline Image erode(Image image, int iter=1)
{
  Image target(image->clone());
  cvErode(image->get(),target->get(),0,iter);
  return target;
}

inline Image erode(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvErode(image->get(),target->get(),kernel->get(),iter);
  return target;
}

inline Image dilate(Image image, int iter=1)
{
  Image target(image->clone());
  cvDilate(image->get(),target->get(),0,iter);
  return target;
}

inline Image dilate(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvDilate(image->get(),target->get(),kernel->get(),iter);
  return target;
}

inline Image morphology_open(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvMorphologyEx(image->get(),target->get(),0,kernel->get(),CV_MOP_OPEN,iter);
  return target;
}

inline Image morphology_close(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvMorphologyEx(image->get(),target->get(),0,kernel->get(),CV_MOP_CLOSE,iter);
  return target;
}

inline Image morphology_tophat(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvMorphologyEx(image->get(),target->get(),0,kernel->get(),CV_MOP_TOPHAT,iter);
  return target;
}

inline Image morphology_blackhat(Image image, Kernel kernel, int iter=1)
{
  Image target(image->clone());
  cvMorphologyEx(image->get(),target->get(),0,kernel->get(),CV_MOP_BLACKHAT,iter);
  return target;
}

inline Image smooth(const Image& image)
{
  Image target(image->clone());
  cvSmooth(image->get(),target->get(),CV_GAUSSIAN,7,7,5);
  return target;
}

template<class T>
inline Image filter(const Image& image, const CPPMatrix<T>& matrix)
{
  Image target(image->clone());
  cvFilter2D(image->get(),target->get(),matrix.get());
  return target;
}

inline Image threshold(const Image& image, double thres)
{
  Image target(image->clone());
  cvThreshold(image->get(),target->get(),thres,255,CV_THRESH_BINARY);
  return target;
}

inline Image convert_to_gray(const Image& image)
{
  if (image->get_channels()==1) return image;
  Image target(new CPPImage(image->get_width(),image->get_height(),1));
  cvCvtColor(image->get(),target->get(),CV_RGB2GRAY);
  return target;
}

inline Image convert_to_rgb(const Image& image)
{
  if (image->get_channels()==3) return image;
  Image target(new CPPImage(image->get_width(),image->get_height(),3));
  cvCvtColor(image->get(),target->get(),CV_GRAY2RGB);
  return target;
}

//written by Assaf and Hila
inline Image convert_bgr_to_hsi(const Image& image)
{
  Image target(new CPPImage(image->get_width(),image->get_height(),3));
  cvCvtColor(image->get(),target->get(),CV_BGR2HSV);
  return target;
}

inline void imageAdd(const Image& image1,const Image& image2,const Image& dst)
{
	cvAdd(((CPPImage*)image1.get())->get(),
		  ((CPPImage*)image2.get())->get(),
		  ((CPPImage*)dst.get())->get());
}
inline void imagePow(const Image& image,const Image& dst,double power)
{
	cvPow(((CPPImage*)image.get())->get(),((CPPImage*)dst.get())->get(),power);
}
inline double getVal(const Image& image,int i,int j)
{
	CvScalar& f = cvGet2D(((CPPImage*)image.get())->get(),i,j);
	return f.val[0];
}
inline void setVal(const Image& image,int i,int j,double val)
{
	return cvSet2D(((CPPImage*)image.get())->get(),i,j,cvScalar(val));
}

inline Image abs_diff(const Image& imga, const Image& imgb)
{
  Image target(imga->clone());
  cvAbsDiff(imga->get(),imgb->get(),target->get());
  return target;
}

inline Rect bounding_rect(const Image& img, byte bg=0)
{
  unsigned x,y,w=img->get_width(),h=img->get_height(),ch=img->get_channels();
  Rect r(w,h,0,0);
  for(y=0;y<h;++y)
  {
    const byte* row=img->get_row(y);
    for(x=0;x<w;++x)
    {
      if (row[x]!=bg) 
      {
        r.l=Min(r.l,x);
        r.t=Min(r.t,y);
        r.r=Max(r.r,x+1);
        r.b=Max(r.b,y+1);
      }
    }
  }
  return r;
}

inline Image crop(const Image& img, const Rect& r)
//IplImage *BasicOpenCV::Crop(IplImage *image,CvRect selection)
{
  IplImage* src=img->get();
  IplImage* dst = cvCreateImage(cvSize(r.width(), r.height()),src->depth,src->nChannels);
  dst->origin = src->origin;
  CvRect area=cvRect(r.l,r.t,r.width(),r.height());
  cvSetImageROI(src,area);
  cvCopy(src, dst);
  cvResetImageROI(src);
  return Image(new CPPImage(dst));
}

inline Image copy(const Image& img)
{
  return crop(img,Rect(0,0,img->get_width(),img->get_height()));
}

inline void scale_color_space(Image& image, int_pair& range)
{
  unsigned w=image->get_width(),h=image->get_height(),x,y;
  unsigned n=w*image->get_channels();
  if (range.second==range.first) return;
  double mult=256.0/(range.second-range.first);
  for(y=0;y<h;++y)
  {
    unsigned char* row=image->get_row(y);
    for(x=0;x<n;++x)
    {
      double v=row[x];
      v-=range.first;
      v*=mult;
      row[x]=byte(Max(0.0,Min(v,255.0)));
    }
  }
}

inline void scale_color_space(Image& image, double tail=0.01)
{
  Image gray=convert_to_gray(image);
  double hist[256];
  gray->calculate_histogram(hist);
  int_pair range;
  calculate_range(hist,range,(gray->get_width()*gray->get_height())*tail);
  scale_color_space(image,range);
}




} // namespace CPP

#endif // H_IMAGEPP