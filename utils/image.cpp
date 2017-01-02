#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <string>
#include <prims.h>
#include <xml.h>
#include <image.h>
#include <imageutils.h>

using namespace CPP;

/*
This class has some functions for images manipulations.
*/

// Convert a single RGB pixel to HSV
// Algorithm described at:  http://en.wikipedia.org/wiki/HSL_and_HSV
void rgb2hsv(const byte* color, double hsv[3])
{
  double rgb[3];
  rgb[0]=color[0]/256.0;
  rgb[1]=color[1]/256.0;
  rgb[2]=color[2]/256.0;
  double max_rgb=Max(rgb[0],Max(rgb[1],rgb[2]));
  double min_rgb=Min(rgb[0],Min(rgb[1],rgb[2]));
  {// hue
    double minmax=max_rgb-min_rgb;
    const double sixth=1.0/6.0;
    if (minmax==0) hsv[0]=0;
    else
    if (max_rgb==rgb[0])  
    { 
      hsv[0]=1.0+sixth*(rgb[1]-rgb[2])/minmax; 
      hsv[0]-=double(floor(hsv[0])); 
    }
    else
    if (max_rgb==rgb[1])  { hsv[0]=2*sixth+sixth*(rgb[2]-rgb[0])/minmax; }
    else
    if (max_rgb==rgb[2])  { hsv[0]=4*sixth+sixth*(rgb[0]-rgb[1])/minmax; }
  }
  {// sat
    if (max_rgb==0) hsv[1]=0;
    else hsv[1]=1.0-min_rgb/max_rgb;
  }
  hsv[2]=max_rgb;
}

void rgb2xyz(const byte* color, double* xyz)
{
  double rgb[3];
  rgb[0]=color[0]/256.0;
  rgb[1]=color[1]/256.0;
  rgb[2]=color[2]/256.0;

  if ( rgb[0] > 0.04045 ) rgb[0] = pow( ( ( rgb[0] + 0.055 ) / 1.055 ) , 2.4);
  else                    rgb[0] = rgb[0] / 12.92;
  if ( rgb[1] > 0.04045 ) rgb[1] = pow( ( ( rgb[1] + 0.055 ) / 1.055 ) , 2.4);
  else                    rgb[1] = rgb[1] / 12.92;
  if ( rgb[2] > 0.04045 ) rgb[2] = pow( ( ( rgb[2] + 0.055 ) / 1.055 ) , 2.4);
  else                    rgb[2] = rgb[2] / 12.92;

  rgb[0] = rgb[0] * 100.0;
  rgb[1] = rgb[1] * 100.0;
  rgb[2] = rgb[2] * 100.0;

  //Observer. = 2°, Illuminant = D65
  xyz[0] = rgb[0] * 0.4124 + rgb[1] * 0.3576 + rgb[2] * 0.1805;
  xyz[1] = rgb[0] * 0.2126 + rgb[1] * 0.7152 + rgb[2] * 0.0722;
  xyz[2] = rgb[0] * 0.0193 + rgb[1] * 0.1192 + rgb[2] * 0.9505;
}

void xyz2lab(double* xyz, double lab[3])
{
  const double ref[] = { 95.047, 100.0, 108.883 };
  double tmp[3];
  for(int i=0;i<3;++i)
  {
    tmp[i]=xyz[i]/ref[i];
    if (tmp[i]>0.008856) tmp[i]=pow(tmp[i],1.0/3.0);
    else                 tmp[i]=(7.787*tmp[i])+(16.0/116.0);
  }
  lab[0] = ( 116.0 * tmp[1] ) - 16.0;
  lab[1] = 500.0 * ( tmp[0] - tmp[1] );
  lab[2] = 200.0 * ( tmp[1] - tmp[2] );
}

void rgb2Lab(const byte* color, double Lab[3])
{
  double xyz[3];
  rgb2xyz(color,xyz);
  xyz2lab(xyz,Lab);
}


double get_mean_intensity(Image image)
{
  if (image->get_channels()==3)
  {
    Image gray=convert_to_gray(image);
    return get_mean_intensity(gray);
  }
  unsigned w=image->get_width(),h=image->get_height();
  unsigned sum=0;
  for(unsigned y=0;y<h;++y)
  {
    const byte* row=image->get_row(y);
    for(unsigned x=0;x<w;++x)
    {
      sum+=row[x];
    }
  }
  return sum/double(w*h);
}

void create_histogram(Image image, int hist[256])
{
  if (image->get_channels()==3) image=convert_to_gray(image);
  std::fill_n(hist,256,0);
  unsigned w=image->get_width(),h=image->get_height();
  unsigned sum=0;
  for(unsigned y=0;y<h;++y)
  {
    const byte* row=image->get_row(y);
    for(unsigned x=0;x<w;++x)
    {
      hist[row[x]]++;
    }
  }
}
