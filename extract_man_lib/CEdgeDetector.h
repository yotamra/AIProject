#ifndef CEDGEDETECTOR_H
#define CEDGEDETECTOR_H

#include "cc.h"
#include "image.h"
#include "hsv.h"
#include <map>
using namespace std;
using namespace CPP;

struct ImageCC
{
	cc_list* depthCC;
	cc_list* generalCC;
	vector<unsigned int>* general_to_depth;
	Image hPic;
};


class CEdgeDetector
{
public :
	static void analyze(Image& colorImage,Image& depthImage,ImageCC& imageCC);

//private:
	static void invImage(Image& src);
	static vector<unsigned int>* matchGeneralToDepthCC(cc_list& generalCC, cc_list& depthCC);
	static void cleanNoise(cc_list* src,int sizeLimit = -1);
	static void calcLaplace(unsigned w,unsigned h,
							Image& colorImage,Image& depthImage,
							Image& dstImage,Image& depthLaplace,Image& hPic);
	static void setCCIds(cc_list* CC);
	static void drawBottomLine(Image& src);
};

#endif /* CEDGEDETECTOR_H */