#pragma once

#include <JuceHeader.h>
#include "BandpassFilter.h"
#include "Envelope.h"

struct FilterSynth
{
    FilterSynth (MidiKeyboardState& keyState)  : keyboardState (keyState)
    {
        active_voices.reserve(num_voices);
    }
    

    void prepare(dsp::ProcessSpec spec) {
        temp_buffer.resize(spec.maximumBlockSize * spec.numChannels);
        sampleRate = spec.sampleRate;
    }

    // Copied from JUCE Synthesiser class
    void process (std::vector<float>& input, const MidiBuffer& midiData)
    {
        // must set the sample rate before using this!
        jassert (sampleRate != 0);
        const int num_channels = 2;

        int start_sample = 0;
        int num_samples = input.size() / num_channels;
        auto midi_iterator = midiData.findNextSamplePosition (start_sample);

        bool firstEvent = true;

        const ScopedLock sl (lock);

        for (; num_samples > 0; ++midi_iterator)
        {
            if (midi_iterator == midiData.cend())
            {
                if (num_channels > 0) {
                    process_filters(input, temp_buffer);
                }
                    //renderVoices (outputAudio, startSample, numSamples);
                std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
                std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
                return;
            }

            const auto metadata = *midi_iterator;
            const int samples_to_next = metadata.samplePosition - start_sample;

            if (samples_to_next >= num_samples)
            {
                if (num_channels > 0)
                    process_filters(input, temp_buffer);

                handleMidiEvent (metadata.getMessage());
                break;
            }

            if (samples_to_next < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
            {
                handleMidiEvent (metadata.getMessage());
                continue;
            }

            firstEvent = false;

            if (num_channels > 0)
                process_filters(input, temp_buffer);

            handleMidiEvent (metadata.getMessage());
            start_sample += samples_to_next;
            num_samples  -= samples_to_next;
        }

        std::for_each (midi_iterator,
                       midiData.cend(),
                       [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
        
        std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
        std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
    }
    
    void note_on(int midi_note, int velocity) {
        
        // TODO: check if same note is already playing somewhere
        
        auto note_iter = std::find_if(
            active_voices.begin(), active_voices.end(), [midi_note](const std::tuple<float, float>& x) mutable {
                return std::get<1>(x) == midi_note;
                
            });
        
        int voice_number;
        if(note_iter != active_voices.end()) {
            voice_number = std::get<0>(*note_iter);
        }
        else {
            bool found = false;
            for(voice_number = 0; voice_number < num_voices; voice_number++){
                if(filters[voice_number].envelope.getReleased())  {
                    found = true;
                    break;
                }
            }
            // If we dont find a voice, steal the oldest voice
            if(!found) {
                voice_number = std::get<0>(active_voices[0]);
            }
        }
        
        active_voices.push_back({voice_number, midi_note});
        
        filters[voice_number].note_on(mtof(midi_note), velocity);
    }
    
    void note_off(int midi_note) {
        int idx = 0;
        for(auto& [voice_number, note] : active_voices) {
            if(note == midi_note) {
                filters[voice_number].note_off();
                active_voices.erase(active_voices.begin() + idx);
                return;
            }
            idx++;
        }
    }
    
    void handleMidiEvent(MidiMessage m) {
        if (m.isNoteOn())
        {
            int midi_note = m.getNoteNumber();
            float velocity = m.getVelocity();
            note_on(midi_note, velocity);
        }
        else if (m.isNoteOff())
        {
            int midi_note = m.getNoteNumber();
            note_off(midi_note);

        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            //allNotesOff (channel, true);
        }
        else if (m.isPitchWheel())
        {
            const int wheelPos = m.getPitchWheelValue();
            //lastPitchWheelValues [channel - 1] = wheelPos;
            //handlePitchWheel (channel, wheelPos);
        }
        else if (m.isAftertouch())
        {
            //handleAftertouch (channel, m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isChannelPressure())
        {
            //handleChannelPressure (channel, m.getChannelPressureValue());
        }
        else if (m.isController())
        {
            //handleController (channel, m.getControllerNumber(), m.getControllerValue());
        }
        else if (m.isProgramChange())
        {
           // handleProgramChange (channel, m.getProgramChangeNumber());
        }
    }
    
    void process_filters(const std::vector<float>& input, std::vector<float>& output)
    {
        for(auto& filter : filters) {
            filter.process(input, output);
        }
    }
    
    void set_q(float q) {
        const ScopedLock sl (lock);
        for(auto& filter : filters) {
            filter.set_q(q + 0.1);
        }
    }
    
    void set_shape(int shape) {
        for(auto& filter : filters) {
            filter.set_shape((Shape)shape);
        }
    }
    
    void set_attack(float attack) {
        const ScopedLock sl (lock);
        for(auto& filter : filters) {
            filter.envelope.set_attack(attack);
        }
    }
    void set_decay(float decay) {
        const ScopedLock sl (lock);
        for(auto& filter : filters) {
            filter.envelope.set_decay(decay);
        }
    }
    void set_sustain(float sustain) {
        const ScopedLock sl (lock);
        for(auto& filter : filters) {
            filter.envelope.set_sustain(sustain);
        }
    }
    void set_release(float release) {
        const ScopedLock sl (lock);
        for(auto& filter : filters) {
            filter.envelope.set_release(release);
        }
    }
    
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
