#include "CPersonTracker.h"
#include <limits>

double CPersonTracker::trackedPersonHist[NUM_COLOR_BINS] = {};
//fix
double CPersonTracker::lastDist = -1;
bool CPersonTracker::lost = true;

/*
Takes a picture of the H space quantization and the person's connected component and 
returns the histogram of the person's colors.
*/
void CPersonTracker::calcPersonHist(Image& HPic, cc& person, int& l, int& t, double* hist)
{
	l = person.rect.l;
	t = person.rect.t;
	int numOfPixels = person.coords.size();

	int x, y, bin;
	//double val;

	//init hist with zeros.
	for(int i=0; i<NUM_COLOR_BINS; i++)
	{
		hist[i]=0;
	}

	//Match each pixel of the person to the correct bin in the histogram;
	for(int i=0; i<numOfPixels; i++) 
	{
		x = person.coords[i].second;
		y = person.coords[i].first;

		//check the value of the current pixel in the H image.
		//val = getVal(HPic,x,y);
		//cout << "val(" << x << "," << y << ")=" << val <<endl;
		//bin = (int)(val / BIN_SIZE);
    bin = (int) getVal(HPic,x,y);
    //if (bin > 6){cout << "error" << endl;}
		hist[bin]++;
	}

	//Calculate the ratio : numOfPixelInBin/TotalNumOfPixels.
	for(int i=0; i<NUM_COLOR_BINS; i++)
	{
		hist[i] /= numOfPixels;
	}
}


/*
Calculates the euclidean distance between the two color histograms.
*/
double CPersonTracker::compareHist(double* hist1, double* hist2)
{
	double totalDistance = 0;
	
	for(int i=0; i<NUM_COLOR_BINS; i++)
	{
		totalDistance += pow((hist1[i]-hist2[i]), 2);
	}

	return sqrt(totalDistance);
}

/*
Inits the color histogram of the given person's connected component in the image.
*/
void CPersonTracker::initHist(Image& HPic,cc& person)
{
	double personHist[NUM_COLOR_BINS];
	int l,t;
	calcPersonHist(HPic, person, l, t, personHist);
	cout << "init hist of " << person.cc_id << " : " << endl;
	for(int i=0; i<NUM_COLOR_BINS; i++)
	{
		trackedPersonHist[i] = personHist[i]; 
		cout << i << ":" << personHist[i] << endl;
	}
}

Image CPersonTracker::findTrackedPerson(Image& HPic,cc_list& persons,int& left,int& top,bool& personFound,double& score)
{
	//double bestDistance = numeric_limits<double>::max();
	//double bestScore = numeric_limits<double>::min();
	//double currDistance = 0;
	//double personHist[NUM_COLOR_BINS];
	//int l, t;

	//cc_list::iterator i;
	//cc_list::iterator bestCC;

	//int num = 0;
	//for(i = persons.begin(); i != persons.end(); ++i)
	//{
	//	calcPersonHist(HPic, *i, l, t, personHist);
	//	currDistance = compareHist(trackedPersonHist, personHist);
	//	//cout << "cand " << num << " currDistance = " << currDistance << endl;
	//	if(currDistance < bestDistance)
	//	//if( ((bestDistance - currDistance)>0.1) ||
	//	//	(((currDistance - bestDistance)<0.1) && (i->score - bestCC->score > 1)) )
	//	{
	//		bestCC = i;
	//		bestDistance = currDistance;
	//		bestScore = i->score;
	//		left = l;
	//		top = t;
	//	}
	//	++num;
	//}

	//if ((bestDistance > MAX_HIST_DIST) || (persons.size() == 0))
	//{
	//	personFound = false;
	//	return Image();
	//}
	//else
	//{
	//	// fix the top and left
	//	top *= 2;
	//	left *= 2;
	//	score = bestCC->score;
	//	personFound = true;
	//	return bestCC->build_image();
	//}
	double bestDistance = numeric_limits<double>::max();
	double bestScore = numeric_limits<double>::min();
	double currDistance = 0;
	double personHist[NUM_COLOR_BINS];
	int l, t;

	cc_list::iterator i;
	cc_list::iterator bestCC;

	int num = 0;
	for(i = persons.begin(); i != persons.end(); ++i)
	{
		calcPersonHist(HPic, *i, l, t, personHist);
		currDistance = compareHist(trackedPersonHist, personHist);
		//cout << "cand " << num << " currDistance = " << currDistance << endl;
		if(currDistance < bestDistance)
		//if( ((bestDistance - currDistance)>0.1) ||
			//(((currDistance - bestDistance)<0.1) && (i->score - bestCC->score > 1)) )
		{
			bestCC = i;
			bestDistance = currDistance;
			bestScore = i->score;
			left = l;
			top = t;
		}
		++num;
	}

	/*cout << " best hist " << endl;
	for(int j=0; j<NUM_COLOR_BINS; j++)
	{
		cout << "bin " << j << ":" << bestHist[j] << endl;
	}*/
	
	if ((persons.empty()) || ((bestDistance > MAX_HIST_DIST) && (lost)))
	{
		lastDist = -1;
		//cout << "lost !!" << endl;
		lost = true;
		personFound = false;
	
      return Image();
   
  }

	if (lastDist == -1)
	{
		lastDist = bestDistance;
	}

	// found the person - check if we can update the hist
	if (abs(bestDistance - lastDist) < 0.4)
	{
		
		lastDist = bestDistance;
    // fix the top and left
		top *= 2;
		left *= 2;
		score = bestCC->score;
		personFound = true;
		return bestCC->build_image();
	}
	else if (bestDistance > MAX_HIST_DIST)
	{
		lost = true;
		personFound = false;
 	}
  return Image();
}

//TODO:: this function is a copy of the function findTrackedPerson (for the test).
//DON'T FORGET TO MAKE ALL CHANGES THERE!!!!
bool CPersonTracker::findTrackedPersonForTester(Image& HPic,cc_list& persons,
												int& left,int& top, double* bestHist,cc& trackedPerson,string& imageNum)
{
	double bestDistance = numeric_limits<double>::max();
	double bestScore = numeric_limits<double>::min();
	double currDistance = 0;
	double personHist[NUM_COLOR_BINS];
	int l, t;

	cc_list::iterator i;
	cc_list::iterator bestCC;

	int num = 0;
	for(i = persons.begin(); i != persons.end(); ++i)
	{
		calcPersonHist(HPic, *i, l, t, personHist);
		currDistance = compareHist(trackedPersonHist, personHist);
		//cout << "cand " << num << " currDistance = " << currDistance << endl;
		//if(currDistance < bestDistance)
		if( ((bestDistance - currDistance)>0.1) ||
			(((currDistance - bestDistance)<0.1) && (i->score - bestCC->score > 1)) )
		{
			bestCC = i;
			bestDistance = currDistance;
			bestScore = i->score;
			left = l;
			top = t;
			for(int j=0; j<NUM_COLOR_BINS; j++)
			{
				bestHist[j] = personHist[j];
			}
		}
		++num;
	}

	
	if ((persons.empty()) || (bestDistance > MAX_HIST_DIST))
	{
		cout << imageNum << " best distance : " << bestDistance << endl;
		return false;
	}

	cout << imageNum << " best distance : " << bestDistance  << " score : " << bestCC->score << endl;
	trackedPerson = *bestCC;
	return true;
}