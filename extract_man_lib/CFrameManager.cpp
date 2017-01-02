#include "CFrameManager.h"
#include "CEdgeDetector.h"
#include "CPersonTracker.h"

#include "InputConstants.h"
#include "image.h"

int imageNum = 0;
// method for edge detect debug
void CFrameManager::findObjects(Image& colorImage,Image& depthImage,Result& res)
{
	// build the CC of the image
	ImageCC imageCC;
	CEdgeDetector::analyze(colorImage,depthImage,imageCC);

	// merge sub cc
	cc_list* mergeList = CComponentMerger::mergeObjects(&imageCC);
	
	res.finalObjects = mergeList;
	res.general_to_depth = imageCC.general_to_depth;
	res.depthCC = imageCC.depthCC;
	res.generalCC = imageCC.generalCC;
}

// method for hist and classifier debug
Image CFrameManager::buildPersonRect(Image& colorImage,Image& depthImage,ClsEvaluator* te,
									 string& resDir, string& imageNum,bool saveTracking)
{
	int off_x,off_y;
//	double start = GetTickCount();
//	printf("start tracking pic %s \n",imageNum.c_str());
	
	// find the cc of the color and depth image
	ImageCC imageCC;
	CEdgeDetector::analyze(colorImage,depthImage,imageCC);
	
	// merge cc
	cc_list* mergeList = CComponentMerger::mergeObjects(&imageCC);

	// find the persons in the cc list
	cc_list personsList = CPersonClassifier::classify(*mergeList,te);

	// find the specific person to track
	double hist[NUM_COLOR_BINS];
	cc trackedPerson;
	bool personInImage = CPersonTracker::findTrackedPersonForTester(imageCC.hPic,personsList,off_x,off_y,hist,trackedPerson,imageNum);
	
//	double end = GetTickCount();
//	printf("tracking on pic %s took %f sec\n",imageNum.c_str(),(end - start)/ 1000);

	if (!personInImage)
	{
		printf("person not found\n");
	}

	// save candidates
	if (saveTracking)
	{
		// save final image
		//string finalObjsPicPath = resDir;
		//finalObjsPicPath.append(imageNum).append("_finalObjs.png");
		//Image finalObjectsPic(new CPPImage(640,480,3));
		//CTester :: colorImageByCC(mergeList,finalObjectsPic);
		//((CPPImage*)finalObjectsPic.get())->save(finalObjsPicPath);

		cc_list::iterator itr;
		int num = 1;
		for(itr = personsList.begin(); itr != personsList.end(); ++itr)
		{
			char buff[50];
			itoa(num,buff,10);
			string candPicPath = resDir;
			candPicPath.append(imageNum).append("_cand").append(buff).append(".png");
			Image p = itr->build_image();
			((CPPImage*)p.get())->save(candPicPath);
			//cout << "cand " << itr->cc_id << " score : " << itr->score << endl;
			++num;
		}
	}
	
	int w = ((CPPImage*)colorImage.get())->get_width();
	int h = ((CPPImage*)colorImage.get())->get_height();
	CvSize size = cvSize(w,h);
	IplImage* img = cvCreateImage( size, 8,3 );
	
	if (saveTracking)
	{	
		// create HSV image and fill it with 
		cvCopyImage(((CPPImage*)colorImage.get())->get(),img);
		cvCvtColor(img,img,CV_BGR2HSV);

		CvScalar temp;
		temp.val[0] = 0;
		temp.val[1] = 240;
		temp.val[2] = 150;
		int i=0;
		int j=0;
		coords_vec::iterator currPixel = trackedPerson.coords.begin();
		for (int k = 0;k < NUM_COLOR_BINS;++k)
		{
			int numPixelsInBin = (int)(hist[k]*trackedPerson.coords.size());
			
			for (int t = 0;t < numPixelsInBin;++t)
			{
				i = currPixel->second;
				j = currPixel->first;
				cvSet2D(img,i,j,temp);
				++currPixel;
			}
			temp.val[0] += BIN_SIZE;
		}

		// translate HSV to RGB
		cvCvtColor(img,img,CV_HSV2BGR);
	}

	// release allocated memory
	delete imageCC.depthCC;
	delete imageCC.general_to_depth;
	delete imageCC.generalCC;


	return Image(new CPPImage(img));
}
bool compare_cc(cc first, cc second)
{
	if (first.group_id < second.group_id)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
Colors an Image in a way that each connected component's pixels get different color.
*/
map<unsigned int,CvScalar>* colorImageByCC(cc_list* CC_list,Image& res)
{
	map<unsigned int,CvScalar>* depthIdToColor = new map<unsigned int,CvScalar>();
	
	//create the output color image. 

	//sort the CC list by group_id
	CC_list->sort(compare_cc);
	
	//set uniqe color for each group_id
	cc_list::iterator cc;
	cc=CC_list->begin();
	int currGroupId = cc->group_id;
	int r = 17;
	int g = 56;
	int b = 100;
	int bDelta = 10;
	int gDelta = 20;
	int rDelta = 30;

	
	for (cc=CC_list->begin(); cc!=CC_list->end(); ++cc)
	{
		if(currGroupId != cc->group_id)
		{
			//change the color.
			r = (r+rDelta)%255;
			g = (g+gDelta)%255;
			b = (b+bDelta)%255;
			rDelta = rand();
			gDelta = rand();
			bDelta = rand();
		}
		//set the color for all the pixels in the current cc.
		for(unsigned int i=0; i<cc->pixels; i++)
		{
			int x = cc->coords[i].first;
			int y = cc->coords[i].second;
			CvScalar color;
			color.val[0] = r; 
			color.val[1] = g;
			color.val[2] = b;
			color.val[3] = 0;
			(*depthIdToColor)[cc->cc_id] = color;
			cvSet2D(((CPPImage*)res.get())->get(),y,x, color);
		}
		currGroupId = cc->group_id;
	}
	return depthIdToColor;
}

/*
Displays the given image in a new window.
*/
void CFrameManager::displayImage(Image& image)
{
	IplImage *img = ((CPPImage*)image.get())->get();
	// Display the image.
    cvNamedWindow("Image:", CV_WINDOW_AUTOSIZE);
    cvShowImage("Image:", img);

    // Wait for the user to press a key in the GUI window.
    cvWaitKey(0);

    // Free the resources.
    cvDestroyWindow("Image:");
    //cvReleaseImage(&img);
}

/*
Displays an image representing the connected components list when each one gets a different color.
*/
void CFrameManager::displayCC(cc_list* resCC)
{
	Image res(new CPPImage(640,480,3));
	colorImageByCC(resCC,res);
	displayImage(res);
}


/*
Gets color image, depth image, target evaluator (the object that gets the classifier file).
Returns a boolean if the person is found or not and if so, then his coordinations are returned.
*/
Image CFrameManager::extract_man(Image& colorImage,Image& depthImage,
								 TargetEvaluator* te, int& off_x, int& off_y,bool& personFound)
{
	int w = ((CPPImage*)colorImage.get())->get_width();
	int h = ((CPPImage*)colorImage.get())->get_height();
	
	Image smaller_color(new CPPImage(w/2,h/2,3));
	cvResize(((CPPImage*)colorImage.get())->get(),((CPPImage*)smaller_color.get())->get());
	
	Image smaller_depth(new CPPImage(w/2,h/2,1));
	cvResize(((CPPImage*)depthImage.get())->get(),((CPPImage*)smaller_depth.get())->get());
	
	//cout << "recieved image..." << endl;
	// find the cc of the color and depth image

	ImageCC imageCC;
	CEdgeDetector::analyze(smaller_color,smaller_depth,imageCC);

	//cout << "found edges..." << endl;
	// merge cc
	cc_list* mergeList = CComponentMerger::mergeObjects(&imageCC);

	//cout << "merged components..." << endl;
	// find the persons in the cc list
	cc_list personsList = CPersonClassifier::classify(*mergeList,te);

	//cout << "found " << personsList.size() << " persons..." << endl;
	// find the specific person to track
	double score;
  
	personFound = true;
	Image res = CPersonTracker::findTrackedPerson(imageCC.hPic,personsList,off_x,off_y,personFound,score);

	///*char imageNumStr[10];
	//itoa(imageNum,imageNumStr,10);
	//int num = 0;
	//for(cc_list::iterator itr = personsList.begin(); itr != personsList.end(); ++itr)
	//{
	//	char buff[10];
	//	itoa(num,buff,10);
	//	string candPicPath = "C:\\Shared\\results\\cand\\";
	//	candPicPath.append(imageNumStr).append("_cand").append(buff).append(".png");
	//	Image p = itr->build_image();
	//	((CPPImage*)p.get())->save(candPicPath);
	//	++num;
	//}
	//++imageNum;*/


	if (personFound)
	{
		/*string candPicPath = "C:\\Shared\\results\\target\\";
		candPicPath.append(imageNumStr).append(".png");
		((CPPImage*)res.get())->save(candPicPath);*/
		
	}
	// release allocated memory
	delete imageCC.depthCC;
	delete imageCC.general_to_depth;
	delete imageCC.generalCC;
	
	return res;
}

bool compare_cc_by_score(cc first, cc second)
{
	if (first.score > second.score)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
Inits the color histogram of the person to follow.
*/
bool CFrameManager::initPersonHist(Image& colorImage,Image& depthImage,ClsEvaluator* te, int& off_x, int& off_y,Image& target)
{
	int w = ((CPPImage*)colorImage.get())->get_width();
	int h = ((CPPImage*)colorImage.get())->get_height();
	
	Image smaller_color(new CPPImage(w/2,h/2,3));
	cvResize(((CPPImage*)colorImage.get())->get(),((CPPImage*)smaller_color.get())->get());
	
	Image smaller_depth(new CPPImage(w/2,h/2,1));
	cvResize(((CPPImage*)depthImage.get())->get(),((CPPImage*)smaller_depth.get())->get());
	
	// find the cc of the color and depth image
	ImageCC imageCC;
	CEdgeDetector::analyze(smaller_color,smaller_depth,imageCC);

	// merge cc
	cc_list* mergeList = CComponentMerger::mergeObjects(&imageCC);

	//displayCC(mergeList);	
	// find the persons in the cc list
	cc_list personsList = CPersonClassifier::classify(*mergeList,te);


	if (personsList.size() == 0)
	{
		return false;
	}
	personsList.sort(compare_cc_by_score);
	cc person = personsList.front();
  	for(cc_list::iterator itr = personsList.begin(); itr != personsList.end(); ++itr)
	{
		cout << "init cand " << itr->cc_id << " score : " << itr->score << endl;
	}
	if (person.score < 1)
	{
		return false;
	}
	
	CPersonTracker :: initHist(imageCC.hPic, person);
    off_y = person.rect.t * 2;
		off_x = person.rect.l * 2;
    target = person.build_image();
	/*char imageNumStr[50];
	itoa(imageNum,imageNumStr,10);*/

	/*string candPicPath = "C:\\develop\\intelligent_systems\\robot_res\\init\\";
	candPicPath.append(imageNumStr).append("init.png");
	((CPPImage*)person.build_image().get())->save(candPicPath);*/

	// release allocated memory
	delete imageCC.depthCC;
	delete imageCC.general_to_depth;
	delete imageCC.generalCC;
	
	return true;
}