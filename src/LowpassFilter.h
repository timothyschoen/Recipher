#pragma once

#include <vector>
#include <array>
#define _USE_MATH_DEFINES
#include <math.h>
#include "SynthVoice.h"
#include "Envelope.h"


template<int order>
struct LowpassFilter
{

    LowpassFilter() {
        set_cutoff(1000);
        set_q(1.0f / sqrt(2.0f));
    }

    void process(const std::vector<float>& input, std::vector<float>& output) {
        const int num_channels = 1;
        int num_samples = input.size();

        for(int n = 0; n < num_samples; n++) {

            float in_sample = input[n];
            float& out_sample = output[n];

            for(int o = 0; o < order; o++) {

                auto yHP = h * (in_sample - s1[o] * (g + R2) - s2[o]);
                auto yBP = yHP * g + s1[o];

                auto yLP = yBP * g + s2[o];

                s1[o]      = yHP * g + yBP;
                s2[o]      = yBP * g + yLP;

                in_sample = yLP;
            }

            out_sample = in_sample;
        }
    }

    void apply_modulation(float value) {
        g = std::tan (M_PI * (cutoff + value) / sample_rate);
        h = 1.0f / (1.0f + R2 * g + g * g);
    }

    void set_cutoff(float cutoff_freq) {
        cutoff = cutoff_freq;
        g = std::tan (M_PI * cutoff_freq / sample_rate);
        h = 1.0f / (1.0f + R2 * g + g * g);
    }

    void set_q(float q) {
        R2 = 1.0f / q;
        h  = 1.0f / (1.0f + R2 * g + g * g);
        gain = R2;
    }

    float g, h, R2;
    std::array<float, order> s1, s2;

    float gain = 1.0f;
    float sample_rate      = 44100.0;
    float cutoff = 1000.0f;
    float resonance       = 1.0f / std::sqrt (2.0f);

};
