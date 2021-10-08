#include "PhaseVocoder.h"
#include <math.h>


PhaseVocoder::PhaseVocoder(float sample_rate, int buffer_size)
:  buffer_size(buffer_size), sample_rate(sample_rate), window_count(buffer_size / hop_size) {

}

void PhaseVocoder::hanning_window(std::vector<float>& samples) {
    for (int i = 0; i < samples.size(); i++) {
        const auto progress = (i - 1.0) / (samples.size() - 1.0);
        samples[i] *= 0.5 - 0.5 * cos(2.0 * M_PI * progress);
    }
}

void PhaseVocoder::process(float* output, int num_samples) {

    if(stretch == 1.0f) {
        for (int i = 0; i < num_samples; i++) {
            output[i] = sample_buffer[sample_position];
            sample_position++;
            sample_position %= sample_buffer.size();
        }
        return;
    }

    for (int window = 0; window < window_count; window++) {
        // Move input signal into buffer to supply to aubio
        int offset = (window * hop_size);

        // Read two buffers from the input sample, spaced apart by hop_size
        for (int i = 0; i < window_size; i++) {
            int current_idx = (int)(sample_position + i + hop_size) % sample_buffer.size();
            int prev_idx = (int)(sample_position + i) % sample_buffer.size();

            current_window[i] = sample_buffer[current_idx];
            previous_window[i] = sample_buffer[prev_idx];
        }

        // Increment sample position
        sample_position += hop_size * stretch;

        if(sample_position >= sample_buffer.size()) sample_position -= sample_buffer.size();
        if(sample_position < 0) sample_position += sample_buffer.size();

        // Apply windowing
        hanning_window(current_window);
        hanning_window(previous_window);

        // Forward FFT
        kiss_fftr(fft, current_window.data(), (kiss_fft_cpx*)current_frame.data());
        kiss_fftr(fft, previous_window.data(), (kiss_fft_cpx*)previous_frame.data());

        for(int bin = 0; bin < window_size / 2; bin++) {
            float real = current_frame[bin].real();
            float imag = current_frame[bin].imag();

            // Phase and amp from current frame
            float phase = atan2_approx(imag, real);
            float amp = sqrt(real * real + imag * imag) / window_size;

            // Phase from previous frame
            float last_phase = atan2_approx(previous_frame[bin].imag(), previous_frame[bin].real());

            // Add phase difference between two frames to the running phase
            running_phase[bin] = fmod(running_phase[bin] + (phase - last_phase), 2.0f * M_PI);

            // Polar to carthesian using running phase and current amplitude
            current_frame[bin].imag(sin(running_phase[bin]) * amp);
            current_frame[bin].real(cos(running_phase[bin]) * amp);


        }

        // Inverse FFT
        kiss_fftri(ifft, (kiss_fft_cpx*)current_frame.data(), current_window.data());

        // Write samples to output and overlap buffer
        for (int i = 0; i < window_size; i++) {
            int idx = offset + i;
            if((offset + i) >= num_samples) {
                next_overlap[idx - num_samples] += (current_window[i] / overlap_amount);
                continue;
            }

            output[idx] += current_window[i] / overlap_amount;
        }
    }

    // Add overlap to output
    // Move all the overlap larger than num_samples back by num_samples
    // This will only work if hop_size < num_samples, currently that's always the case
    for(int n = 0; n < num_samples; n++) {
        output[n] += overlap[n];

        overlap[n] = next_overlap[n] + overlap[n + num_samples];
        overlap[n + num_samples] = next_overlap[n + num_samples];
    }




    std::fill(next_overlap.begin(), next_overlap.end(), 0.0f);
}


float PhaseVocoder::atan2_approx(float y, float x) {
    float abs_y = std::fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
    float r = (x - std::copysign(abs_y, x)) / (abs_y + std::fabs(x));
    float angle = M_PI/2.f - std::copysign(M_PI/4.f, x);

    angle += (0.1963f * r * r - 0.9817f) * r;
    return std::copysign(angle, y);
}
