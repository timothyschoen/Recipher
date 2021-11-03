#pragma once

#include <array>
#include "LowpassFilter.h"


template<int buffer_size>
struct Delay
{

    Delay() {
        smoother.set_cutoff(12.0f);
        smoother.set_q(1.0f / sqrt(2.0f));
    }
    std::array<float, buffer_size> delay_buffer;
    int write_pos = 0;
    float delay_samples = 500;
    
    float modulation = 0.0f;

    float feedback = 0.0f;

    std::vector<float> input_buffer = std::vector<float>(BLOCK_SIZE);

    void process(const std::vector<float>& input, std::vector<float>& output) {
        
        std::copy(input.begin(), input.end(), input_buffer.begin());
        for(int n = 0; n < input.size(); n++) {
            float final_delay = std::clamp(delay_samples + modulation, 0.0f, (float)buffer_size);
            final_delay = std::max(smoother.process_sample(final_delay), 1.0f);
            
            float delay_frac = final_delay - (int)final_delay;
            
            // Read with linear interpolation
            //output[n] = map(delay_frac, delay_buffer[wrap(write_pos + final_delay)], delay_buffer[wrap((write_pos + final_delay) + 1.0f)]);
            
            output[n] = delay_buffer[wrap(write_pos + final_delay)];
            
            delay_buffer[write_pos] = (input_buffer[n] + output[n]) * feedback;
            
            output[n] += input_buffer[n];
            
            write_pos++;
            write_pos = wrap(write_pos);
        }
    }

    int wrap(int to_wrap) {
        if(to_wrap >= buffer_size) to_wrap -= buffer_size;
        if(to_wrap < 0) to_wrap += buffer_size;
        return to_wrap;
    }


    void set_feedback(float fb) {
        feedback = fb;
    }

    void set_delay_samples(float delay_samps) {
        delay_samples = delay_samps;
    }

    
    void apply_modulation(float mod) {
        modulation = mod;
        
    }
    
    // Linear interpolation
    template <typename Type>
    constexpr Type map (Type value0To1, Type targetRangeMin, Type targetRangeMax)
    {
        return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
    }
    
    LowpassFilter<2> smoother;
};
