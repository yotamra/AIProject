#include "feature_manager.h"
#include "moments.h"

#define MOMENT(p,q) double m##p##q = calculate_moment(pimg.get_image(),cntr,p,q)

double perimeter(Image img)
{
  double p=0;
  unsigned w=img->get_width(),h=img->get_height();
  for(unsigned y=0;y<h;++y)
  {
    byte* row=img->get_row(y);
    if (y==0 || y==(h-1)) p+=std::count(row,row+w,255);
    else
    {
      byte* prow=img->get_row(y-1);
      byte* nrow=img->get_row(y+1);
      for(unsigned x=0;x<w;++x)
      {
        if (row[x])
        {
          if (x==0 || x==(w-1)) p+=1;
          else
          {
            if (row[x-1]==0 || row[x+1]==0 || prow[x]==0 || nrow[x]==0)
              p+=1;
          }
        }
      }
    }
  }
  return p;
}

FEATURE(SPPA)
{
  Vec2D cntr=calculate_centroid(pimg.get_image());
  MOMENT(0,0);
  std::ostringstream os;
  os << sqr(perimeter(pimg.get_image()))/m00;
  return os.str();
}