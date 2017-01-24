#ifndef H_CC
#define H_CC

#include <algorithm>
#include <list>
#include <math.h>
#include <image.h>

using namespace std;

typedef unsigned short ushort;
typedef std::pair<ushort,ushort> pixel_coords;
typedef std::vector<pixel_coords> coords_vec;

// Connected component to be used in the connected components search algorithm
struct cc
{
 cc() : leader(0), pixels(0), cgx(0), cgy(0),group_id(-1), score(-1) {}
  cc(int currGroupId) : leader(0), pixels(0), cgx(0), cgy(0),group_id(currGroupId) {}
  cc*   leader;  // Internally used.  Leader for the equivalence class
  unsigned   pixels;  // number of pixels
  float cgx,cgy; // Component's center of gravity
  Rect  rect;    // Bounding rectangle
  coords_vec coords;
  int group_id; //added by Assaf and Hila
  int cc_id; //added by Assaf and Hila
  double score; //added by Assaf and Hila
  void follow()
  {
    while (leader && leader->leader)
      leader=leader->leader;
  }
 
  cc* getLeader()
  {
	  follow();
	  if (!leader)
	  {
		  return this;
	  }
	  return leader;
  }
  // Add a new pixel at position
  void add(ushort x, ushort y, bool save_coords)
  {
    follow();
    if (leader) leader->add(x,y,save_coords);
    else
    {
      //cgx=(cgx*pixels+x)/(pixels+1.0);
      //cgy=(cgy*pixels+y)/(pixels+1.0);
      pixels++;
      if (rect.r==0) 
      {
        rect.l=x;
        rect.t=y;
        rect.r=x+1;
        rect.b=y+1;
      }
      else rect.unite(x,y);
      if (save_coords)
        coords.push_back(std::make_pair(x,y));
    }
  }

  // Unite 2 cc's when a pixel bridges between them
  bool unite(cc& c)
  {
    if (&c == this) return true;
    follow();
    if (leader) return leader->unite(c);
    if (c.leader) return unite(*c.leader);
    if (c.pixels > pixels) return c.unite(*this);
    //cgx=(cgx*pixels+c.cgx*c.pixels)/(pixels+c.pixels);
    //cgy=(cgy*pixels+c.cgy*c.pixels)/(pixels+c.pixels);
    pixels+=c.pixels;
    rect.unite(c.rect);
    c.leader=this;
	c.group_id = group_id; //added by Assaf and Hila
    coords.insert(coords.end(),c.coords.begin(),c.coords.end());
    return true;
  }

  bool has_leader() const { return leader!=0; }

  CPP::Image build_image() const
  {
    CPP::Image image(new CPP::CPPImage(rect.width(),rect.height(),1));
    CPP::fill(image,0);
    coords_vec::const_iterator b=coords.begin(),e=coords.end();
    for(;b!=e;++b)
    {
      const pixel_coords& p=*b;
      unsigned y=p.second-rect.t,x=p.first-rect.l;
      image->get_row(y)[x]=255;
    }
    return image;
  }

  struct config
  {
    config(bool save = false) 
      : min_x(0), min_y(0), max_x(9999), max_y(9999),
        collect_images(save)
    {}
    unsigned min_x,max_x;
    unsigned min_y,max_y;
    bool     collect_images;
  };

  typedef std::list<cc> cc_list;
  
  // Main scanning algorithm.   Builds equivalence classes for components
  static void analyze(const CPP::Image* image, cc_list* l, const config& cfg=config())
  {
    static cc* const null_cc=0;
    typedef std::vector<cc*> ccp_vec;

	CPP::CPPImage* cppImage = (CPP::CPPImage*)image->get();
    l->clear();
	int currGroupId = 1;
	unsigned int h = cppImage->get_height();
	unsigned int w = cppImage->get_width();
	ccp_vec last(w,0),cur(w,0);
    for(unsigned y=cfg.min_y;y<h && y<cfg.max_y;++y)
	{	
      for(unsigned x=cfg.min_x;x<w && x<cfg.max_x;++x)
      {  
        uchar pixel=*(cppImage->get_row(y,x));
		//uchar pixel = getVal(*image,y,x);
		
        if (pixel)
        {
          if (last[x]) 
          {
            last[x]->add(x,y,cfg.collect_images);
            cur[x]=last[x];
            if (x>0 && cur[x-1]) last[x]->unite(*cur[x-1]);
			
          }
          else
          if (x>0 && cur[x-1]) 
          {
            cur[x-1]->add(x,y,cfg.collect_images);
            cur[x]=cur[x-1];
          }
          else
          {
            l->push_back(cc(currGroupId));
			++currGroupId;
            l->back().add(x,y,cfg.collect_images);
            cur[x]=&(l->back());

          }
        }
      }
      last.swap(cur);
      std::fill(cur.begin(),cur.end(),null_cc);
    }
	
	// Remove all the zombies that are left leaderless
    l->erase(std::remove_if(l->begin(),l->end(),std::mem_fun_ref(&cc::has_leader)),l->end());
  }
};

typedef cc::cc_list cc_list;
typedef std::vector<cc> cc_vec;

struct smaller_than : public std::unary_function<cc,bool>
{
  int N;
  smaller_than(int n) : N(n){} 
  bool operator() (const cc& c) const { return int(c.pixels)<N; }
};

struct shorter_than : public std::unary_function<cc,bool>
{
  int N;
  shorter_than(int n) : N(n){} 
  bool operator() (const cc& c) const { return c.rect.height()<N; }
};

