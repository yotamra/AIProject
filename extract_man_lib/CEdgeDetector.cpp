#include "CEdgeDetector.h"
#include "math.h"
#include "InputConstants.h"
#include <iostream>
#include "CFrameManager.h"

using namespace CPP;

/*
Translates hue values into numbers representing colors.
*/
int calcBin7(int hVal)
{
	if( (hVal>=340) || (hVal<=20) ) //red
	{
		return 0;
	}
	else if( (hVal>=21) && (hVal<=50) ) //orange
	{
		return 1;
	}
	else if( (hVal>=51) && (hVal<=75) ) //yellow
	{
		return 2;
	}
	else if( (hVal>=76) && (hVal<=140) ) //green
	{
		return 3;
	}
	else if( (hVal>=141) && (hVal<=210) ) //pale blue
	{
		return 4;
	}
	else if( (hVal>=211) && (hVal<=267) ) //blue
	{
		return 5;
	}
	else if( (hVal>=267) && (hVal<=339) ) //pink+purple
	{
		return 6;
	}
  return 6;
}

/*
Calculates the distance between a pixel and its neighbours as explained in the
project's report docx file.
*/
double calc_dist(hsl& a, byte da, hsl& b, byte db, double& depth_dist)
{
	double dda=da/256.0,ddb=db/256.0;
	//double sa=a.s*sin(a.h),ca=a.s*cos(a.h);
	//double sb=b.s*sin(b.h),cb=b.s*cos(b.h);
	double sa=a.sy,ca=a.sx;
	double sb=b.sy,cb=b.sx;
	double d;
    depth_dist = sqr(dda-ddb);
	d=sqr(a.l-b.l)+sqr(sa-sb)+sqr(ca-cb)+12.25*sqr(dda-ddb);
	return d;
}

void CEdgeDetector::calcLaplace(unsigned w,unsigned h,
								Image& colorImage,Image& depthImage,
								Image& dstImage,Image& depthLaplace,Image& hPic)
{
	Mat* dstFrame = ((CPPImage*)dstImage.get())->get();

	Matrix<double> mat(w,h,0.0);
	Matrix<double> matDepth(w,h,0.0);

	//convert image from BGR to HSL.
	byte* row, *hRow;
	hsl hslInit(0,0,0,0);
	rgb src;
	Matrix<hsl> matHsl(w,h,hslInit);
	hsl* hslRow;
	for (unsigned int i = 0; i < h; ++i)
	{
		row = colorImage->get_row(i);
		hRow = hPic->get_row(i);
		hslRow = matHsl.get_row(i);
		for (unsigned int j = 0; j < w; ++j)
		{
			src.set(row);
			*hslRow = hsl(src);

			// Save H channel for histogram.
			(*hRow) = (byte)calcBin7(((*hslRow).h/(2*PI))*360);
      if ((*hRow) > 6){cout << "error" << endl;}

			row += 3;
			hRow++;
			hslRow++;
		}
	}

	
	byte* depth_row, *depth_down_row;
	double rightDiff, downDiff, rightDepthDiff, downDepthDiff;
	double maxV = -1, maxDepthV = -1;
	unsigned int shiftSizeX = 2;
	unsigned int shiftSizeY = 2;

	hsl* hsl_row, *hsl_down_row = 0;
	double* total_diff_row, *depth_diff_row, *total_diff_down_row, *depth_diff_down_row;

	for (unsigned int i = shiftSizeY; i < (h-shiftSizeY); ++i)
	{
		depth_row=depthImage->get_row(i)+shiftSizeX;
		depth_down_row=depthImage->get_row(i+shiftSizeY)+shiftSizeX;

		hsl_row = matHsl.get_row(i)+shiftSizeX;
		hsl_down_row = matHsl.get_row(i+shiftSizeY)+shiftSizeX;

		depth_diff_row = matDepth.get_row(i)+shiftSizeX;
		depth_diff_down_row = matDepth.get_row(i+shiftSizeY)+shiftSizeX;

		total_diff_row = mat.get_row(i)+shiftSizeX;
		total_diff_down_row = mat.get_row(i+shiftSizeY)+shiftSizeX;

		for (unsigned int j = shiftSizeX; j < (w-shiftSizeX); ++j)
		{
			rightDiff = calc_dist(*hsl_row, *depth_row, *(hsl_row+shiftSizeX), *(depth_row+shiftSizeX), rightDepthDiff);
			downDiff = calc_dist(*hsl_row, *depth_row, *hsl_down_row, *(depth_down_row), downDepthDiff);

			*(depth_diff_row+shiftSizeX) = max(*(depth_diff_row+shiftSizeX), rightDepthDiff);
			*(depth_diff_down_row) = max(*depth_diff_down_row, downDepthDiff);

			*(total_diff_row+shiftSizeX) = max(*(total_diff_row+shiftSizeX), rightDiff);
			*(total_diff_down_row) = max(*total_diff_down_row, downDiff);

			*(depth_diff_row) = max(*(depth_diff_row), max(rightDepthDiff, downDepthDiff));

			*(total_diff_row) = max(*(total_diff_row), max(rightDiff, downDiff));

			maxDepthV = max(matDepth(j,i),maxDepthV);
			maxV = max(mat(j,i),maxV);

			depth_row++;
			depth_down_row++;

			hsl_row++;
			hsl_down_row++;

			depth_diff_row++;
			depth_diff_down_row++;

			total_diff_row++;
			total_diff_down_row++;
		}
	}

	// normalize the image
	byte* colorRow, *depthRow = NULL;
	for (unsigned int i = 0;i < h;++i)
	{
		colorRow = dstImage->get_row(i);
		depthRow = depthLaplace->get_row(i);

		depth_diff_row = matDepth.get_row(i);
		total_diff_row = mat.get_row(i);

		for (unsigned int j = 0;j < w;++j)
		{
			(*colorRow) = (((*total_diff_row)*255/maxV) > 0.15) ? 255 : 0; 

			(*depthRow) = (((*depth_diff_row)*255/maxDepthV) > 0.1) ? 255 : 0;

			colorRow++;
			depthRow++;
			total_diff_row++;
			depth_diff_row++;
		}
	}
	
	// clean noise
	cv::medianBlur(*dstFrame,*dstFrame,3);
	
	// draw line to seperate the top black
	colorRow = dstImage->get_row(shiftSizeY);
	depthRow = depthLaplace->get_row(shiftSizeY);
	for (unsigned int j = 0;j < w; ++j)
	{
		(*colorRow) = 255;
		(*depthRow) = 255;
		colorRow++;
		depthRow++;
	}
	colorRow = dstImage->get_row(h-shiftSizeY);
	depthRow = depthLaplace->get_row(h-shiftSizeY);
	for (unsigned int j = 0;j < w; ++j)
	{
		(*colorRow) = 255;
		(*depthRow) = 255;
		colorRow++;
		depthRow++;
	}
	for(unsigned int i = 0; i < h; ++i)
	{
		colorRow = dstImage->get_row(i);
		depthRow = depthLaplace->get_row(i);
		*(colorRow+shiftSizeX) = 255;
		*(colorRow+w-shiftSizeX) = 255;
		*(depthRow+shiftSizeX) = 255;
		*(depthRow+w-shiftSizeX) = 255;
	}
}

