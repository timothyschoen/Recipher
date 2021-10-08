#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <complex>
//#include <atomic>
#include "fft/kiss_fftr.h"

class PhaseVocoder {
public:
    PhaseVocoder(float sample_sate, int buffer_size);

    // Hanning window function
    void hanning_window(std::vector<float>& samples);

    // Get output samples
    void process(float *input, int num_samples);

    // Sets input sample which will be looped at variable speed
    void set_input_sample(std::vector<float>& samples) {
        sample_buffer = samples;
    }

    // Set time stretch factor
    void set_stretch(float stretch_amt) {
        stretch = stretch_amt;
    }

    float atan2_approx(float y, float x);

private:

    std::vector<float> sample_buffer;

    int buffer_size;
    int sample_position = 0; // Read position for input sample
    float sample_rate;

    // Size of window for phase vocoder
    static constexpr int window_size = 2048;

    static constexpr int overlap_amount = 8;

    // Number of samples between start of one phase vocoder window and next
    static constexpr int hop_size = window_size / overlap_amount;


    std::vector<float> running_phase = std::vector<float>(window_size, 0.0f);

    // Time-domain windows spaces apart by a hop_size
    std::vector<float> current_window = std::vector<float>(window_size, 0.0f);
    std::vector<float> previous_window = std::vector<float>(window_size, 0.0f);

    // Frequency-domain windows spaces apart by a hop_size
    std::vector<std::complex<float>> current_frame = std::vector<std::complex<float>>(window_size, 0.0f);
    std::vector<std::complex<float>> previous_frame = std::vector<std::complex<float>>(window_size, 0.0f);

    // Overlap buffers for Overlap-add algorithm
    std::vector<float> overlap = std::vector<float>(window_size * 2, 0.0f);
    std::vector<float> next_overlap = std::vector<float>(window_size * 2, 0.0f);

    kiss_fftr_cfg fft = kiss_fftr_alloc(window_size, false, 0, 0);
    kiss_fftr_cfg ifft = kiss_fftr_alloc(window_size, true, 0, 0);

    // Number of windows per block
    int window_count;

    //std::atomic<float> stretch = 0.5f;
    float stretch = 0.5f;
};
