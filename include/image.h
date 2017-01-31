#ifndef H_IMAGEPP
#define H_IMAGEPP

#include <Windows.h>
#include <cv.hpp>

#include <cxcore.h>
#include <cvaux.h>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <string>
#include "prims.h"


typedef unsigned char byte;

namespace CPP {

using cv::Mat;
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

inline unsigned char* get_row(Mat* image, int row)
{
  return (unsigned char*)(image->data + row*image->step);
}

class CPPImage
{
  cv::Mat* m_Image;
  bool      m_Failed;

  static CvSize size(unsigned w, unsigned h)
  {
    CvSize sz;
    sz.width=w;
    sz.height=h;
    return sz;
  }
public:
  CPPImage(unsigned w, unsigned h, int channels,int depth = CV_8U)
    : m_Image(new Mat(size(w,h),depth,channels)),
      m_Failed(false)
  {}

  CPPImage(const char* filename)
    : m_Image(new Mat(cv::imread(filename))),
      m_Failed(false)
  {
    if (!m_Image) m_Failed=true;
  }

  CPPImage(const std::string& filename)
    : m_Image(new Mat(cv::imread(filename.c_str()))),
      m_Failed(false)
  {
    if (!m_Image) m_Failed=true;
  }

  CPPImage(Mat* img)
    : m_Image(new Mat(img->clone())),
      m_Failed(false)
  {}

  ~CPPImage(){}

  bool failed() const { return m_Failed; }

  CPPImage* clone() 
  { 
    CPPImage* img=new CPPImage(m_Image);
    return img;
  }

  Mat* get() { return m_Image; }

  void save(const std::string& filename)
  {
    cv::imwrite(filename.c_str(),*m_Image);
  }

  void save(const char* filename)
  {
	  cv::imwrite(filename,*m_Image);
  }

