if (personFound  && target != NULL)//(cls.classify(s)))
{
	Vec2D cg = calculate_centroid(target);
	cg.x += x;
	cg.y += y;
	targets.push_back(TargetPosition(cg));
	images.push_back(target);
	filtered = filtered*alpha + (1 - alpha);
}
else
{
	targets.clear();
	images.clear();
	filtered = filtered*alpha;
}

int dashes = int(Min(filtered, 0.6) * 72);
if (dashes>0)
{
	std::cout << string(dashes, '-');
	int asts = int(filtered * 72) - dashes;
	if (asts>0)
		std::cout << string(asts, '*');
	std::cout << std::endl;
}
if (filtered<0.2)
{
	if (last_speed_m1>0 && last_speed_m2 > 0){}

	if (counterPersonNotFound > STOP_COUNTER)
	{
		robot->stop();
	}
	std::cout << "Turning.  lastx=" << lastx << std::endl;
	speedx = 0;
	speedy = 0;
	turn = (lastx >= 0 ? 1 : -1);
	if (TURN_COUNTER == counterPersonNotFound)
	{
		play_wav_clip("target lost");
	}
	if (counterPersonNotFound < TURN_COUNTER)
	{
		turn = 0;
	}
}
if (filtered>0.6)
{
	turn = 0;
	flog << GetTickCount() << ' ';
	int i = 0;
	std::vector<double> last_positions;
	int n = 0;
	for (tgt_seq::iterator it = targets.begin(); it != targets.end(); ++it, ++n)
	{
		const Vec2D& p = it->position;
		flog << "fp=" << int(p.x) << ' ';
		last_positions.push_back(p.x);
	}
	std::sort(last_positions.begin(), last_positions.end());
	double posx = 320.0;
	if (!last_positions.empty())
		posx = last_positions[last_positions.size() / 2];
	if (n>4) targets.pop_front();
	double az = fabs(posx - 320.0);
	lastx = int(posx - 320);
	if (az>500)
	{
		int turn = posx>320.0 ? 416 : -416;
		speedy = 0;
		speedx = turn;
		//robot->drive(400,turn);
	}
	else
	{
		speedy = 300;
		speedx = int((posx - 320.0)*0.7);
	}

}
else
{
	speedx = 0;
	speedy = 0;
}
short speed1 = speedy + speedx, speed2 = speedy - speedx;
if (speed1<256) speed1 = 256;
if (speed2<256) speed2 = 256;
if (speed1>512) speed1 = 512;
if (speed2>512) speed2 = 512;
if (turn != 0)
{
	speed1 = 416 * turn;
	speed2 = -416 * turn;
}

//fall detection code calling
//fall_detector.provide_person(target,personFound,x,y, speed1, speed2);



if (speed1 != last_speed_m1 || speed2 != last_speed_m2)
{
	if (speed1>0 && speed2>0 && last_speed_m1 == 0 && last_speed_m2 == 0)
		play_wav_clip("found you");
	unsigned cur_tick = GetTickCount();
	unsigned dt = cur_tick - last_tick;
	if (dt>(500 + extra_time))
	{
		if (speed1*last_speed_m1<0 || speed2*last_speed_m2<0)
		{
			flog << "Stopping and ";
			robot->stop();
			Sleep(500);
			std::cout << "Stop\n";
		}
		int sl = abs(speed1), sr = abs(speed2);
		flog << "Drive:" << speed1 << ',' << speed2;
		if ((sl<256 || sl>512 || sr<256 || sr>512) &&
			(sl != 0 || sr != 0))
			flog << "  Invalid speeds";
		else
		{
			extra_time = 0;
			last_tick = GetTickCount();
			last_speed_m1 = speed1;
			last_speed_m2 = speed2;
			std::cout << "Drive: " << speed1 << " " << speed2 << std::endl;
			if (filtered < 0.2 && counterPersonNotFound > STOP_COUNTER)
			{
				int newspeed1 = speed1 - (counterPersonNotFound - STOP_COUNTER) * 5;
				int newspeed2 = speed2 - (counterPersonNotFound - STOP_COUNTER) * 5;
				speed1 = (newspeed1 > 0) ? newspeed1 : 0;
				speed2 = (newspeed1 > 0) ? newspeed2 : 0;
			}
			robot->drive(speed1, speed2);
			if (turn != 0)
			{
				Sleep(800);
				robot->stop();
				extra_time = 2700;
				last_speed_m1 = last_speed_m2 = 0;
				last_tick = GetTickCount();
			}
		}
	}
}
flog << std::endl;
//Sleep(50);
  }
  close_wavs_and_play_new_wav("goodbye");
  robot->stop();
  //fall_detector.terminate();
  sampler.terminate();
  Sleep(1000);
  delete robot;
}