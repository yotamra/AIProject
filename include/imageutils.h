#ifndef H_IMAGE_UTILS
#define H_IMAGE_UTILS

void save_ppm(const std::string& filename, CPP::Image img);
CPP::Image load_ppm(const std::string& filename);

double get_mean_intensity(CPP::Image image);

void rgb2hsv(const byte* color, double hsv[3]);
void rgb2Lab(const byte* color, double Lab[3]);

void create_histogram(CPP::Image image, int hist[256]);

#endif // H_IMAGE_UTILS