/*
Uses the color image and depth image to calculate the connected components by both images.
Also, saves a vector that maps a connected component to its depth.
All of that is saved in the ImageCC object- res.
*/
void CEdgeDetector::analyze(Image& colorImage,Image& depthImage,ImageCC& res)
{
	int w = ((CPPImage*)depthImage.get())->get_width();
	int h = ((CPPImage*)depthImage.get())->get_height();

	Image total_laplace(new CPPImage(w,h,1));
	Image depth_laplace(new CPPImage(w,h,1));
	Image hPic(new CPPImage(w,h,1));

	calcLaplace(w,h,colorImage,depthImage,total_laplace,depth_laplace,hPic);

	double now1 = GetTickCount();
	invImage(total_laplace);

	// build general CC
	cc_list* generalCC = new cc_list();


	cc::analyze(&total_laplace, generalCC,cc::config(true));
	cleanNoise(generalCC,MIN_CC_SIZE);
	setCCIds(generalCC);
	
	// debug
  //CFrameManager::displayCC(generalCC);

	// paint the line on the floor on the depth binary image
  
	drawBottomLine(depth_laplace);
	invImage(depth_laplace); 
	
	// build the CC of depth pic
	cc_list* depthCC = new cc_list();
	cc::analyze(&depth_laplace, depthCC,cc::config(true));
	cleanNoise(depthCC);
	setCCIds(depthCC);


	// debug
	//CFrameManager::displayCC(depthCC);

	// match the general CC to the depth CC
	vector<unsigned int>* match = matchGeneralToDepthCC(*generalCC, *depthCC);

	res.depthCC = depthCC;
	res.generalCC = generalCC;
	res.general_to_depth = match;
	res.hPic = hPic;
}

void CEdgeDetector::invImage(Image& src)
{
	Mat* srcImg = ((CPPImage*)src.get())->get();
	cv::bitwise_not(*srcImg,*srcImg);
}
void CEdgeDetector::cleanNoise(cc_list* src,int sizeLimit)
{
	// if the user didnt pass a limit - use avg size
	if (sizeLimit == -1)
	{
		sizeLimit = 0;
		for (cc_list::iterator currCC = src->begin();
			 currCC != src->end();
			 ++currCC)
		{
			sizeLimit += currCC->pixels;
		}

		sizeLimit = sizeLimit / src->size();
	}

	remove_small_cc(*src,sizeLimit);
}
void CEdgeDetector::setCCIds(cc_list* CC)
{
	cc_list::iterator i;
	int id = 0;

	for(i=CC->begin(); i != CC->end(); ++i)
	{
		i->cc_id = id;
		id++;
	}
}

