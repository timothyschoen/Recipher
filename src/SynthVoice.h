#pragma once

#include <vector>
#include <tuple>
#include <algorithm>
#include "Envelope.h"
#include "Oscillator.h"

enum Shape
{
    Sine,
    Triangle,
    Square,
    Saw,
    NumShapes
};

using FilterState = std::pair<float, float>;


struct SynthVoice
{

    std::vector<std::vector<float>> shape_harmonics;

    SynthVoice();

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

    FilterState svf[cascade][num_harmonics];


    template <typename Type>
    constexpr Type map (Type value0To1, Type targetRangeMin, Type targetRangeMax)
    {
        return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
    }
    
    
    // Filter variables
    float g;
    float h;
    float R2;
    float gain;
};
