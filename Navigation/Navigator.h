#ifndef NAVIGATOR
#define NAVIGATOR

#include "Map.h"
#include "MovmentManager.h"
#include "Common.h"
#include "image.h"
#include <vector>

#include "nav_utils.h"
using namespace std;
using namespace CPP;

class Navigator
{
public:
	void init(string inputMapPath);
	void buildTrack(unsigned int sourceRoom, unsigned int destRoom);
	bool navigate(CPP::Image* colorImage,CPP::Image* image,Image* calcedImage);
  void stop();
	// display support
	Map* getMap();

private:
	Map* m_map;
	MovmentManager* m_movmentManager;
	vector<MapPart*> m_partsInPath;
	unsigned int m_currMapPartIndex;
};

#endif