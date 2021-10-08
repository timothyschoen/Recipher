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
        message_queue.push_back(message);
    };


    void process_filters(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples);

    void set_q(float q);
    void set_shape(float shape);

    void set_attack(float attack);
    void set_decay(float decay);
    void set_sustain(float sustain);
    void set_release(float release);

    void set_sub(float sub_level);

    float ftom(float freq) { return 69.0f + 12.0f * log2(freq / 440.0f);     };
    float mtof(float midi) { return pow(2, (midi - 69.0f) / 12.0f) * 440.0f; };

    std::vector<MidiMess> message_queue;

    // Max num voices
    static constexpr int num_voices = 8;

    std::vector<std::tuple<int, int>> active_voices;

    // Synth voices
    SynthVoice filters[num_voices];

    //std::mutex audio_lock;

    int last_voice = 0;
    int note_on_counter = 0;
    int min_block_size = 32;
    bool strict_subblocks = false;

    std::vector<float> temp_buffer;
};
