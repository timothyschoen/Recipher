#pragma once

#include <JuceHeader.h>
#include "Envelope.h"

enum class Shape: int
{
    Sine,
    Square,
    Saw,
    Triangle,
    NumShapes
};

using FilterState = std::tuple<float, float, float, float, std::array<float, 2>, std::array<float, 2>>;


struct BandpassFilter
{
    
    std::vector<std::vector<float>> shape_harmonics;
    
    BandpassFilter();
    
    void note_on(float freq, float velocity);
    
    void note_off();
    
    void set_shape(Shape shape);
    
    void set_q(float q);
    
    void process(const std::vector<float>& input, std::vector<float>& output);
    
    float apply_filter(int ch, float input, FilterState& state);
    
    
    float* current_harmonics = nullptr;
    Shape current_shape = Shape::Sine;
    
    bool enabled = false;
    
    static constexpr int num_harmonics = 12;
    
    Envelope envelope = Envelope(500, 200, 0.4, 1000.0f, 44100.0f);
    
    FilterState svf_1[num_harmonics];
    FilterState svf_2[num_harmonics];
};
