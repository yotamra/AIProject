#include "moments.h"
#include <stdlib.h>


Vec2D  calculate_centroid(Image img)
{
  Vec2D sum(0,0);
  int   n=0;
  if (img == NULL)
  {
	  int i =5;
  }
  unsigned w=img->get_width(),h=img->get_height();
  for(unsigned y=0;y<h;++y)
  {
    byte* row=img->get_row(y);
    for(unsigned x=0;x<w;++x)
    {
      if (row[x])
      {
        sum.x+=x;
        sum.y+=y;
        ++n;
      }
    }
  }
  if (n==0) return sum;
  return (1.0/n)*sum;
}


double calculate_moment(Image img, const Vec2D& centroid, int p, int q)
{
  double sum=0;
  unsigned w=img->get_width(),h=img->get_height();
  for(unsigned y=0;y<h;++y)
  {
    byte* row=img->get_row(y);
    for(unsigned x=0;x<w;++x)
    {
      if (row[x])
      {
        sum+=pow(x-centroid.x,p)*pow(y-centroid.y,q);
      }
    }
  }
  return sum;
}
