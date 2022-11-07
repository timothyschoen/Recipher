#define _USE_MATH_DEFINES
#include <math.h>

#pragma once

class LFO {

public:
    
    LFO(float sr, int block_size) {
        rate = sr / block_size;
    }

    // Move phase value to the next sample
    float tick() {
        
        if(frequency != 0)
        {
            phase += frequency / rate;
            if(phase >= 1) phase = phase - 1;
        }
        
        // Mix between shapes
        int first_shape = shape;
        int second_shape = shape == 3 ? 3 : shape + 1;
        float mix = shape - first_shape;
        
        float first_value = (this->*shapePointers[first_shape])();
        float second_value = (this->*shapePointers[second_shape])();
        
        return (first_value + mix * (second_value - first_value));
    }

    void set_shape(float shp) {
        shape = shp;
    }


    void set_frequency(float freq) {
        frequency = freq;
    }

private:
    float shape = 0;

    float frequency = 1.0f;
    float phase = 0.0f;
    float twoPI = 2.0f * M_PI;

    // fast cosine approximation
    float sine() {
        auto x = phase * twoPI;
        auto x2 = x * x;
        auto numerator = -(-39251520 + x2 * (18471600 + x2 * (-1075032 + 14615 * x2)));
        auto denominator = 39251520 + x2 * (1154160 + x2 * (16632 + x2 * 127));
        return numerator / denominator;
    }
    
    float sawtooth() {
        return (phase * 2.0f) - 1.0f;
    }
    
    float square() {
        float sample;
        if(phase >= 0.5) sample = 1.0f; // 1 and -1 makes it significantly louder than the sine and saw
        else sample = -1.0f;
        return sample;
    }
    
    float triangle() {
        float sample = -1.0 + (2.0 * phase);
        sample = 0.9 * (fabs(sample) - 0.5);
        return sample;
    }

    float rate;
    //Pointers to the waveshape generators
    float (LFO::*shapePointers[4])() = {&LFO::sine, &LFO::square, &LFO::sawtooth, &LFO::triangle};

};
