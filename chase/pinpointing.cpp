#include "pinpointing.h"
#include "image.h"
#include <stdio.h>
#include "InputConstants.h"
#include "Sounds.h"

void current_position_comparing_optimal(Image& colorImage,Image& depthImage)
{
  int w = ((CPPImage*)depthImage.get())->get_width();
	int h = ((CPPImage*)depthImage.get())->get_height();

	Image total_laplace(new CPPImage(w,h,1));
	Image depth_laplace(new CPPImage(w,h,1));
	Image hPic(new CPPImage(w,h,1));

  CEdgeDetector::calcLaplace(w,h,colorImage,depthImage,total_laplace,depth_laplace,hPic);

  //find the bottom row on the image
	IplImage* img = ((CPPImage*)depth_laplace.get())->get();

	int rowSum = 0;
	int i = 0;

	// search the bottom line until we hit a minimum sum
	for(i=img->height-5; i>0; i--)
	{
		rowSum = 0;
		for(int j=WALLS_OFFSET; j<img->width-WALLS_OFFSET; j++)
		{
			if (cvGet2D(img,i,j).val[0] != 0)
			{
				++rowSum;
			}
		}

    //putting lines in the picture for debugging visualy
    int jumpSize=20;

    if (i%jumpSize==0)
    {
      printf("line number %d has rowSum=%d\n",i/jumpSize,rowSum);
      for(int j=0; j<img->width; j++)
      {
        setVal(depth_laplace,i,j,255);
      }
        
    }


		if(rowSum > BAR_NUM_OF_PIXELS)
		{
      for(int j=0; j<img->width; j++)
      {
        setVal(depth_laplace,i,j,150);
      }
      printf("breaking in line %d with rowSum=%d",i,rowSum);
			break;
		}
	}



  if (i<BAR_MIN_WANTED_HEIGHT-BAR_HEIGHT_DELTA_RANGE)
  {
    close_wavs_and_play_new_wav("the camera is too low");
  }
  else if (i<BAR_MIN_WANTED_HEIGHT)
  {
    close_wavs_and_play_new_wav("the camera is a little too low");
  }
  else if (i<=BAR_MAX_WANTED_HEIGHT)
  {
    close_wavs_and_play_new_wav("the camera is ok");
  }
  else if (i<BAR_MAX_WANTED_HEIGHT+BAR_HEIGHT_DELTA_RANGE)
  {
    close_wavs_and_play_new_wav("the camera is a little too high");
  }
  else 
  {
    close_wavs_and_play_new_wav("the camera is too high");
  }
  //CFrameManager::displayImage(total_laplace);
  //CFrameManager::displayImage(depth_laplace);

}