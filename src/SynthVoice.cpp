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
    
    envelope.on_release = [this](){
        current_note = -1;
    };
    
    set_q(6.05);
    set_shape(0.0f);
}

void SynthVoice::note_on(int note, float velocity) {
    current_note = note;
    float freq = mtof(note);
    
    sub_osc_1.set_frequency(freq / 2.0f);
    sub_osc_2.set_frequency(freq / 4.0f);
    
    envelope.note_on(velocity);
    
    // Update filters
    for(int i = 0; i < num_harmonics; i++) {
        float frequency = freq * (i + 1.0f) * stretch;
        if(frequency > 20000) break;
        
        g[i] = std::tan (M_PI * frequency / 44100.0f);
        h[i] = 1.0f / (1.0f + R2 * g[i] + g[i] * g[i]);
    }
}

void SynthVoice::set_stretch(float stretch_amt)
{
    float freq = mtof(current_note);
    stretch = stretch_amt;
    
    // Update filters
    for(int i = 0; i < num_harmonics; i++) {
        float frequency = freq * (i + 1.0f) * stretch;
        if(frequency > 20000) break;
        
        g[i] = std::tan (M_PI * frequency / 44100.0f);
        h[i] = 1.0f / (1.0f + R2 * g[i] + g[i] * g[i]);
    }
}

void SynthVoice::retrigger(float velocity)
{
    envelope.note_on(velocity);
}

void SynthVoice::note_off() {
    envelope.note_off();
}


void SynthVoice::set_shape(float new_shape) {
    shape = new_shape;
}


void SynthVoice::set_shape_mod(float new_shape_mod)
{
    shape_mod = new_shape_mod;
}

void SynthVoice::set_q(float q) {
    q = std::clamp(q, 0.1f, 20.0f);
    
    for(int i = 0; i < num_harmonics; i++) {
        // Update filter variables
        R2 = 1.0f / q;
        h[i]  = 1.0f / (1.0f + R2 * g[i] + g[i] * g[i]);
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
        
        float sub = sub_osc_1.tick() * sub_level * volume * 0.1f;
        sub += sub_osc_2.tick() * std::max(sub_level - 0.5f, 0.0f) * volume * 0.15f;
        
        out_sample += sub;
        
        // Calculate shape position
        float total_shape = std::clamp(shape + shape_mod, 0.0f, 2.9f);
        int low_shape = total_shape;
        int high_shape = low_shape + 1;
        float distance = total_shape - low_shape;
        
        for(int hr = 0; hr < num_harmonics; hr++) {
            // Calculate volume of current harmonic
            float current_harmonic =  map(distance, shape_harmonics[low_shape][hr], shape_harmonics[high_shape][hr]);
            
            if(current_harmonic) {
                // Apply 2 cascaded filters
                float filtered = in_sample;
                
                for(int c = 0; c < cascade; c++) {
                    filtered = apply_filter(filtered, c, hr);
                }
                
                out_sample += filtered * volume * current_harmonic;
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
float SynthVoice::apply_filter(float input, int c, int hr) {
    
    /*
     An IIR filter band-pass filter with 12 dB of attenuation per octave, using a TPT structure, designed
     for fast modulation (see Vadim Zavalishin's documentation about TPT
     structures for more information). Its behaviour is based on the analog
     state variable filter circuit.
     */
    
    auto& [s1, s2] = svf[c][hr];
    
    auto yHP = h[hr] * (input - s1 * (g[hr] + R2) - s2);
    auto yBP = yHP * g[hr] + s1;
    
    s1      = yHP * g[hr] + yBP;
    s2      = yBP * g[hr] + (yBP * g[hr] + s2);
    
    return yBP * gain;
}
