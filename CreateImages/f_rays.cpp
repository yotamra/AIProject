#include "feature_manager.h"
#include "moments.h"

using namespace CPP;

#define MOMENT(p,q) double m##p##q = calculate_moment(pimg.get_image(),cntr,p,q)
#define NI(p,q) double ni##p##q = m##p##q/pow(m00,(1+0.5*(p+q)))

typedef std::vector<double> dvec;

/*
Calcs a vector where each cell represents the size of the vector between the center of mass of an object to its border.
The size of all the vectors is normallized to maximum 1 so that the distance of the person from the camera wouldn't matter.
*/
FEATURE(Rays)
{
  Image img=pimg.get_image();
  Vec2D cntr=calculate_centroid(img);
  const int N=60;
  dvec dists(N,0.0);
  unsigned x,y,w=img->get_width(),h=img->get_height();
  for(y=0;y<h;++y)
  {
    const byte* row=img->get_row(y);
    for(x=0;x<w;++x)
    {
      if (row[x])
      {
        double dx=(x-cntr.x);
        double dy=(y-cntr.y);
        double angle=atan2(dy,dx);
        int index=int((angle+PI)*(0.5*N/PI));
        if (index==N) --index;
        double dist=sqr(dx)+sqr(dy);
        dists[index]=Max(dists[index],dist);
      }
    }
  }
  double sum=0;
  for(int i=0;i<N;++i)
  {
    sum+=dists[i];
    dists[i]=sqrt(dists[i]);
  }
  double mult=1.0/sqrt(sum);
  std::ostringstream os;
  for(int i=0;i<N;++i)
  {
    if (i>0) os << ',';
    os << (mult*dists[i]);
  }
  return os.str();
}
