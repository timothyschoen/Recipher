#include "SynthVoice.h"


SynthVoice::SynthVoice() {
    shape_harmonics.resize((int)Shape::NumShapes, std::vector<float>(num_harmonics, 0.0f));
    current_harmonics.resize(num_harmonics);
    
    // Initialise harmonics for each shape
    for(int i = 0; i < num_harmonics; i++) {
        int overtone = i + 1;
        shape_harmonics[(int)Shape::Sine][i] = !i;
        shape_harmonics[(int)Shape::Square][i] = (0.66f / overtone) * (overtone & 1);
        shape_harmonics[(int)Shape::Saw][i] = 0.5f / overtone;
        
        // triangle is a square wave with the amplitude values squared
        shape_harmonics[(int)Shape::Triangle][i] = 0.75f / (overtone) * (overtone & 1);
        shape_harmonics[(int)Shape::Triangle][i] *= shape_harmonics[(int)Shape::Triangle][i];
    }
    
    set_q(6.05);
    set_shape(0.0f);
}

void SynthVoice::note_on(float freq, float velocity) {
    
    envelope.note_on(velocity);
    
    for(int i = 0; i < num_harmonics; i++) {
        float frequency = freq * (i + 1.0f);
        if(frequency > 20000) break;
        
        g = std::tan (M_PI * frequency / 44100.0f);
        h = 1.0f / (1.0f + R2 * g + g * g);
    }
}

void SynthVoice::note_off() {
    envelope.note_off();
}



void SynthVoice::set_shape(float shape) {
    shape = std::clamp(shape, 0.0f, 2.99f);
    
    int low_shape = shape;
    int high_shape = low_shape + 1;
    float distance = shape - low_shape;
    
    for(int h = 0; h < num_harmonics; h++) {
        current_harmonics[h] = map(distance, shape_harmonics[low_shape][h], shape_harmonics[high_shape][h]);
    }
    
    //current_harmonics = shape_harmonics[(int)shape].data();
}

void SynthVoice::set_q(float q) {
    q = std::clamp(q, 0.1f, 20.0f);
    
    for(int i = 0; i < num_harmonics; i++) {
        // Update filter variables
        R2 = 1.0f / q;
        h  = 1.0f / (1.0f + R2 * g + g * g);
        gain = R2;
    }
}

void SynthVoice::set_sub(float sub) {
    sub_level = sub;
}


void SynthVoice::process(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples) {
    
    for(int n = start_sample; n < start_sample + num_samples; n++) {
        float volume = envelope.tick();
        
        float in_sample = input[n];
        float& out_sample = output[n];
        
        if(volume == 0.0f)  {
            clear_filters();
            continue;
        }
        
        float sub = sub_osc.tick() * sub_level * volume * 0.1f;
        
        out_sample += sub;
        
        for(int h = 0; h < num_harmonics; h++) {
            if(current_harmonics[h]) {
                // Apply 2 cascaded filters
                float filtered = in_sample;
                
                for(int c = 0; c < cascade; c++) {
                    filtered = apply_filter(filtered, svf[c][h]);
                }
                
                // Apply clipping distortion
                out_sample += tanh(filtered * volume * current_harmonics[h]);
            }
            
        }
    }
    
}

void SynthVoice::clear_filters() {
    
    for(int i = 0; i < num_harmonics; i++) {
        auto& [s1, s2] = svf[0][i];
        
        s1 = 0.0f;
        s2 = 0.0f;
        
        // Apply to cascaded filter
        for(int c = 0; c < cascade; c++) {
            svf[c][i] = svf[0][i];
        }
        
    }
    
    
}
float SynthVoice::apply_filter(float input, FilterState& state) {
    
    /*
     An IIR filter band-pass filter with 12 dB of attenuation per octave, using a TPT structure, designed
     for fast modulation (see Vadim Zavalishin's documentation about TPT
     structures for more information). Its behaviour is based on the analog
     state variable filter circuit.
     */
    
    auto& [s1, s2] = state;
    
    auto yHP = h * (input - s1 * (g + R2) - s2);
    auto yBP = yHP * g + s1;
    
    s1      = yHP * g + yBP;
    s2      = yBP * g + (yBP * g + s2);
    
    return yBP * gain;
}
