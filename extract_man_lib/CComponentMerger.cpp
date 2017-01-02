#include "CComponentMerger.h"
#include "InputConstants.h"

#include "cc.h"
using namespace CPP;
/*
Merges connected components that belong to the same object such as
pants and shirt of the same person.
*/
cc_list* CComponentMerger::mergeObjects(ImageCC* imageCC)
{
	cc_list* depthCC = imageCC->depthCC;
	cc_list* generalCC = imageCC->generalCC;
	vector<unsigned int>* general_to_depth = imageCC->general_to_depth;
	
	cc_list::iterator i;
	cc_list::iterator j;
	
	double containmentRatio = 0;
	double intersectionRatio = 0;

	// Iterate over all generalCC pairs trying to merge 2 cc's.
	for(i = generalCC->begin(); i != generalCC->end(); ++i)
	{
		for(j = i; j != generalCC->end(); ++j)
		{
			if (i->getLeader()==j->getLeader())
			{
				continue;
			}

			//Check 3 merge conditions.
			if(isDepthAlike(*i,*j, general_to_depth) &&
			   ((getContainmentRatio(*(i->getLeader()), *(j->getLeader())) > MIN_CONTAINMENT_RATIO) ||
			   (getIntersectionRatio(*(i->getLeader()), *(j->getLeader())) > MIN_INTERSECTION_RATIO)))
			{
				i->unite(*j);
			}
		}
	}

	// remove all the non leaders
    generalCC->erase(std::remove_if(generalCC->begin(),generalCC->end(),std::mem_fun_ref(&cc::has_leader)),generalCC->end());
	return generalCC;
}


double CComponentMerger::getContainmentRatio(cc& cc1, cc& cc2)
{
	//Calculate rectangle of cc1.
	double area1 = (cc1.rect.r-cc1.rect.l)*(cc1.rect.b-cc1.rect.t);

	//Calculate rectangle of cc2.
	double area2 = (cc2.rect.r-cc2.rect.l)*(cc2.rect.b-cc2.rect.t);

	//Calculate rectangle of cc1.
	int xMin1 = cc1.rect.l;
	int xMax1 = cc1.rect.r;
	int yMin1 = cc1.rect.t;
	int yMax1 = cc1.rect.b;

	//Calculate rectangle of cc2.
	int xMin2 = cc2.rect.l;
	int xMax2 = cc2.rect.r;
	int yMin2 = cc2.rect.t;
	int yMax2 = cc2.rect.b;	

	//Calculate the intersection rectangle. 
	int xMin = max(cc1.rect.l, cc2.rect.l);
	int xMax = min(cc1.rect.r, cc2.rect.r);
	int yMin = max(cc1.rect.t, cc2.rect.t);
	int yMax = min(cc1.rect.b, cc2.rect.b);
	double area = (xMax-xMin)*(yMax-yMin);

	//Check that the 2 rectangles are intersect.
	if (!((xMax >= xMin) && (yMax >= yMin)))
	{
		return -1;
	}


	// cc2 is bigger.
	if (area1<=area2)
	{
		if( (yMax2>=FLOOR_HEIGHT) && (xMax2-xMin2>FLOOR_MIN_WIDTH) )
		{
			return -1;
		}
		if( (yMin2<=CEILING_HEIGHT) && (xMax2-xMin2>CEILING_MIN_WIDTH) )
		{
			return -1;
		}
		return area/area1;
	}
	// cc1 is bigger.
	else
	{
		if( (yMax1>=FLOOR_HEIGHT) && (xMax1-xMin1>FLOOR_MIN_WIDTH) )
		{
			return -1;
		}
		if( (yMin1<=CEILING_HEIGHT) && (xMax1-xMin1>CEILING_MIN_WIDTH) )
		{
			return -1;
		}
		return area/area2;
	}
}

double CComponentMerger::getIntersectionRatio(cc& cc1, cc& cc2)
{
	//Calculate rectangle of cc1.
	int xMin1 = cc1.rect.l;
	int xMax1 = cc1.rect.r;
	int yMin1 = cc1.rect.t;
	int yMax1 = cc1.rect.b;

	//Calculate rectangle of cc2.
	int xMin2 = cc2.rect.l;
	int xMax2 = cc2.rect.r;
	int yMin2 = cc2.rect.t;
	int yMax2 = cc2.rect.b;

	//Check if cc1 is above cc2. 
	if (yMin1<=yMin2 && yMax1<=yMax2)
	{
		if ((yMin2 - yMax1) > MAX_Y_INTERSECTION)
		{
			return -1;
		}
		if( (yMax2>=FLOOR_HEIGHT) && (xMax2-xMin2>FLOOR_MIN_WIDTH) )
		{
			return -1;
		}
		if( (yMin1<=CEILING_HEIGHT) && (xMax1-xMin1>CEILING_MIN_WIDTH) )
		{
			return -1;
		}
	}
	//Check if cc2 is above cc1.
	else if (yMin2<=yMin1 && yMax2<=yMax1)
	{
		if ((yMin1 - yMax2) > MAX_Y_INTERSECTION)
		{
			return -1;
		}
		if( (yMax1>=FLOOR_HEIGHT) && (xMax1-xMin1>FLOOR_MIN_WIDTH) )
		{
			return -1;
		}
		if( (yMin2<=CEILING_HEIGHT) && (xMax2-xMin2>CEILING_MIN_WIDTH) )
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

	//Calculate the intersection ratio.
	int xMinUnion = min(xMin1, xMin2);
	int xMaxUnion = max(xMax1, xMax2);
	int xMinInter = max(xMin1, xMin2);
	int xMaxInter = min(xMax1, xMax2);

	double unionSize = xMaxUnion-xMinUnion;
	double interSize = xMaxInter-xMinInter;

	return (interSize/unionSize);
}

bool CComponentMerger::isDepthAlike(cc& cc1, cc& cc2, vector<unsigned int>* general_to_depth)
{
	//Find the depth conponent that match cc1.
	int depthCC1 = (*general_to_depth)[cc1.cc_id];

	//Find the depth conponent that match cc2.
	int depthCC2 = (*general_to_depth)[cc2.cc_id];

	//Check if both cc's has the same depth component.
	return depthCC1 == depthCC2;
}
