/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include "Config.h"

#include <random>
#include <vector>
#include <memory>
//#include <mutex>
#include "LowpassFilter.h"
#include "FilterSynth.h"

#ifdef JUCE_API
#include "AudioFile.h"
#endif

#include "Delay.h"
//==============================================================================
/**
*/


struct Processor {
    
    bool realtime_input = false;

    void prepare (double sampleRate, int samplesPerBlock)  {
        filter_synth.prepare(sampleRate, samplesPerBlock);

        delay_line.set_delay_samples(22050);

        input_buffer.resize(samplesPerBlock);
        noise_buffer.resize(samplesPerBlock);

#ifdef JUCE_API
        AudioFile<float> in_file;
        in_file.load("/Users/timschoen/Documents/Media/TestAudio/Solo2.wav");
        auto sample_input = in_file.samples[0];
        //phase_vocoder.reset(new PhaseVocoder(sampleRate, samplesPerBlock));
        //phase_vocoder->set_input_sample(sample_input);
#endif

        lfo.set_sample_rate(sampleRate / samplesPerBlock);
        lfo.set_frequency(3.0f);
        lfo.set_shape(0);
    }

    void process (float* input, int num_samples)  {
        // LFO at control rate...
        float lfo_value = lfo.tick();
        //for(int n = 0; n < num_samples; n++) lfo.tick();

        std::copy(input, input + num_samples, input_buffer.begin());

        // Create white noise
        static auto generator = std::default_random_engine();  // Generates random integers
        static auto distribution = std::uniform_real_distribution<float>(-0.999, +0.999);
        auto gen = []() {
            return distribution(generator);
        };

        // fill with random numbers
        std::generate(noise_buffer.begin(), noise_buffer.end(), gen);
        for(int n = 0; n < num_samples; n++) {
            input_buffer[n] *= input_gain;
            input_buffer[n] = input_buffer[n] + noise_level * (noise_buffer[n] - input_buffer[n]);
        }
        
        // Modulator functions
        std::function<void(float)> mod_targets[3] = {
            [this](float mod){
                filter_synth.set_shape_mod(mod * 3.0f);
            },
            [this](float mod){
                lpf.apply_modulation(std::clamp(mod * 8000.0f, 1.0f, 20000.0f));
        },  [this](float mod){
            delay_line.apply_modulation(mod * 5000.0f);
        }};
        
        // Split modulator between sources when the knob is inbetween positions
        int first_target = lfo_destination;
        int second_target = first_target == 2 ? 0 : lfo_destination + 1;
        float diff = lfo_destination - first_target;

        float mod_1 = lfo_value * lfo_depth * (1.0f - diff);
        float mod_2 = lfo_value * lfo_depth * diff;
        
        mod_targets[first_target](mod_1);
        mod_targets[second_target](mod_2);
        
        filter_synth.process(input_buffer);

        lpf.process(input_buffer, input_buffer);

        delay_line.process(input_buffer, input_buffer);
        
        std::copy(input_buffer.begin(), input_buffer.begin() + num_samples, input);
        std::fill(input_buffer.begin(), input_buffer.end(), 0.0f);

        // Apply distortion and compensate volume
        for(int n = 0; n < num_samples; n++) input[n] = (tanh(input[n] * drive) * (1.0f / sqrt(drive))) * volume;
    }

    void set_lfo_freq(float new_freq) {
        lfo.set_frequency(new_freq);
    }

    void set_lfo_depth(float new_depth) {
        lfo_depth = new_depth;
    }
    
    void set_lfo_shape(int shape) {
        lfo.set_shape(shape);
    }
    void set_lfo_dest(float destination) {
        lfo_destination = destination;
    }

    void set_delay_time(int samples) {
        delay_line.set_delay_samples(samples);
    }

    void set_delay_fb(float fb) {
        delay_line.set_feedback(fb);
    }
    
    void set_volume(float lvl) {
        volume = lvl;
        
    }
    
    void set_mix(float mix) {
        noise_level = mix;
    }
    
    void set_gain(float gain) {
        input_gain = gain;
    }
    
    void set_drive(float gain) {
        drive = gain;
    }
    
    FilterSynth filter_synth;

    Delay<11025> delay_line;

    LowpassFilter<2> lpf;

    Oscillator lfo;

    float lfo_depth = 0.2;
    float lfo_destination = 1;
    
    float volume = 0.6f;
    float noise_level = 0.2;
    float drive = 1.0f;
    float input_gain = 1.0f;

private:

    std::vector<float> input_buffer, noise_buffer;
};
