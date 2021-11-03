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

#ifdef JUCE_API
        AudioFile<float> in_file;
        in_file.load("/Users/timschoen/Documents/Media/TestAudio/Solo2.wav");
        auto sample_input = in_file.samples[0];
        //phase_vocoder.reset(new PhaseVocoder(sampleRate, samplesPerBlock));
        //phase_vocoder->set_input_sample(sample_input);
#endif


        lfo.set_frequency(3.0f);
        lfo.set_shape(0);
    }

    void process (float* input, int num_samples)  {

        //audio_lock.lock();

        // LFO at control rate...
        float lfo_value = lfo.tick();
        //for(int n = 0; n < num_samples; n++) lfo.tick();

        if(realtime_input) {
            std::copy(input, input + num_samples, input_buffer.begin());
        }
        else {
            // Create white noise
            static auto generator = std::default_random_engine();  // Generates random integers
            static auto distribution = std::uniform_real_distribution<float>(-0.999, +0.999);
            auto gen = []() {
                return distribution(generator);
            };

            // fill with random numbers
            std::generate(input_buffer.begin(), input_buffer.end(), gen);
        
            //phase_vocoder->process(input, input_buffer.data(), num_samples);
        }

        if(lfo_destination == 1) {
            lpf.apply_modulation(lfo_value * lfo_depth * 200);
        }
        if(lfo_destination == 2) {
            delay_line.apply_modulation(lfo_value * lfo_depth * 200);
        }
        
        filter_synth.process(input_buffer);

        lpf.process(input_buffer, input_buffer);

        delay_line.process(input_buffer, input_buffer);
        
        std::copy(input_buffer.begin(), input_buffer.begin() + num_samples, input);
        std::fill(input_buffer.begin(), input_buffer.end(), 0.0f);

        for(int n = 0; n < num_samples; n++) input[n] *= volume;
    }


    void note_on(int note, float velocity, int position) {
        //filter_synth.note_on({position, 1, note, velocity});
    }

    void note_off(int note, int position) {
       // filter_synth.post_midi_message({position, 0, note, 0});
    }

    void set_sample_speed(float stretch) {
        //audio_lock.lock();
        //phase_vocoder->set_stretch(stretch);
        //audio_lock.unlock();
    }

    void set_lfo_freq(float new_freq) {
        //audio_lock.lock();
        lfo.set_frequency(new_freq);
        //audio_lock.unlock();
    }

    void set_lfo_depth(float new_depth) {
        //audio_lock.lock();
        lfo_depth = new_depth;
        //audio_lock.unlock();
    }

    void set_delay_time(int samples) {
        //audio_lock.lock();
        delay_line.set_delay_samples(samples);
        //audio_lock.unlock();
    }

    void set_delay_fb(float fb) {
        //audio_lock.lock();
        delay_line.set_feedback(fb);
        //audio_lock.unlock();
    }
    
    void set_volume(float lvl) { volume = lvl; }

    FilterSynth filter_synth;

    Delay<11025> delay_line;

    LowpassFilter<2> lpf;

    Oscillator lfo;

    float lfo_depth = 0.2;
    int lfo_destination = 1;
    
    float volume = 0.6f;

   // std::mutex audio_lock;

private:

    std::vector<float> input_buffer;
    //float input_buffer[BUFFER_SIZE];
};
