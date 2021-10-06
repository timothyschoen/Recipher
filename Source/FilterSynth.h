#pragma once

#include <JuceHeader.h>
#include "BandpassFilter.h"
#include "LowpassFilter.h"
#include "Envelope.h"

struct FilterSynth
{
    FilterSynth (MidiKeyboardState& key_state);

    void prepare(dsp::ProcessSpec spec);
    
    void process(std::vector<float>& input, const MidiBuffer& midiData);
    
    void note_on(int midi_note, int velocity);
    void note_off(int midi_note);
    
    void handleMidiEvent(MidiMessage m);
    
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
    
    
    // Max num voices
    static constexpr int num_voices = 8;
        
    std::vector<std::tuple<int, int>> active_voices;
    
    MidiKeyboardState& keyboard_state;

    // Synth voices
    BandpassFilter filters[num_voices];
    
    CriticalSection lock;
    
    int last_voice = 0;
    uint32 note_on_counter = 0;
    int min_block_size = 32;
    bool strict_subblocks = false;
    
    std::vector<float> temp_buffer;
};
