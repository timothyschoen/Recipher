#include <JuceHeader.h>
#include "BandpassFilter.h"


BandpassFilter::BandpassFilter() {
    shape_harmonics.resize((int)Shape::NumShapes, std::vector<float>(num_harmonics, 0.0f));
    
    // Initialise harmonics for each shape
    for(int h = 0; h < num_harmonics; h++) {
        shape_harmonics[(int)Shape::Sine][h] = !h;
        shape_harmonics[(int)Shape::Square][h] = (1.0f / (h + 1.0f)) * ((h + 1) & 1);
        shape_harmonics[(int)Shape::Saw][h] = 1.0f / (h + 1.0f);
        
        // triangle is a square wave with the amplitude values squared
        shape_harmonics[(int)Shape::Triangle][h] = 1.0f / (h + 1.0f) * ((h + 1) & 1);
        shape_harmonics[(int)Shape::Triangle][h] *= shape_harmonics[(int)Shape::Triangle][h];
    }
    
    set_q(6.05);
    current_harmonics = shape_harmonics[(int)Shape::Sine].data();
}

void BandpassFilter::note_on(float freq, float velocity) {
    enabled = true;
    
    envelope.noteOn(velocity);
    
    for(int i = 0; i < num_harmonics; i++) {
        float frequency = freq * (i + 1.0f);
        if(frequency > 20000) break;
        
        auto& [g, h, R2, gain, s1, s2] = svf_1[i];
        
        g = std::tan (juce::MathConstants<float>::pi * frequency / 44100.0f);
        h = 1.0f / (1.0f + R2 * g + g * g);
        
        svf_2[i] = svf_1[i];
    }
}

void BandpassFilter::note_off() {
    envelope.noteOff();
    enabled = false;
}

void BandpassFilter::set_shape(Shape shape) {
    current_shape = shape;
    current_harmonics = shape_harmonics[(int)shape].data();
}

void BandpassFilter::set_q(float q) {
    for(int i = 0; i < num_harmonics; i++) {
        q = std::clamp(q, 0.0f, 20.0f);
        
        auto& [g, h, R2, gain, s1, s2] = svf_1[i];
        R2 = 1.0f / q;
        h  = 1.0f / (1.0f + R2 * g + g * g);
        gain = R2;
        
        svf_2[i] = svf_1[i];
    }
}

void BandpassFilter::process(const std::vector<float>& input, std::vector<float>& output) {
    
    static int num_channels = 2;

    for(int n = 0; n < input.size() / num_channels; n++) {
        float volume = envelope.tick();
        
        if(volume == 0.0f) continue;
        
        for(int ch = 0; ch < num_channels; ch++) {
            float in_sample = input[n * 2 + ch];
            float& out_sample = output[n * 2 + ch];
            
            for(int h = 0; h < num_harmonics; h++) {
                if(current_harmonics && current_harmonics[h]) {
                    float filtered = apply_filter(ch, in_sample, svf_1[h]);
                    filtered = apply_filter(ch, filtered, svf_2[h]);
                    
                    // Apply clipping distortion
                    out_sample += tanh(filtered * volume);
                }
            }
            
        }
    }
    
}

float BandpassFilter::apply_filter(int ch, float input, FilterState& state) {
    auto& [g, h, R2, gain, s1, s2] = state;

    auto yHP = h * (input - s1[ch] * (g + R2) - s2[ch]);
    auto yBP = yHP * g + s1[ch];
    
    s1[ch]      = yHP * g + yBP;
    s2[ch]      = yBP * g + (yBP * g + s2[ch]);

    return yBP * gain;
}
