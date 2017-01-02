#ifndef CCOMPONENTMERGER_H
#define CCOMPONENTMERGER_H

#include "cc.h"
#include "image.h"
#include "CEdgeDetector.h"

using namespace std;
using namespace CPP;

class CComponentMerger
{
public:
	static cc_list* mergeObjects(ImageCC* imageCC);

private:
	static double getContainmentRatio(cc& cc1, cc& cc2);
	static double getIntersectionRatio(cc& cc1, cc& cc2);
	static bool isDepthAlike(cc& cc1, cc& cc2, vector<unsigned int>* general_to_depth);
};

#endif /* CCOMPONENTMERGER_H */