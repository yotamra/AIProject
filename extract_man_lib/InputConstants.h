#ifndef INPUTCONSTANTS_H
#define INPUTCONSTANTS_H


//////////////////////////////// CComponentMerger Constants  ////////////////////////////////
#define MAX_Y_INTERSECTION 20 //Merge 2 objects by intersection ratio only if their y axis 
							 //intersection is less than MAX_Y_INTERSECTION.

#define MIN_CONTAINMENT_RATIO 0.6

#define MIN_INTERSECTION_RATIO 0.2

#define FLOOR_HEIGHT 460/2
#define FLOOR_MIN_WIDTH 200/2
#define CEILING_HEIGHT 20/2
#define CEILING_MIN_WIDTH 200/2

/////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////// CEdgeDetector Constants  //////////////////////////////////

#define MIN_CC_SIZE 40/4 // minimum connected component size to be consider not to be noise
					   // (can be -1 -> minimum size is the avg size of all cc in the image)

// draw line
#define FRAME_OFFSET 45/2 //for drawing the line on the bottom of the depth image.

#define BOTTOM_LINE_FIX 35/2 //fix the line after we find the bottom of the frame.

#define MIN_NUM_OF_PIXELS_ON_BOTTOM_LINE 20

#define MIN_SIZE_OF_PERSON 10 /4
#define MAX_SIZE_OF_PERSON 150/4


/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////// CPersonClassifier Constants  //////////////////////////////////

//#define NUM_PERSONS 5
#define MIN_PERSON_SIZE 600//8000/4
#define MIN_PERSON_HEIGHT 60//200/2


/////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////// CPersonTracker Constants  //////////////////////////////////

#define NUM_COLOR_BINS 7 //the number of equal parts that the H space is divided to
#define BIN_SIZE 43 //(255 / 6)
#define MAX_HIST_DIST 0.3 //0.4

/////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////// CFrameManager Constants  //////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////// pinpointing Constants  //////////////////////////////////

#define BAR_NUM_OF_PIXELS 250// the first row from the bottom of the picture with this number
//of pixels or above considered to be a bar line in pinpointing

#define WALLS_OFFSET 45 //the pixels of a row are counted withoud the offset from the right and left because there
//may be a wall on the sides



#define BAR_MIN_WANTED_HEIGHT 350 //the min hight in the picture that bar should start from when
//the person is 2 meters from the robot

#define BAR_MAX_WANTED_HEIGHT 360 //the max hight in the picture that bar should start from when
//the person is 2 meters from the robot

#define BAR_HEIGHT_DELTA_RANGE 25 //the delta between bar ranges

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* INPUTCONSTANTS_H */