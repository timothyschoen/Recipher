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

    void note_on(int note, float velocity);
    void retrigger(float velocity);
    void note_off();

    void set_shape(float shape);
    void set_q(float q);
    void set_sub(float sub_level);


    void process(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples);

    float apply_filter(float input, int c, int hr);

    void clear_filters();

    std::vector<float> current_harmonics;

    float sub_level = 0.0f;


    static constexpr int num_harmonics = 5;
    static constexpr int cascade = 2;

    Envelope envelope = Envelope(50.0f, 100.0f, 0.2f, 40.0f, 44100.0f);

    Oscillator sub_osc;

    FilterState svf[cascade][num_harmonics];


    template <typename Type>
    constexpr Type map (Type value0To1, Type targetRangeMin, Type targetRangeMax)
    {
        return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
    }
    
    float ftom(float freq) { return 69.0f + 12.0f * log2(freq / 440.0f);     };
    float mtof(float midi) { return pow(2, (midi - 69.0f) / 12.0f) * 440.0f; };
    
    int current_note = -1;
    
    // Filter variables
    float g[num_harmonics];
    float h[num_harmonics];
    float R2;
    float gain;
};
