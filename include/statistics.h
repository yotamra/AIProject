#ifndef statistics_h__
#define statistics_h__

#include <algorithm>
#include <numeric>

template<class II, class OI>
void calculate_difference(II b, II e, OI o)
{
  double prev=*b;
  while(++b!=e)
    *o++ = (*b - prev);
}

template<class II>
double calculate_median(II b, II e)
{
  int n=std::distance(b,e);
  std::sort(b,e);
  return *(b+n/2);
}

template<class II>
double calculate_average(II b, II e)
{
  return std::accumulate(b,e,0.0)/std::distance(b,e);
}

template<class II>
double calculate_variance(II b, II e)
{
  int n=std::distance(b,e);
  if (n<=0) return 0;
  double sum=std::accumulate(b,e,0.0);
  double avg=sum/n;
  sum=0;
  for(;b!=e;++b)
  {
    sum+=sqr(*b - avg);
  }
  return sum/n;
}

template<class II>
double calculate_variance_coefficient(II b, II e)
{
  if (b==e) return 0.0;
  double var=calculate_variance(b,e);
  double avg=calculate_average(b,e);
  return var/avg;
}

#endif // statistics_h__
