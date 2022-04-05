#define _USE_MATH_DEFINES
#include <math.h>

#pragma once

class LFO {

public:

    // Move phase value to the next sample
    float tick() {
        
        if(frequency != 0)
        {
            phase += frequency / sample_rate;
            if(phase >= 1) phase = phase - 1;
        }
        
        
        // Mix between shapes
        int first_shape = shape;
        int second_shape = shape == 3 ? 3 : shape + 1;
        float mix = shape - first_shape;
        
        float first_value = (this->*shapePointers[first_shape])();
        float second_value = (this->*shapePointers[second_shape])();
        
        return (first_value + mix * (second_value - first_value)) * 0.5f;
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

    // fast sine approximation
    float sine() {
        auto x = phase * twoPI;
        auto x2 = x * x;
        auto numerator = -x * (-11511339840 + x2 * (1640635920 + x2 * (-52785432 + x2 * 479249)));
        auto denominator = 11511339840 + x2 * (277920720 + x2 * (3177720 + x2 * 18361));
        return numerator / denominator;
    }
    
    float sawtooth() {
        return (1-phase)-0.5;
    }
    
    float square() {
        float sample;
        if(phase >= 0.5) sample = 0.7; // 1 and -1 makes it significantly louder than the sine and saw
        else sample = -0.7;
        return sample;
    }
    
    float triangle() {
        float sample = -1.0 + (2.0 * phase);
        sample = 0.9 * (fabs(sample) - 0.5);
        return sample;
    }

    //Pointers to the waveshape generators
    float (LFO::*shapePointers[4])() = {&LFO::sine, &LFO::triangle, &LFO::square, &LFO::sawtooth};

};