  unsigned get_pitch()
  {
    return m_Image->step;
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

  unsigned get_width()  const   { return m_Image->cols; }
  unsigned get_height() const   { return m_Image->rows; }
  unsigned get_channels() const { return m_Image->channels(); }
  Rect     get_rect() const     { return Rect(0,0, m_Image->cols, m_Image->rows); }

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
inline Image attach(Mat* image) { return Image(new CPPImage(image)); }

template<class T>
class CPPMatrix
{
  Mat* m_Matrix;

  static int get_type(const char&) { return CV_8U; }
  static int get_type(const int&) { return CV_32S; }
  static int get_type(const float&) { return CV_32F; }
  static int get_type(const double&) { return CV_64F; }
public:
  CPPMatrix(int w, int h)
    : m_Matrix(new Mat(h,w,get_type(T())))
  {}

  ~CPPMatrix(){}

  const Mat* get() const { return m_Matrix; }
  Mat* get() { return m_Matrix; }

  void fill(const T& value)
  {
	  *m_Matrix = value;
  }

  void set(int x, int y, const T& value)
  {
	m_Matrix->at<T>(x, y) = value;
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
	unsigned w = a->get_width(), h = a->get_height(), ch = a->get_channels();
	if (w != b->get_width() || h != b->get_height() || ch != b->get_channels())
		throw std::string("and: Images mismatch");
	Image c(new CPPImage(w, h, ch));
	cv::bitwise_and(*a->get(), *b->get(), *a->get());
}

inline Image or(Image a, Image b)
{
  unsigned w=a->get_width(),h=a->get_height(),ch=a->get_channels();
  if (w!=b->get_width() || h!=b->get_height() || ch!=b->get_channels())
    throw std::string("and: Images mismatch");
  Image c(new CPPImage(w,h,ch));
  cv::bitwise_or(*a->get(),*b->get(),*c->get());
  return c;
}

inline Image xor(Image a, Image b)
{
  unsigned w=a->get_width(),h=a->get_height(),ch=a->get_channels();
  if (w!=b->get_width() || h!=b->get_height() || ch!=b->get_channels())
    throw std::string("and: Images mismatch");
  Image c(new CPPImage(w,h,ch));
  cv::bitwise_xor(*a->get(), *b->get(), *c->get());
  return c;
}

inline void fill(Image image, unsigned char value)
{
	*image->get() = value;
}

inline Image flip_vertical(Image image)
{
  int w=image->get_width(),h=image->get_height(),n=image->get_channels();
  Image target(new CPPImage(w,h,n));
  flip(*image->get(), *target->get(), 0);
  return target;
}

inline Image rotate_90(Image image, bool clockwise)
{
	int w = image->get_width(), h = image->get_height(), n = image->get_channels();
	Image target(new CPPImage(h, w, n));
	flip(*image->get(), *target->get(), 1);
  return target;
}

inline Image median_filter(Image image)
{
  Image target(image->clone());
  cv::medianBlur(*image->get(),*target->get(),3);
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
  cv::Sobel(*image->get(),*target->get(), CV_8U, xorder,yorder,apperture);
  return target;
}

inline Image laplace(Image& image, int apperture=1)
{
	CPPImage* img = ((CPPImage*)image.get());
	Mat* im2 = new Mat(cvGetSize(img->get()),
		CV_8U, 1);
	cv::Laplacian(*img->get(),*im2,CV_8U,apperture);
	return Image(new CPPImage(im2));
}

inline Image scale_to(Image image, int w, int h, int interpolation=1)
{
  if (w<1) w=1;
  if (h<1) h=1;
  Image target(new CPPImage(w,h,image->get_channels()));
  cv::resize(*image->get(),*target->get(),cv::Size(w,h),0,0,interpolation);
  return target;
}

inline Image dilate(Image image, int iter=1)
{
  Image target(image->clone());
  Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4));
  cv::dilate(*image->get(),*target->get(),element,cv::Point(-1,-1),iter);
  return target;
}

inline Image CannyWithBiliteral(const Image& image, int channels)
{
	Image target(image->clone());
	cv::bilateralFilter(*image->get(), *target->get(), 5, 80, 80);
	//cv::namedWindow("Image:", CV_WINDOW_AUTOSIZE);
	if (channels == 1) 
	{
		//CFrameManager::displayImage(target);
		//cv::imshow("Image:", *target->get());
		//cv::waitKey(0);
		cv::Canny(*target->get(), *target->get(), 30, 90);
	}
	if (channels == 3) {
		Mat bgr[3];					//destination array
		cv::split(*target->get(), bgr);			//split source  

		cv::Canny(bgr[0], bgr[0], int(255/3), 255);
		cv::Canny(bgr[1], bgr[1], int(255/3), 255);
		cv::Canny(bgr[2], bgr[2], int(255/3), 255);

		Mat mergedImage;
		bitwise_or(bgr[0], bgr[1], mergedImage);
		bitwise_or(mergedImage, bgr[2], mergedImage);
		*target->get() = mergedImage;
	//CFrameManager::displayImage(target);
	//cv::imshow("Image:", *target->get());
	//cv::waitKey(0);
	// Wait for the user to press a key in the GUI window.
	}
	
	return target;
}
template<class T>
inline Image filter(const Image& image, const CPPMatrix<T>& matrix)
{
  Image target(image->clone());
  cv::filter2D(*image->get(), *target->get(),image->get()->depth(),*matrix.get());
  return target;
}

inline Image threshold(const Image& image, double thres)
{
  Image target(image->clone());
  cv::threshold(*image->get(),*target->get(),thres,255, cv::THRESH_BINARY);
  return target;
}

inline Image convert_to_gray(const Image& image)
{
  if (image->get_channels()==1) return image;
  Image target(new CPPImage(image->get_width(),image->get_height(),1));
  cv::cvtColor(*image->get(),*target->get(),CV_RGB2GRAY);
  return target;
}

inline Image convert_to_rgb(const Image& image)
{
  if (image->get_channels()==3) return image;
  Image target(new CPPImage(image->get_width(),image->get_height(),3));
  cv::cvtColor(*image->get(),*target->get(),CV_GRAY2RGB);
  return target;
}

//written by Assaf and Hila
inline Image convert_bgr_to_hsi(const Image& image)
{
  Image target(new CPPImage(image->get_width(),image->get_height(),3));
  cv::cvtColor(*image->get(), *target->get(),CV_BGR2HSV);
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
	cv::Scalar S(((CPPImage*)image.get())->get()->at<uchar>(j, i));
	return S.val[0];
}
inline void setVal(const Image& image,int i,int j,double val)
{
	((CPPImage*)image.get())->get()->at<uchar>(i, j) = val;
}

inline Image abs_diff(const Image& imga, const Image& imgb)
{
  Image target(imga->clone());
  cv::absdiff(*imga->get(),*imgb->get(),*target->get());
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

inline Image crop(const Image& img, const cv::Rect& r)
{
  Mat src=*img->get();
  Mat* dst = new Mat(src(r).clone());
  return Image(new CPPImage(dst));
}

inline Image copy(const Image& img)
{
  return crop(img,cv::Rect(0,0,img->get_width(),img->get_height()));
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