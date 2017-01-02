#ifndef H_HSV
#define H_HSV

typedef unsigned char uchar;

inline void findMinMaxNum(double x, double y, double z, double& min, double& max)
{
	if (x > y )
	{
		   if (x > z)
		   {
				   max = x;
				   if (y > z)
				   {
						   min = z;
				   }
				   else
				   {
						   min = y;
				   }
		   }
		   else
		   {
				   max = z;
				   min = y;
		   }
	}
	else if (y > z)
	{
		   max = y;
		   if (x > z)
		   {
				   min = z;
		   }
		   else
		   {
				   min = x;
		   }
	}
	else
	{
		   max = z;
		   min = x;
	}
}

struct rgb
{
  uchar r,g,b;

  rgb(const uchar* ptr) : r(ptr[0]), g(ptr[1]), b(ptr[2]) {}
  rgb(uchar red=0, uchar green=0, uchar blue=0) : r(red), g(green), b(blue) {}
  rgb(const struct hsv& v);

  void copy(uchar* ptr) const { ptr[0]=r; ptr[1]=g; ptr[2]=b; }

  void set(const uchar* ptr)
  {
	  r = ptr[0];
	  g = ptr[1];
	  b = ptr[2];
  }
};

struct hsv
{
  double h,s,v;

  hsv(double hue=0, double sat=0, double value=0) : h(hue), s(sat), v(value) {}
  hsv(const rgb& c)
  {
    double r=c.r/255.0;
    double g=c.g/255.0;
    double b=c.b/255.0;
    double max_rgb=Max(r,Max(g,b));
    double min_rgb=Min(r,Min(g,b));
    {// hue
      double minmax=max_rgb-min_rgb;
      const double sixth=1.0/6.0;
      if (minmax==0) h=0;
      else
      if (max_rgb==r)
      { 
        h=1.0+sixth*(g-b)/minmax; 
        h-=floor(h);
      }
      else
      if (max_rgb==g)  { h=2*sixth+sixth*(b-r)/minmax; }
      else
      if (max_rgb==b)  { h=4*sixth+sixth*(r-g)/minmax; }
    }
    {// sat
      if (max_rgb==0) s=0;
      else s=1.0-min_rgb/max_rgb;
    }
    v=max_rgb;
  }
};
struct hsl
{
  double h,sx,sy,l;

  hsl(double hue=0, double Sx=0,double Sy=0, double light=0) : h(hue), sy(Sy), sx(Sx), l(light) {}
  hsl(const rgb& c)
  {
    double r=c.r/255.0;
    double g=c.g/255.0;
    double b=c.b/255.0;
	double max_rgb;
    double min_rgb;
	findMinMaxNum(r, g, b, min_rgb, max_rgb);
    {// hue
      double minmax=max_rgb-min_rgb;
      const double sixth=1.0/6.0;
      if (minmax==0) h=0;
      else
        if (max_rgb==r)
        { 
          h = ((g-b)/minmax) + (g < b ? 6 : 0);
         // h-=floor(h);
        }
        else
          if (max_rgb==g)  { h=2+(b-r)/minmax; }
          else
            if (max_rgb==b)  { h=4+(r-g)/minmax; }

		h = h/6;
    }
	h*=2*PI;
    {// sat
		double s;
      if (max_rgb==0) s=0;
      else s=max_rgb-min_rgb;

	  sx = s*cos(h);
	  sy = s*sin(h);
    }
    l=(r+g+b)/3.0;
  }

