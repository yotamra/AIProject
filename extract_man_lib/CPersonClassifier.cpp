#include "CPersonClassifier.h"
#include "InputConstants.h"
#include "image.h"
#include "cc.h"

using namespace CPP;

/*
cc_list CPersonClassifier::classify(cc_list& candidates,TargetEvaluator* te)
{
	double bestScores[NUM_PERSONS]={-9e99,-9e99,-9e99,-9e99,-9e99};
	cc bestCC[NUM_PERSONS];
	double score;
	Image ccImg;
	cc_list personsList;

	for(cc_list::iterator it = candidates.begin();
		it != candidates.end();
		++it)
	{
		cc c=*it;

		// remove all the cc that are small
			if ((c.rect.get_area() > MIN_PERSON_SIZE) &&
				(c.rect.height() > MIN_PERSON_HEIGHT) && 
				(c.rect.aspect_ratio() < 1))
			{
			// calc the current cc score
			ccImg = c.build_image();//buildImageNoHoles(c);
			
			//cout << "cc id = " << c.cc_id << endl;
			score = te->evaluate(ccImg);
			c.score = score;
			if (c.score < 0)
				continue;

			// check if the score is better
			for (int i = 0;i < NUM_PERSONS;++i)
			{
				
				if ((score > bestScores[i]))
				{
					// move all the scores down
					for (int j = NUM_PERSONS-1;j > i;--j)
					{
						bestScores[j] = bestScores[j-1];
						//bestCC[j] = bestCC[j-1];
						bestCC[j].cc_id = bestCC[j-1].cc_id;
						bestCC[j].coords = bestCC[j-1].coords;
						bestCC[j].leader = bestCC[j-1].leader;
						bestCC[j].group_id = bestCC[j-1].group_id;
						bestCC[j].pixels = bestCC[j-1].pixels;
						bestCC[j].rect = bestCC[j-1].rect;
						bestCC[j].score = bestCC[j-1].score;


					}

					// save the current best
					bestScores[i] = score;
					bestCC[i].cc_id = c.cc_id;
					bestCC[i].coords = c.coords;
					bestCC[i].leader = c.leader;
					bestCC[i].group_id = c.group_id;
					bestCC[i].pixels = c.pixels;
					bestCC[i].rect = c.rect;
					bestCC[i].score = c.score;

					//cout << "best score : " << bestCC[0].score << " cc.score " << c.score << endl;
					
					break;
				}
			}
		}
	}

	// copy only the cc the have valid scores
	for (int i = 0;i < NUM_PERSONS;++i)
	{
		if (bestScores[i] != -9e99)
		{
			personsList.push_back(bestCC[i]);
		}
	}


	return personsList;
}
*/
bool checkProportion(double firstWidth, double currWidth)
{
#define THRESHOLD_PROPORTION 1.7
	bool proportion = (firstWidth / currWidth <= THRESHOLD_PROPORTION && currWidth / firstWidth <= THRESHOLD_PROPORTION);
	return proportion;
}
bool evaluate(Image image,int& score)
{
#define THRESHOLD_COUNT 0.38
#define THRESHOLD_LARGEST 0.10
	int w = image->get_width(), h = image->get_height();
	int count = 0;
	int largest_streak = 0;
	int streak = 0;
	for (int y = 0; y < h; ++y)
	{
		const uchar* row = image->get_row(y);
		bool last = false;
		int runs = 0;
		int runsWidth = 0;
		int currWidth = 0;
		int firstWidth = 0;
		bool isValidRow = true;
		for (int x = 0; x < w; ++x)
		{
			bool cur = (row[x]>128);
			if (cur)
			{
				currWidth++;

			}
			if (!cur && last && currWidth >= 7 && currWidth < 66)
			{
				if (0 == runsWidth)
				{
					firstWidth = currWidth;
				}
				else if (runsWidth == 1 && !checkProportion(firstWidth, currWidth))
				{
					isValidRow = false;
					break;
				}
				else if (runsWidth >= 2)
				{
					isValidRow = false;
					break;
				}
				//get here when it first legs, or 2 legs and proportional
				++runsWidth;
			}
			if (!cur && last)
			{
				++runs;
			}
			if (!cur)
			{
				currWidth = 0;
			}
			last = cur;
		}
		if (runs < 4 && runsWidth == 2 && isValidRow)
		{
			++count;
			++streak;
			largest_streak = Max(streak, largest_streak);
		}
		else
			streak = 0;
	}
	score = 1+(double(count) / h);
	bool isManFound = ((double(count) / h)>THRESHOLD_COUNT) && ((double(largest_streak)) / h > THRESHOLD_LARGEST);
	return isManFound;
}

/*
Gets the connected components list and the target evaluator (an object created by the classifier file)
and returns a list of the connected components that are recognized as humans.
*/
cc_list CPersonClassifier::classify(cc_list& candidates,TargetEvaluator* te)
{
	static int i=1;
	std::stringstream depthSTR;
	double score;
	Image ccImg;
	cc_list personsList;

	for(cc_list::iterator it = candidates.begin();
		it != candidates.end();
		++it)
	{
		cc c=*it;

		// remove all the cc that are small
		if ((c.rect.get_area() > MIN_PERSON_SIZE) &&
			(c.rect.height() > MIN_PERSON_HEIGHT) && 
			(c.rect.aspect_ratio() < 1))
		{
			// calc the current cc score
			ccImg = c.build_image();//buildImageNoHoles(c);
			
			//debug for old evaloate
			////cout << "cc id = " << c.cc_id << endl;
			//depthSTR << "C:\\temp\\taltestregions\\test" << ++i << ".jpg";
			////CPP::Image& depthImage = Image(new CPPImage(depthSTR.str()));
			////depthImage = convert_to_gray(depthImage);
			//ccImg->save(depthSTR.str());

			//old evaluate
			//score = te->evaluate(ccImg);
			//c.score = score;

			int score;
			if (evaluate(ccImg, score))
			{
				//cout << "cand " << c.cc_id << " score " << c.score << endl;
				c.score = score;
				personsList.push_back(c);

				//save image region before validate.
				/*std::stringstream talSTR , depthSTR;
				talSTR << "C:\\temp\\micah_tal" << i << ".jpg";
				ccImg->save(talSTR.str());
				i++;*/
			}
		}
	}			
	return personsList;
}