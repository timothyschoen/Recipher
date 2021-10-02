#pragma once

#include <JuceHeader.h>
#include "Envelope.h"

enum class Shape: int
{
    Sine,
    Square,
    Saw,
    Triangle,
    NumShapes
};

struct BandpassFilter
{
    
    std::vector<std::vector<float>> shape_harmonics;
    
    BandpassFilter() {
        shape_harmonics.resize((int)Shape::NumShapes, std::vector<float>(num_harmonics, 0.0f));
        
        // Initialise harmonics for each shape
        for(int h = 0; h < num_harmonics; h++) {
            shape_harmonics[(int)Shape::Sine][h] = !h;
            shape_harmonics[(int)Shape::Square][h] = (0.66f / (h + 1.0f)) * ((h + 1) & 1);
            shape_harmonics[(int)Shape::Saw][h] = 0.5f / (h + 1.0f);
            
            // triangle is a square wave with the amplitude values squared
            shape_harmonics[(int)Shape::Triangle][h] = 0.66f / (h + 1.0f) * ((h + 1) & 1);
            shape_harmonics[(int)Shape::Triangle][h] *= shape_harmonics[(int)Shape::Triangle][h];
            
            
            
            svf_1[h].setType(dsp::StateVariableTPTFilterType::bandpass);
            svf_2[h].setType(dsp::StateVariableTPTFilterType::bandpass);
        }
        
        set_q(6.05);
        current_harmonics = shape_harmonics[(int)Shape::Sine].data();
    }
    
    void note_on(float freq, float velocity) {
        enabled = true;
        
        envelope.noteOn(velocity);
        
        for(int h = 0; h < num_harmonics; h++) {
            float frequency = freq * (h + 1.0f);
            if(frequency > 20000) break;
            
            svf_1[h].setCutoffFrequency(frequency);
            svf_2[h].setCutoffFrequency(frequency);
        }
    }
    
    void note_off() {
        
        envelope.noteOff();
        enabled = false;
    }
    
    void set_shape(Shape shape) {
        current_shape = shape;
        current_harmonics = shape_harmonics[(int)shape].data();
    }
    
    void set_q(float q) {
        for(int h = 0; h < num_harmonics; h++) {
            svf_1[h].setResonance(std::clamp(q, 0.0f, 20.0f));
            svf_2[h].setResonance(std::clamp(q, 0.0f, 20.0f));
        }
    }
    
    void prepare(dsp::ProcessSpec& spec) {
        for(int h = 0; h < num_harmonics; h++) {
            svf_1[h].prepare(spec);
            svf_2[h].prepare(spec);
        }
        
        temp_buffer = AudioBuffer<float>(spec.numChannels, spec.maximumBlockSize);
        temp_output = AudioBuffer<float>(spec.numChannels, spec.maximumBlockSize);
    }
    
    void process(dsp::AudioBlock<float> input, dsp::AudioBlock<float> output) {
       

        for(int n = 0; n < input.getNumSamples(); n++) {
            float volume = envelope.tick();
            
            if(volume == 0.0f) continue;
            
            for(int ch = 0; ch < input.getNumChannels(); ch++) {
                const auto* in_ptr = input.getChannelPointer(ch);
                auto* out_ptr = output.getChannelPointer(ch);
  
                for(int h = 0; h < num_harmonics; h++) {
                    if(current_harmonics && current_harmonics[h]) {
                        float filtered = svf_1[h].processSample(ch, in_ptr[n]);
                        filtered = svf_2[h].processSample(ch, filtered);
                        out_ptr[n] += filtered * volume;
                    }
                }

            }
        }
        
    }
    
    
    float* current_harmonics = nullptr;
    Shape current_shape = Shape::Sine;
    
    bool enabled = false;
    
    AudioBuffer<float> temp_buffer, temp_output;
    
    static constexpr int num_harmonics = 12;
    
    Envelope envelope = Envelope(500, 200, 0.4, 1000.0f, 44100.0f);
    
    dsp::StateVariableTPTFilter<float> svf_1[num_harmonics];
    dsp::StateVariableTPTFilter<float> svf_2[num_harmonics];
    
    
};
