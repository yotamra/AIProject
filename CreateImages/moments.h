#ifndef moments_h__
#define moments_h__

#include <image.h>

using namespace CPP;

Vec2D  calculate_centroid(Image img);
double calculate_moment(Image img, const Vec2D& centroid, int p, int q);

#endif // moments_h__
