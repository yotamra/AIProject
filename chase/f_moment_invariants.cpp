#include "feature_manager.h"
#include "moments.h"

#define MOMENT(p,q) double m##p##q = calculate_moment(pimg.get_image(),cntr,p,q)
#define NI(p,q) double ni##p##q = m##p##q/pow(m00,(1+0.5*(p+q)))

FEATURE(Invar)
{
  Vec2D cntr=calculate_centroid(pimg.get_image());
  MOMENT(0,0);
  MOMENT(0,1);
  MOMENT(1,0);
  MOMENT(1,1);
  MOMENT(0,2);
  MOMENT(2,0);
  MOMENT(2,1);
  MOMENT(1,2);
  MOMENT(0,3);
  MOMENT(3,0);
  NI(1,1);
  NI(0,2);
  NI(2,0);
  NI(1,2);
  NI(2,1);
  NI(0,3);
  NI(3,0);
  double I1=ni20+ni02;
  double I2=sqr(ni20-ni02)+sqr(2*ni11);
  double I3=sqr(ni30-3*ni12)+sqr(3*ni21-ni03);
  double I4=sqr(ni30+ni12)+sqr(ni21+ni03);
  double I5=(ni30-3*ni12)*(ni30+ni12)*(sqr(ni30+ni12)-3*sqr(ni21+ni03)) +
            (3*ni21-ni03)*(ni21+ni03)*(3*sqr(ni30+ni12)-sqr(ni21+ni03));
  double I6=(ni20-ni02)*(sqr(ni30+ni12)-sqr(ni21+ni03))+4*ni11*(ni30+ni12)*(ni21+ni03);
  double I7=(3*ni21-ni03)*(ni30+ni12)*(sqr(ni30+ni12)-3*sqr(ni21+ni03)) + 
            (ni30-3*ni12)*(ni21+ni03)*(3*sqr(ni30+ni12)-sqr(ni21+ni03));
  std::ostringstream os;
  os << I1 << ',' << I2 << ',' << I3 << ',' << I4 << ','
     << I5 << ',' << I6 << ',' << I7;
  return os.str();
}
