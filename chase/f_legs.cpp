#include "feature_manager.h"
#include <statistics.h>

using namespace CPP;

typedef std::vector<double> dvec;

/*
Calcs a vector in the size of the number of rows in the object. Each cell represents the number of runs in its row.
A run is a sequence of white pixels with black pixels on both sides.
*/
FEATURE(Legs)
{
  Image img=pimg.get_image();
  int n=0;
  unsigned x,y,w=img->get_width(),h=img->get_height();
  bool last_row=false;
  int seq_len=0,best_len=0;
  dvec runs;
  for(y=0;y<h;++y)
  {
    const byte* row=img->get_row(y);
    int m=0;
    bool last=false;
    for(x=0;x<w;++x)
    {
      bool cur=(row[x]!=0);
      if (!cur && last) ++m;
      last=cur;
    }
    if (last) ++m;
    runs.push_back(m);
    if (m==2)
    {
      if (last_row)
      {
        if (++seq_len > best_len)
          best_len=seq_len;
      }
      last_row=true;
    }
    else 
    {
      last_row=false;
      seq_len=0;
    }
  }
  n=runs.size();
  double m1=calculate_median(runs.begin(),runs.begin()+n/3);
  double m2=calculate_median(runs.begin()+(n*2)/3,runs.end());
  double mx=*std::max_element(runs.begin(),runs.end());
  std::ostringstream os;
  os << m1 << " " << m2 << " " << mx;
  return os.str();
}
