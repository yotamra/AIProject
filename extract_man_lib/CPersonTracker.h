#ifndef PERSON_TRACKER
#define PERSON_TRACKER

#include "cc.h"
#include "InputConstants.h"
#include "image.h"

using namespace CPP;

class CPersonTracker
{
public:
	static Image findTrackedPerson(Image& HPic,cc_list& persons,int& left,int& top,bool& personFound,double& score);

	static bool findTrackedPersonForTester(Image& HPic,cc_list& persons,
										int& left,int& top, double* bestHist,cc& trackedPerson,string& imageNum);

	//initialize the histogram  according to the person in the given image.
	static void initHist(Image& HPic,cc& person); 

private:
	static double trackedPersonHist[NUM_COLOR_BINS];
	//fix
  static double lastDist;
	static bool lost;

	//Calculate the color histogram of the given person. 
	static void calcPersonHist(Image& HPic, cc& person, int& x, int& y, double* hist);
	
	//Calculate the distance between 2 histograms.
	static double compareHist(double* hist1, double* hist2);
};

#endif