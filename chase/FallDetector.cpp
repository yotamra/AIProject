#include "FallDetector.h"

#define NUM_COLORS 256


void FallDetector::print_person_location(Image& depth_image, Image& person, int person_x,int person_y, int center_x, int center_y, bool history, unsigned long long history_secs, unsigned long long history_micro)
{
  int w = ((CPPImage*)person.get())->get_width() * 2;
  int h = ((CPPImage*)person.get())->get_height() * 2;

  IplImage* img = ((CPPImage*)depth_image.get())->get();
  for(int j=0; j<img->width; j++)
  {
    setVal(depth_image,person_y,j,255);
  }
  for(int j=0; j<img->width; j++)
  {
    setVal(depth_image,person_y+ h,j,255);
  }
  for(int i=0; i<img->height; i++)
  {
    setVal(depth_image,i,person_x,255);
  }
  for(int i=0; i<img->height; i++)
  {
    setVal(depth_image,i,person_x +w,255);
  }
  for(int j=0; j<img->width; j++)
  {
    setVal(depth_image,(int)center_y,j,150);
  }
  for(int i=0; i<img->height; i++)
  {
    setVal(depth_image,i,(int)center_x,150);
  }
  unsigned tc=GetTickCount();
  unsigned long long secs = time(NULL);
  if (history == false)
  {
    //{
    //  std::ostringstream os;
    //  os << "pics\\" << secs << "_" << tc << "_person" << ".png";
    //  person->save(os.str());
    //}
    {
      std::ostringstream os;
      os << "pics\\" << secs << "_" << tc << "_depth" << ".png";
      depth_image->save(os.str());
    }
  }
  else
  {
    {
      std::ostringstream os;
      os << "pics\\" << history_secs << "_" << history_micro << "_person_history" << ".png";
      person->save(os.str());
    }
    {
      std::ostringstream os;
      os << "pics\\" << history_secs << "_" << history_micro << "_depth_history" << ".png";
      depth_image->save(os.str());
    }
  }
}

void FallDetector::print_person_location(boost::circular_buffer<History> history)
{
  for (unsigned int i=0; i < history.size(); ++i)
  {
    print_person_location(history[i].depth_image, history[i].person, history[i].person_x, 
      history[i].person_y, history[i].center_x , history[i].center_y, true, history[i].secs, history[i].micro);
  }
}

/*
Uses k-means with k=2
returns the color under which all colors should be considered part of the person
*/
int find_2_means_color(int * histogram)
{
  int low_center, high_center;
  for (int i = 0; i < NUM_COLORS ; ++i)
  {
    if (histogram[i] > 0)
    {
      low_center = i;
      break;
    }
  }
  for (int i = NUM_COLORS -1; i >=0 ; --i)
  {
    if (histogram[i] > 0)
    {
      high_center = i;
      break;
    }
  }
  int center;
  for (int i=0 ; i < 100 ; ++i)
  {
    center = (high_center + low_center) / 2;
    low_center = 0;
    high_center = 0;
    int low_num_pixels = 0, high_num_pixels = 0;
    for (int j=0; j <center; ++j)
    {
      low_center+=histogram[j]*j;
      low_num_pixels+=histogram[j];
    }
    for (int j=center; j < NUM_COLORS; ++j)
    {
      high_center+=histogram[j]*j; 
      high_num_pixels+=histogram[j];
    }
    if (low_num_pixels == 0) 
    {
      low_num_pixels = 1;
    }
    if (high_num_pixels == 0)
    {
      high_num_pixels = 1;
    }
    high_center = high_center /high_num_pixels; //weighted average
    low_center = low_center / low_num_pixels; //weighted average
  }

  //because usually the center was too high and unhuman objects were considered as human,
  //we change the center to be closer to the lower center.
  center = (high_center + 3*low_center) / 4;
  return center;
} 