inline void remove_small_cc(cc_list& l, int min_size)
{
  l.erase(std::remove_if(l.begin(),l.end(),smaller_than(min_size)),l.end());
}

inline void remove_short_cc(cc_list& l, int min_size)
{
  l.erase(std::remove_if(l.begin(),l.end(),shorter_than(min_size)),l.end());
}

////added by Assaf and Hila
//inline double findInterRatio(coords_vec generalCoords, coords_vec depthCoords)
//{
//	double matchPixelsCounter = 0;
//	for(int i=0; i<generalCoords.size(); i++)
//	{
//		pixel_coords currGeneral = generalCoords[i];
//		for(int j=0; j<depthCoords.size(); j++)
//		{
//			pixel_coords currDepth = depthCoords[i];
//			if (currGeneral.first==currDepth.first && currGeneral.second==currDepth.second)
//			{
//				matchPixelsCounter++;
//				break;
//			}
//		}
//	}
//	return (matchPixelsCounter / generalCoords.size());
//}

//added by Assaf and Hila
inline double findContainRatio(cc& cc1, cc& cc2)
{
	//Calculate rectangle of cc1.
	int xMin1 = cc1.rect.l;
	int xMax1 = cc1.rect.r;
	int yMin1 = cc1.rect.t;
	int yMax1 = cc1.rect.b;
	double area1 = (xMax1-xMin1)*(yMax1-yMin1);

	//Calculate rectangle of cc2.
	int xMin2 = cc2.rect.l;
	int xMax2 = cc2.rect.r;
	int yMin2 = cc2.rect.t;
	int yMax2 = cc2.rect.b;
	double area2 = (xMax2-xMin2)*(yMax2-yMin2);

	//Calculate the intersection rectangle. 
	int xMin = max(xMin1, xMin2);
	int xMax = min(xMax1, xMax2);
	int yMin = max(yMin1, yMin2);
	int yMax = min(yMax1, yMax2);
	double area = (xMax-xMin)*(yMax-yMin);

	//Check that the 2 rectangles are intersect.
	bool check = (xMax >= xMin) && (yMax >= yMin);

	if (!check)
	{
		return -1;
	}

	// cc2 is bigger.
	if (area1<=area2)
	{
		return area/area2;
	}
	// cc1 is bigger.
	else
	{
		return area/area1;
	}
}

inline void findGeneral2DepthContainRatio(cc& generalCC, cc& depthCC,pair<double,double>& res)
{
		//Calculate rectangle of cc1.
	unsigned int xMinDepth = depthCC.rect.l;
	unsigned int xMaxDepth = depthCC.rect.r;
	unsigned int yMinDepth = depthCC.rect.t;
	unsigned int yMaxDepth = depthCC.rect.b;
	double areaDepth = (xMaxDepth-xMinDepth)*(yMaxDepth-yMinDepth);

	//Calculate rectangle of cc2.
	unsigned int xMinGeneral = generalCC.rect.l;
	unsigned int xMaxGeneral = generalCC.rect.r;
	unsigned int yMinGeneral = generalCC.rect.t;
	unsigned int yMaxGeneral = generalCC.rect.b;
	double areaGeneral = (xMaxGeneral-xMinGeneral)*(yMaxGeneral-yMinGeneral);

	//Calculate the intersection rectangle. 
	unsigned int xMin = max(xMinGeneral, xMinDepth);
	unsigned int xMax = min(xMaxGeneral, xMaxDepth);
	unsigned int yMin = max(yMinGeneral, yMinDepth);
	unsigned int yMax = min(yMaxGeneral, yMaxDepth);
	double area = (xMax-xMin)*(yMax-yMin);

	//Check that the 2 rectangles are intersect.
	if (!((xMax >= xMin) && (yMax >= yMin)))// || (areaGeneral > areaDepth))
	{
		res.first = -1;
		res.second = -1;
		return;
		//return  pair<double,double>(-1,-1);
	}

	res.first = area / areaGeneral;
	res.second = area / areaDepth;
	//return pair<double,double>(,area / areaDepth);
}

inline CPP::Image buildImageNoHoles(cc& c)
{
	CPP::Image ccImg = c.build_image();
	cv::Mat* img = ((CPP::CPPImage*)ccImg.get())->get();
	//Image res(new CPPImage(img->width,img->height,1));
	//IplImage* resImg = ((CPPImage*)res.get())->get();
	int counter;
	ushort shiftY = 25;
	ushort shiftX = 2;
	double val;
	int t;
	for (ushort i = 0;i < img->rows;++i)
	{
		for (ushort j = 0;j < img->cols;++j)
		{
			val = img->at<float>(i, j);
			if (val > 0)
				continue;

			counter = 0;

			t = i-shiftY;
			if (t < 0)
			{
				t = 0;
			}
			if (img->at<float>(t, j) > 0)
			{
				++counter;
			}

			t = j-shiftX;
			if (t < 0)
			{
				t = 0;
			}
			if (img->at<float>(i, t) > 0)
			{
				++counter;
			}

			t = j+shiftX;
			if (t >= img->cols)
			{
				t = img->cols-2;
			}
			if (img->at<float>(i, t) > 0)
			{
				++counter;
			}
			
			t = i+shiftY;
			if (t >= img->rows)
			{
				t = img->rows-1;
			}
			if (img->at<float>(t, j) > 0)
			{
				++counter;
			}

			if (counter >= 2)
			{
				c.add(c.rect.l + j,c.rect.t + i,true);
			}
		}
	}

	//c.coords.push_back(pixel_coords(0,0));
	return c.build_image();
}

#endif // H_CC

