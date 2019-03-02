#include <math.h>
#include "mathutils.h"

double evToLux(double ev) {
    return pow(2, ev) * 2.5;
}

double luxToEV(double lux) {
    return log2(lux/2.5);
}

// Exposure value (EV) calculation from the aperture and exposure time
float calcEV(float aperture, float exptime) {
	return (float)(log(pow(aperture,2))/log(2) + log(1/exptime)/log(2));
}
    
// Exposure time calculation
float calcT(float luxvalue, float aperture, float konst, float isovalue) {
	return (float)(pow(aperture,2)*konst/(luxvalue*isovalue));                  //T = exposure time, in seconds
 }

// Aperture size calculation
float calcA(float luxvalue, float timevalue, float konst, float isovalue) {
	return (float)(sqrt((luxvalue*isovalue*timevalue)/konst));	
}