bool FallDetector::detect_fall()
{
  int min_y_loop_index, max_y_loop_index;//the loop indexes when min_y and max_y were discovered
  double max_y = -1;
  double min_y = 2;
  double max_x = -1;
  double min_x = 2;
  for(unsigned int i = 0; i < m_centers_buffer.size() ; ++i)  
  {
    if (m_centers_buffer[i].y < min_y)
    {
      min_y = m_centers_buffer[i].y;
      min_y_loop_index = i;
    }
    if (m_centers_buffer[i].y > max_y)
    {
      max_y = m_centers_buffer[i].y;
      max_y_loop_index = i;
    }
    if (m_centers_buffer[i].x < min_x)
    {
      min_x = m_centers_buffer[i].x;
    }
    if (m_centers_buffer[i].x > max_x)
    {
      max_x = m_centers_buffer[i].x;
    }
  }

  int micros=GetTickCount();
  unsigned long long secs = time(NULL);

  bool falling1, falling2, falling3;
  bool falling = true;

  // להוסיף בשני התנאים שאם יש שיוויון בין שני המשתנים וזה פחות זה יוצא 0 אז להתעלם
  if (max_y != min_y)
  {
	  falling1 = ((max_y - min_y) > 0.1) && (max_y_loop_index > min_y_loop_index); //a fall - fell enough and max height was before the min height
  }
  else
  {
	  falling1 = (max_y_loop_index > min_y_loop_index);
  }
  
  if ((abs(max_x - min_x) > 0.2) && (abs(max_x - min_x) < 0.3) )
  {
	  falling2 = true; //if the person is going left or right, he's not falling
  }
  else
  {
	  falling2 = false;
  }

//  falling2 = falling && (abs(max_x - min_x)<0.3); //if the person is going left or right, he's not falling
//  falling3 = falling && (m_speed1 > 0) && (m_speed2 > 0) && abs(m_speed1 - m_speed2) < 150; //not considered a fall if the robot is turning
 
  
  // To Remove !!!!
  //falling1 = false;
  //falling2 = false;
  // m_log << secs << "." << micros << " detect_fall() falling1: " << falling1 << " falling2: " << falling2 << " falling3: " << falling3 << endl;
  m_log << secs << "." << micros << " falling1, max_y: " << max_y << " min_y: " << min_y << " difference: " << max_y - min_y << endl;
  m_log << secs << "." << micros << " falling1, max_y_loop_index: " << max_y_loop_index << " min_y_loop_index: " << min_y_loop_index << " difference: " << max_y_loop_index - min_y_loop_index << endl;

  m_log << secs << "." << micros << " falling2, max_x: " << max_x << " min_x: " << min_x << " difference: " << max_x - min_x << endl;

  falling = falling1 | falling2;
  
  if (falling1)
  {
    //close_wavs_and_play_new_wav("mayday"); 
    unsigned tc=GetTickCount();
    unsigned long long secs = time(NULL);
#ifdef Debug_Fall_Detect
    {
      std::ostringstream os;
      os << "pics\\" << secs << "_" << tc << "_person" << ".png";
      m_person->save(os.str());
    }
    {
      Image depth_image;
      m_cam->update_depth_frame(depth_image);
      std::ostringstream os;
      os << "pics\\" << secs << "_" << tc << "_depth" << ".png";
      depth_image->save(os.str());
    }
	//send the pause key to stop the robot
	Sleep(1000);
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = '1'; //send the pause key
	input.ki.dwFlags = 0; //sends as key down
	SendInput(1,&input,sizeof(INPUT));

	Sleep(200);

	input.ki.dwFlags = KEYEVENTF_KEYUP; //sends as key up
	SendInput(1,&input,sizeof(INPUT));
#endif // Debug_Fall_Detect

	m_log << secs << "Fell" << endl;
	m_log << secs << "." << micros << " falling1, max_y: " << max_y << " min_y: " << min_y << " difference: " << max_y - min_y << endl;
	m_log << secs << "." << micros << " falling1, max_y_loop_index: " << max_y_loop_index << " min_y_loop_index: " << min_y_loop_index << " difference: " << max_y_loop_index - min_y_loop_index << endl;

	m_log << secs << "." << micros << " falling2, max_x: " << max_x << " min_x: " << min_x << " difference: " << max_x - min_x << endl;

	//print_person_location(m_history); //prints the last 10 pictures
    clear_buffers();

// To Do - Stop The Robot

  }
  return falling1;
}

void FallDetector::update_data()
{

  Image depth_image;
  m_cam->update_depth_frame(depth_image);
  int w = ((CPPImage*)m_person.get())->get_width()*2;
  int h = ((CPPImage*)m_person.get())->get_height()*2;
  int total_w = ((CPPImage*)depth_image.get())->get_width();
  int total_h = ((CPPImage*)depth_image.get())->get_height();

  //calculates center
  Vec2D center(0, 0);

  int   n=0;
  int x_left_border = max(0, m_person_x - w);
  int x_right_border = min(m_person_x + 2*w, total_w);
  for(int i=m_person_y; i<m_person_y +h && i < total_h; ++i)
  {
    byte* row=depth_image->get_row(i);
    for(int j=x_left_border; j<x_right_border && j < total_w; ++j)
    {
      if (row[j] < m_color_border)
      {
        //ignore black pixels, they are just objects that don't reflect light such as mirrors, 
        //distant areas and black objects which are not always near the camera
        if(row[j]==0)
        {
          continue;
        }
        center.x+=j;
        center.y+=i;
        ++n;
      }
    }
  }
  if (n==0)
  {
    m_log<<"***************************************************division by zero "<< m_color_border << endl;//shouldn't happen
  }
  center =(1.0/n)*center; 
  double person_center_y = ((m_person_y + h) - center.y) / h ; //person center of mass y axis as a percentage of total person area
  double person_center_x = (x_right_border - center.x) / (x_right_border - x_left_border); //x axis
  m_centers_buffer.push_front(Vec2D(person_center_x,person_center_y));

//  unsigned long long secs = time(NULL);
//  int micros = GetTickCount();

//  m_log << secs << "." << micros << " update_data() center is at (" << center.x << "," <<  center.y << ") and relatively (" << person_center_x << "," << person_center_y << ")" <<  endl;
  //print_person_location(depth_image,m_person,person_x,person_y,center.x,center.y);
//  History history(depth_image,m_person,person_x,person_y,(int) center.x, (int) center.y, secs , micros);
//  m_history.push_front(history);
}


