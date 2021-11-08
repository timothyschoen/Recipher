#include <cmath>
#include "Oscillator.h"

void Oscillator::set_sample_rate(float sr)
{
    sample_rate = sr;
}

void Oscillator::set_frequency(float freq) {
    frequency = freq;
}

// Calculation for sine
float Oscillator::sine() {
    return sin(phase * twoPI);
}
// Calculation for saw
float Oscillator::sawtooth() {
    return (1-phase)-0.5;
}
float Oscillator::square() {
    float sample;
    if(phase >= 0.5) sample = 0.7; // 1 and -1 makes it significantly louder than the sine and saw
    else sample = -0.7;
    return sample;
}
float Oscillator::triangle() {
    float sample = -1.0 + (2.0 * phase);
    sample = 0.9 * (fabs(sample) - 0.5);
    return sample;
}

// Move phase value to the next sample
float Oscillator::tick() {
    phase += frequency / sample_rate;
    if(phase >= 1) phase = phase - 1;
    
    int first_shape = shape;
    int second_shape = shape == 3 ? 3 : shape + 1;
    float mix = shape - first_shape;
    
    float first_value = (this->*shapePointers[first_shape])();
    float second_value = (this->*shapePointers[second_shape])();
    
    return (first_value + mix * (second_value - first_value)) * 0.5f;
}

// Move phase value to the next sample
void Oscillator::set_shape(float shp) {
    shape = shp;
}