  void set(const rgb& c)
  {
    double r=c.r/255.0;
    double g=c.g/255.0;
    double b=c.b/255.0;
	double max_rgb;
    double min_rgb;
	findMinMaxNum(r, g, b, min_rgb, max_rgb);
    //double max_rgb=max(r,max(g,b));
    //double min_rgb=min(r,min(g,b));
    {// hue
      double minmax=max_rgb-min_rgb;
      const double sixth=1.0/6.0;
      if (minmax==0) h=0;
      else
        if (max_rgb==r)
        { 
          h=1.0+sixth*(g-b)/minmax; 
          h-=floor(h);
        }
        else
          if (max_rgb==g)  { h=2*sixth+sixth*(b-r)/minmax; }
          else
            if (max_rgb==b)  { h=4*sixth+sixth*(r-g)/minmax; }
    }
	h*=2*PI;
    {// sat
		double s;
      if (max_rgb==0) s=0;
      else s=max_rgb-min_rgb;
	  
	  sx = s*cos(h);
	  sy = s*sin(h);
    }
    l=(r+g+b)/3.0;
  }
};

//struct hsl
//{
//  double h,s,l;
//
//  hsl(double hue=0, double sat=0, double light=0) : h(hue), s(sat), l(light) {}
//  hsl(const rgb& c)
//  {
//    double r=c.r/255.0;
//    double g=c.g/255.0;
//    double b=c.b/255.0;
//	double max_rgb;
//    double min_rgb;
//	findMinMaxNum(r, g, b, min_rgb, max_rgb);
//    //double max_rgb=max(r,max(g,b));
//    //double min_rgb=min(r,min(g,b));
//    {// hue
//      double minmax=max_rgb-min_rgb;
//      const double sixth=1.0/6.0;
//      if (minmax==0) h=0;
//      else
//        if (max_rgb==r)
//        { 
//          h=1.0+sixth*(g-b)/minmax; 
//          h-=floor(h);
//        }
//        else
//          if (max_rgb==g)  { h=2*sixth+sixth*(b-r)/minmax; }
//          else
//            if (max_rgb==b)  { h=4*sixth+sixth*(r-g)/minmax; }
//    }
//	h*=2*PI;
//    {// sat
//      if (max_rgb==0) s=0;
//      else s=max_rgb-min_rgb;
//    }
//    l=(r+g+b)/3.0;
//  }
//
//  void set(const rgb& c)
//  {
//    double r=c.r/255.0;
//    double g=c.g/255.0;
//    double b=c.b/255.0;
//	double max_rgb;
//    double min_rgb;
//	findMinMaxNum(r, g, b, min_rgb, max_rgb);
//    //double max_rgb=max(r,max(g,b));
//    //double min_rgb=min(r,min(g,b));
//    {// hue
//      double minmax=max_rgb-min_rgb;
//      const double sixth=1.0/6.0;
//      if (minmax==0) h=0;
//      else
//        if (max_rgb==r)
//        { 
//          h=1.0+sixth*(g-b)/minmax; 
//          h-=floor(h);
//        }
//        else
//          if (max_rgb==g)  { h=2*sixth+sixth*(b-r)/minmax; }
//          else
//            if (max_rgb==b)  { h=4*sixth+sixth*(r-g)/minmax; }
//    }
//	h*=2*PI;
//    {// sat
//      if (max_rgb==0) s=0;
//      else s=max_rgb-min_rgb;
//    }
//    l=(r+g+b)/3.0;
//  }
//};


inline uchar m256(double d)
{
  int i=int(d*255.0);
  return uchar(i&255);
}

inline rgb::rgb(const hsv& v)
{
  int hi=int(floor(v.h*6.0));
  double f=v.h-floor(v.h);
  double p=v.v*(1-v.s);
  double q=v.v*(1-f*v.s);
  double t=v.v*(1-(1-f)*v.s);
  switch (hi)
  {
  case 0: r=m256(v.v); g=m256(t); b=m256(p); break;
  case 1: r=m256(q); g=m256(v.v); b=m256(p); break;
  case 2: r=m256(p); g=m256(v.v); b=m256(t); break;
  case 3: r=m256(p); g=m256(q); b=m256(v.v); break;
  case 4: r=m256(t); g=m256(p); b=m256(v.v); break;
  case 5: r=m256(v.v); g=m256(p); b=m256(q); break;
  default: r=g=b=0; 
  }
}

#endif // H_HSV
