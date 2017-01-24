#include "CameraSampler.h"


CameraSampler::CameraSampler()
{
  m_timer = FPS_timer("Camera");
  m_color_frame = Image(new CPPImage(640,480,3));
  m_depth_frame = Image(new CPPImage(640, 480, 1));
  m_finished_init=false;
  m_Terminate = false;
  m_recored_image = false;
  TakeSampleFromCamera = false;
  frameNumber = 0;
  image_num = 0;
}

void CameraSampler::wait_for_finished_initializing()
{
  while(!m_finished_init);
}

void CameraSampler::run()
{
	if (TakeSampleFromCamera == true) {
		m_cam = Sensor::create();
	}
	update();

	if (m_recored_image == true) {
		m_color_frame->save("C:\\TempPic\\" + to_string(ULONGLONG(image_num)) + "color"); //every taken picture is recorded
		m_depth_frame->save("C:\\TempPic\\" + to_string(ULONGLONG(image_num)) + "depth"); //every taken picture is recorded
		image_num++;
	}

    m_finished_init = true;
    while(!m_Terminate)
    {
      m_timer.sample();
      update();

	  if (m_recored_image == true) {

		  m_color_frame->save("C:\\TempPic\\" + to_string(ULONGLONG(image_num)) + "color");
		  m_depth_frame->save("C:\\TempPic\\" + to_string(ULONGLONG(image_num)) + "depth");
	  }

      Sleep(50); //sleeps 50 millisecs
    }
    delete m_cam;
}

void CameraSampler::update(Image& color_frame, Image& depth_frame)
{
  SYNCHRONIZED;
  color_frame = m_color_frame;
  depth_frame = m_depth_frame;
}

void CameraSampler::update_color_frame(Image& color_frame)
{
  SYNCHRONIZED;
  color_frame = m_color_frame;
}

void CameraSampler::update_depth_frame(Image& depth_frame)
{
  SYNCHRONIZED;
  depth_frame = m_depth_frame;
}

void CameraSampler::update()
{
  SYNCHRONIZED;

  if (TakeSampleFromCamera == true) {
	  m_color_frame = Image(new CPPImage(640, 480, 3));
	  m_depth_frame = Image(new CPPImage(640, 480, 1));
	  m_cam->update(m_color_frame, m_depth_frame);
  }
  else {
	  string path = "C:\\Benchmarks\\BenchmarkPicturesChairs\\";
	  path += to_string(static_cast<long long>(frameNumber));
	  m_color_frame = Image(new CPPImage(path + "color.jpg"));
	  m_depth_frame = Image(new CPPImage(path + "depth.jpg"));
	  frameNumber++;
  }

}