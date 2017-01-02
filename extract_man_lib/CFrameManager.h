#ifndef CFRAMEMANAGER_H
#define CFRAMEMANAGER_H

#include "cc.h"
#include "image.h"
#include "CComponentMerger.h"
#include "CPersonClassifier.h"


using namespace CPP;

struct Result
{
	cc_list* finalObjects;
	cc_list* depthCC;
	cc_list* generalCC;
	vector<unsigned int>* general_to_depth;
};

/*
This class is presenting segmentation services for an image.
*/
class CFrameManager
{
public:
	// method for edge detect debug
	static void findObjects(Image& colorImage,Image& depthImage,Result& res);

	// method for hist and classifier debug
	static Image buildPersonRect(Image& colorImage,Image& depthImage,
								ClsEvaluator* te, string& resDir, string& imageNum,bool saveTracking);

	// operational method (a comment for this method is where it is implemented)
	static Image extract_man(Image& colorImage,Image& depthImage, 
							 TargetEvaluator* te, int& off_x, int& off_y,bool& personFound);

	static bool initPersonHist(Image& colorImage,Image& depthImage,ClsEvaluator* te, int& off_x, int& off_y,Image& target);
  static void displayCC(cc_list* resCC);
  static void displayImage(Image& image); 
};

#endif /* CFRAMEMANAGER_H */