#define _USE_MATH_DEFINES
#include <math.h>

#pragma once


class Oscillator {

    // Constructor Class
public:

    // Move phase value to the next sample
    float tick();

    void set_shape(float shp);

    void set_frequency(float freq);

    void set_sample_rate(float sr);

private:
    float shape = 0;

    float frequency = 1.0f;
    float sample_rate = 44100.0f;
    float phase = 0.0f;
    float twoPI = 2.0f * M_PI;

    float sine();
    float sawtooth();
    float square();
    float triangle();

    //Pointers to the waveshape generators
    float (Oscillator::*shapePointers[4])() = {&Oscillator::sine, &Oscillator::triangle, &Oscillator::square, &Oscillator::sawtooth};

};
