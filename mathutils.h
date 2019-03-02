#ifndef __MATHUTILS_H__
#define __MATHUTILS_H__

#include <math.h>

#define log2(a) log10(a)/log10(2)

double evToLux(double ev);
double luxToEV(double lux);
// Exposure value (EV) calculation from the aperture and exposure time
float calcEV(float aperture, float exptime);
// Exposure time calculation
float calcT(float luxvalue, float aperture, float konst, float isovalue);
// Aperture size calculation
float calcA(float luxvalue, float timevalue, float konst, float isovalue);
	
#endif
