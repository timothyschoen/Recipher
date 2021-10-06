#pragma once

#include <JuceHeader.h>
#include "Envelope.h"
#include "Oscillator.h"

enum class Shape: int
{
    Sine,
    Triangle,
    Square,
    Saw,
    NumShapes
};

using FilterState = std::tuple<float, float, float, float, float, float>;


struct BandpassFilter
{
    
    std::vector<std::vector<float>> shape_harmonics;
    
    BandpassFilter();
    
    void note_on(float freq, float velocity);
    
    void note_off();
    
    void set_shape(float shape);
    void set_q(float q);
    void set_sub(float sub_level);

    
    void process(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples);
    
    float apply_filter(float input, FilterState& state);
    
    void clear_filters();
    
    std::vector<float> current_harmonics;
    Shape current_shape = Shape::Sine;
    
    float sub_level = 0.0f;
    float feedback = 0.5f;
    
    static constexpr int num_harmonics = 6;
    static constexpr int cascade = 6;
    
    Envelope envelope = Envelope(50, 1000, 0.2, 1000.0f, 44100.0f);
    
    Oscillator sub_osc;
    
    FilterState svf_1[cascade][num_harmonics];
};
