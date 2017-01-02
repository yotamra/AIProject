#ifndef H_PRIMS
#define H_PRIMS

#include <iostream>
#include <vector>
#include <cmath>

const double PI = 3.1415926535897932384626433832795;

template<class T>
inline const T& Max(const T& a, const T& b)
{
  return a>b?a:b;
}

template<class T>
inline const T& Min(const T& a, const T& b)
{
  return a<b?a:b;
}

//template<class T>
//class min_func : public std::binary_function<T,T,T>
//{
//public:
//  const T& operator() (const T& a, const T& b) const
//  {
//    return Min(a,b);
//  }
//};

//template<class T>
//class max_func : public std::binary_function<T,T,T>
//{
//public:
//  const T& operator() (const T& a, const T& b) const
//  {
//    return Max(a,b);
//  }
//};



template<class T>
inline T udiff(const T& a, const T& b)
{
  return a<b?b-a:a-b;
}


struct Vec2D
{
  Vec2D() : x(0), y(0) {}
  Vec2D(double _x, double _y) : x(_x), y(_y) {}
  double x,y;
  Vec2D& operator-=(const Vec2D& rhs) { x-=rhs.x; y-=rhs.y; return *this; }
  Vec2D& operator+=(const Vec2D& rhs) { x+=rhs.x; y+=rhs.y; return *this; }
  Vec2D& operator*=(double d) { x*=d; y*=d; return *this; }
  Vec2D& operator/=(double d) { x/=d; y/=d; return *this; }
  double square_magnitude() const { return x*x+y*y; }
  double magnitude() const { return sqrt(square_magnitude()); }
  Vec2D& normalized() { return (*this)/=magnitude(); }
};

inline Vec2D operator- (Vec2D a, const Vec2D& b)
{
  return a-=b;
}

inline Vec2D operator+ (Vec2D a, const Vec2D& b)
{
  return a+=b;
}

inline double operator* (const Vec2D& a, const Vec2D& b)
{
  return a.x*b.x+a.y*b.y;
}

inline Vec2D operator* (const Vec2D& a, double b)
{
  return Vec2D(a.x*b,a.y*b);
}

inline Vec2D operator* (double b, const Vec2D& a)
{
  return Vec2D(a.x*b,a.y*b);
}

struct sPoint
{
  unsigned short x,y;
  sPoint(unsigned short X=0, unsigned short Y=0) : x(X), y(Y) {}
  bool operator== (const sPoint& rhs) { return x==rhs.x && y==rhs.y; }
  bool operator!= (const sPoint& rhs) { return !(*this==rhs); }
};

struct iPoint
{
  int x,y;
  iPoint(int X=0, int Y=0) : x(X), y(Y) {}
};

struct Rect
{
  unsigned l,t,r,b;
  Rect(unsigned L=0, unsigned T=0, unsigned R=0, unsigned B=0) : l(L), t(T), r(R), b(B) {}
  void unite(unsigned x, unsigned y)
  {
    if (x<l) l=x;
    if (x>=r) r=x+1;
    if (y<t) t=y;
    if (y>=b) b=y+1;
  }
  void unite(const Rect& rhs)
  {
    if (rhs.l<l) l=rhs.l;
    if (rhs.r>r) r=rhs.r;
    if (rhs.t<t) t=rhs.t;
    if (rhs.b>b) b=rhs.b;
  }
  bool is_point_inside(unsigned x, unsigned y) const
  {
    return (x>=l && x<r && y>=t && y<b);
  }
  Vec2D get_center() const { return Vec2D(0.5f*(l+r),0.5f*(t+b)); }
  int width() const { return r-l; }
  int height() const { return b-t; }
  int get_area() const { return width()*height(); }
  double aspect_ratio() const { return double(width())/double(height()); }
  bool valid() const { return (r>l && b>t); }

  void offset(int x, int y)
  {
    l+=x; r+=x;
    t+=y; b+=y;
  }

  Rect& intersect(const Rect& rhs)
  {
    l=Max(l,rhs.l);
    t=Max(t,rhs.t);
    r=Min(r,rhs.r);
    b=Min(b,rhs.b);
    return *this;
  }

  bool contains(const Rect& rhs) const
  {
    return (rhs.l>=l && rhs.t>=t && rhs.r<=r && rhs.b<=b);
  }
};

inline std::ostream& operator<< (std::ostream& os, const Rect& r)
{
  return os << r.l << ',' << r.t << ',' << r.r << ',' << r.b;
}

template<class T>
inline T sqr(const T& t) { return t*t; }

class WindowAverage
{
  float m_Value;
  float m_Alpha;
public:
  WindowAverage(float alpha=0.6) : m_Value(0), m_Alpha(alpha) {}
  void set_value(float v) { m_Value=v; }
  void set_alpha(float a) { m_Alpha=a; }
  float update(float v) 
  { 
    m_Value=(v*m_Alpha)+m_Value*(1-m_Alpha);
    return m_Value;
  }
  float get() const { return m_Value; }
};

template<class T>
class Matrix
{
  unsigned m_Width,m_Height;
  typedef std::vector<T> data_vec;
  data_vec m_Data;
public:
  Matrix() : m_Width(0), m_Height(0) {}
  Matrix(unsigned w, unsigned h, const T& init=0) 
    : m_Width(w),
      m_Height(h),
      m_Data(w*h,init)
  {}

  void resize(unsigned w, unsigned h, const T& init=0)
  {
    m_Width=w;
    m_Height=h;
    m_Data.resize(w*h,init);
  }

  T& operator() (unsigned x, unsigned y) 
  { 
    if (x>=m_Width || y>=m_Height) throw 1; 
    return m_Data[y*m_Width+x];
  }

  const T& operator() (unsigned x, unsigned y) const
  { 
    if (x>=m_Width || y>=m_Height) throw 1; 
    return m_Data[y*m_Width+x];
  }

  unsigned get_width() const { return m_Width; }
  unsigned get_height() const { return m_Height; }

  T* get_row(unsigned row) { return &m_Data[row*m_Width]; }

  typedef typename data_vec::const_iterator const_iterator;
  const_iterator begin() const { return m_Data.begin(); }
  const_iterator end() const { return m_Data.end(); }

  void scale(double scalar)
  {
    data_vec::iterator b=m_Data.begin(),e=m_Data.end();
    for(;b!=e;++b)
      *b = T(*b * scalar);
  }

  void normalize(const T& to_max)
  {
    data_vec::iterator b=m_Data.begin(),e=m_Data.end();
    T max_value=*std::max_element(b,e);
    for(;b!=e;++b)
    {
      *b = (*b * to_max / max_value);
    }
  }
};


#endif // H_PRIMS