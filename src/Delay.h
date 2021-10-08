#pragma once

#include <array>
#include "SynthVoice.h"
#include "Envelope.h"


template<int buffer_size>
struct Delay
{

    std::array<float, buffer_size> delay_buffer;
    int write_pos = 0;
    int delay_samples = 1;

    float feedback = 0.5;

    // TODO: Fix this heap allocation!
    void process(const std::vector<float> input, std::vector<float>& output) {
        for(int n = 0; n < input.size(); n++) {
            

            output[n] = delay_buffer[write_pos];
            delay_buffer[write_pos] = 0;
            
            delay_buffer[wrap(write_pos + delay_samples)] = input[n] + (output[n] * feedback);
            
            // Do interpolation!
            
            output[n] += input[n];
            
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

    void set_delay_samples(int delay_samps) {
        delay_samples = delay_samps;
    }

};
