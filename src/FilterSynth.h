#pragma once

#include <tuple>
#include <algorithm>
#include <vector>
//#include <mutex>
#define _USE_MATH_DEFINES
#include <math.h>
#include "SynthVoice.h"


using MidiMess = std::tuple<int, int, int, int>;



struct FilterSynth
{
    FilterSynth();

    void prepare(float sample_rate, int block_size);

    void process(std::vector<float>& input);

    void note_on(int midi_note, int velocity);
    void note_off(int midi_note);

    void post_midi_message(MidiMess message) {
        //message_queue.push_back(message);
    };
    
    void remove_duplicates(std::vector<std::pair<int, int>>& v) {
        auto end = v.end();
            for (auto it = v.begin(); it != end; ++it) {
                end = std::remove_if(it + 1, end,
                    [&it](std::pair<int, int> x){
                        return x.second == it->second;
                });
            }
        
            v.erase(end, v.end());
    }


    void process_filters(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples);

    void set_q(float q);
    void set_shape(float shape);

    void set_attack(float attack);
    void set_decay(float decay);
    void set_sustain(float sustain);
    void set_release(float release);

    
    void set_sub(float sub_level);
    

    std::vector<MidiMess> message_queue;

    // Max num voices
    static constexpr int num_voices = 5;

    std::vector<int> active_voices;

    // Synth voices
    SynthVoice filters[num_voices];

    //std::mutex audio_lock;

    int last_voice = 0;
    int note_on_counter = 0;
    int min_block_size = 32;
    bool strict_subblocks = false;

    std::vector<float> temp_buffer;
};
