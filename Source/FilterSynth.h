#pragma once

#include <JuceHeader.h>
#include "BandpassFilter.h"
#include "Envelope.h"

struct FilterSynth
{
    FilterSynth (MidiKeyboardState& keyState);
    

    void prepare(dsp::ProcessSpec spec);

    // Copied from JUCE Synthesiser class
    void process (std::vector<float>& input, const MidiBuffer& midiData);
    
    void note_on(int midi_note, int velocity);
    void note_off(int midi_note);
    
    void handleMidiEvent(MidiMessage m);
    
    void process_filters(const std::vector<float>& input, std::vector<float>& output);
    
    void set_q(float q);
    void set_shape(int shape);
    
    void set_attack(float attack);
    void set_decay(float decay);
    void set_sustain(float sustain);
    void set_release(float release);
    
    // Max num voices
    static constexpr int num_voices = 8;
        
    std::vector<std::tuple<int, int>> active_voices;
    
    MidiKeyboardState& keyboardState;
    
    // Midi <-> frequency conversion
    std::function<float(float)> ftom = [](float pitch){ return 69.0f + 12.0f * log2(pitch / 440.0f); };
    std::function<float(float)> mtof = [](float midi) { return pow(2, (midi - 69.0f) / 12.0f) * 440.0f; };
    
    // Synth voices
    BandpassFilter filters[num_voices];
    
    CriticalSection lock;
    
    int last_voice = 0;
    
    double sampleRate = 0;
    uint32 lastNoteOnCounter = 0;
    int minimumSubBlockSize = 32;
    bool subBlockSubdivisionIsStrict = false;
    bool shouldStealNotes = true;
    BigInteger sustainPedalsDown;
    
    std::vector<float> temp_buffer;
};