//constructor
FallDetector::FallDetector(CameraSampler* cam):m_log("FallDetector.log"), m_centers_buffer(10), m_history(10)
{
  m_cam = cam;
  m_timer = FPS_timer("Fall Detector");
  m_person_in_frame = false;
  m_paused = false;
  m_num_lost_person = 0;
  m_color_border=-1; //shows that there is no previuse value for m_color_border to use
  m_Provide_Person_Flag = true;
  m_recently_found_person = false;
  m_log << time(NULL) << " started fall detector" << endl;
}

void FallDetector::pause()
{
  m_paused=true;
  m_log << time(NULL) << " paused" << endl;
}
void FallDetector::resume()
{
  m_paused=false;
  m_log << time(NULL) << " resumed" << endl;
}

void FallDetector::set_person_in_frame_status(bool person_in_frame)
{
	m_person_in_frame = person_in_frame;
}

void FallDetector::provide_person(Image& person, bool person_in_frame, int x, int y,int speed1,int speed2)
{
	if (m_Provide_Person_Flag == true)
	{
		m_Provide_Person_Flag = false;
		Image depth_image;
		m_cam->update_depth_frame(depth_image);
		m_speed1 = speed1;
		m_speed2 = speed2;
		m_log << time(NULL) << " provided person: " << person_in_frame << endl;
		m_log << time(NULL) << " speeds: " << speed1 << " " << speed2 << endl;


		if (person_in_frame == true)
		{
			m_num_lost_person = 0;
			m_person = person;
			m_person_x = x;
			m_person_y = y;
//			int w = ((CPPImage*)m_person.get())->get_width() * 2;
//			int h = ((CPPImage*)m_person.get())->get_height() * 2;
//			int total_w = ((CPPImage*)depth_image.get())->get_width();
//			int total_h = ((CPPImage*)depth_image.get())->get_height();

			int w = ((CPPImage*)person.get())->get_width();
			int h = ((CPPImage*)person.get())->get_height();
			unsigned int total_w = ((CPPImage*)depth_image.get())->get_width();
			unsigned int total_h = ((CPPImage*)depth_image.get())->get_height();


			int histogram[NUM_COLORS];
			int num_pixels = 0;
			//initialize an array in O(1)
			int not_i = 0;
			for (unsigned int i = 0; i < NUM_COLORS; ++i)
			{
				not_i = i;
				histogram[i] = 0;
			}
			//build histogram
			for (int i = m_person_y; i<m_person_y + h && i <total_h; ++i)
			{
				byte* row = depth_image->get_row(i);
				for (int j = m_person_x; j<m_person_x + w && j <total_w; ++j)
				{
					//ignore black pixels, they are just objects that don't reflect light such as mirrors, 
					//distant areas and black objects which are not always near the camera
					if (row[j] == 0)
					{
						continue;
					}

					++histogram[row[j]];
					++num_pixels;
				}
			}

			//to avoid glitches of unhuman objects we base the color border more on history
			if (m_color_border != -1)
			{
				m_color_border = (3 * m_color_border + find_2_means_color(histogram)) / 4;
			}
			else//there is no history for m_color_border
			{
				m_color_border = find_2_means_color(histogram);
			}
			unsigned long long secs = time(NULL);
			int micros = GetTickCount();

			m_log << secs << "." << micros << " provide_person() person is at " << m_person_x << "-" << m_person_x + w << "," << m_person_y << "-" << m_person_y + h << endl;
			m_log << secs << "." << micros << " provide_person() center color is: " << m_color_border << endl;
		}
		else
		{
			m_num_lost_person++;
		}
	}
  
}

void FallDetector::run()
{

    m_timer.sample();

	if (m_person_in_frame == true)
	{
		m_recently_found_person = true;
		update_data();
	}
	else
	{
		if (m_recently_found_person == true)
		{
    		update_data();
			m_recently_found_person = !detect_fall(); //after falling the target is always lost
			m_Provide_Person_Flag = true;
		}
		if ((m_num_lost_person > 1) && (m_recently_found_person == true)) //protect buffers from losing the person in the middle of the fall
		{
			m_recently_found_person = false;
			clear_buffers();
		}
	}
	//m_log << "m_person_in_frame: " << m_person_in_frame << "m_recently_found_person: " << m_recently_found_person << endl;

}

void FallDetector::clear_buffers()
{
  m_centers_buffer.clear();
  m_history.clear();
  m_color_border = -1;//cancel any history of 2-means color when person is lost
  m_log << time(NULL) << " cleared buffers" << endl;
}