vector<unsigned int>* CEdgeDetector::matchGeneralToDepthCC(cc_list& generalCC, cc_list& depthCC)
{
	//map<unsigned int,unsigned int>* res = new map<unsigned int,unsigned int>();
	vector<unsigned int>* res = new vector<unsigned int>(generalCC.size());
	pair<double,double> bestContain(0,0);
	pair<double,double> containRatio;

	cc_list::iterator i;
	cc_list::iterator j;

	for(i=generalCC.begin(); i != generalCC.end(); ++i)
	{
		int bestCC = 0;
		bestContain.first = 0;
		bestContain.second = 0;
		
		for(j=depthCC.begin(); j != depthCC.end(); ++j)
		{
			findGeneral2DepthContainRatio(*i, *j,containRatio);
			if (containRatio.first == -1)
				continue;

			if ((containRatio.first - bestContain.first > 0.1) ||
			   ((abs(containRatio.first - bestContain.first) <= 0.1) &&
				(containRatio.second > bestContain.second)))
			{

				bestContain = containRatio;
				bestCC = j->cc_id;
			}
		}
		(*res)[i->cc_id] = bestCC;	
	}

	return res;
}

struct ColData
{
	int col;
	int sum;
	int lowestPixelLineNum;
	int distFromNextCol;
	int distFromPrevCol;
};

void CEdgeDetector::drawBottomLine(Image& src)
{
	//find the bottom row on the image
	Mat* img = ((CPPImage*)src.get())->get();

	int rowSum = 0;
	int i = 0;

	// search the bottom line until we hit a minimum sum
	for(i=img->rows-1; i>0; i--)
	{
		rowSum = 0;
		double* mat_row = img->ptr<double>(i);
		for(int j=FRAME_OFFSET; j<img->cols-FRAME_OFFSET; j++)
		{
			if (mat_row[j] != 0)
			{
				++rowSum;
			}
		}

		if(rowSum > MIN_NUM_OF_PIXELS_ON_BOTTOM_LINE)
		{
			break;
		}
	}

	int bottom = i-10;

	int sub = 100;
	if (bottom > (3/4)*img->rows)
	{
		sub = 130;
	}

	int top = bottom - sub;
	if (top  < img->rows/2)
	{
		top = img->rows/2;
	}
	ColData cols[640/2] = {0,0,0,30,30};
	int currMin = 50;
	int currMinIndex = -1;

	// calc the sum of each col
	for (int k = 0;k < 640/2;++k)
	{
		for (int t = top;t < bottom;++t)
		{
			if ((int)cv::Scalar(img->at<uchar>(t, k)).val[0])
			{
				++cols[k].sum;
				cols[k].lowestPixelLineNum = t;
			}
		}
	}

	// save only the cols that make a peak
	for (int k = 6;k < img->cols - 6;++k)
	{
		if ((cols[k].sum - cols[k+6].sum < 35) &&
			(cols[k].sum - cols[k-6].sum < 35))
		{
			cols[k].sum = 0;
		}
	}

	// calc for each peak it's distance from the next and prev peak
	for (int k = 0;k < img->cols;++k)
	{
		if (cols[k].sum != 0)
		{
			int currCol = k;
			int dist = 1;
			++k;
			while ((cols[k].sum == 0) && (k < img->cols))
			{
				++k;
				++dist;
			}

			cols[k].distFromPrevCol = cols[currCol].distFromNextCol = dist;
			--k;
		}
	}

	// remove the peaks the are too close (from both sides) to another peak
	for (int k = 0;k < img->cols;++k)
	{
		if (cols[k].sum != 0)
		{
			if (((cols[k].distFromNextCol < MIN_SIZE_OF_PERSON) || 
				 (cols[k].distFromNextCol > MAX_SIZE_OF_PERSON)) &&
				(((cols[k].distFromPrevCol < MIN_SIZE_OF_PERSON) || 
				  (cols[k].distFromPrevCol > MAX_SIZE_OF_PERSON))))
			{
				cols[k].sum = 0;
			}
		}
	}

	// paint a line in the lowest pixel of the peak 
	for (int k = 0;k < img->cols;++k)
	{
		if (cols[k].sum != 0)
		{
			int currCol = k;
			int dist = 1;
			++k;
			while ((cols[k].sum == 0) && (k < img->cols))
			{
				++k;
				++dist;
			}

			int line = min (cols[currCol].lowestPixelLineNum,cols[k].lowestPixelLineNum);
			
			int s = currCol-50;
			if (s < 0)
			{
				s = 0;
			}
			int e = k+50;
			if (e > img->cols)
			{
				e = img->cols;
			}
			for(int j=s; j<e; j++)
			{
				setVal(src,line,j,255);
			}
			--k;
		}
	}
	for(int j=0; j<img->cols; j++)
	{
		setVal(src,bottom,j,255);
	}
}