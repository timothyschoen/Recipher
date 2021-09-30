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
        for(auto& filter : filters) {
            filter.prepare(spec);
        }
        
        tempBuffer = AudioBuffer<float>(spec.numChannels, spec.maximumBlockSize);
        sampleRate = spec.sampleRate;
    }

    // Copied from JUCE Synthesiser class
    void process (dsp::AudioBlock<float> input, const MidiBuffer& midiData)
    {
        
        int startSample = 0;
        int numSamples = input.getNumSamples();
        
        // must set the sample rate before using this!
        jassert (sampleRate != 0);
        const int targetChannels = input.getNumChannels();

        auto midiIterator = midiData.findNextSamplePosition (startSample);

        bool firstEvent = true;

        const ScopedLock sl (lock);

        for (; numSamples > 0; ++midiIterator)
        {
            if (midiIterator == midiData.cend())
            {
                if (targetChannels > 0) {
                    process_filters(input, tempBuffer);
                }
                    //renderVoices (outputAudio, startSample, numSamples);
                input.clear();
                input.copyFrom(tempBuffer);
                tempBuffer.clear();
                return;
            }

            const auto metadata = *midiIterator;
            const int samplesToNextMidiMessage = metadata.samplePosition - startSample;

            if (samplesToNextMidiMessage >= numSamples)
            {
                if (targetChannels > 0)
                    process_filters(input, tempBuffer);

                handleMidiEvent (metadata.getMessage());
                break;
            }

            if (samplesToNextMidiMessage < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
            {
                handleMidiEvent (metadata.getMessage());
                continue;
            }

            firstEvent = false;

            if (targetChannels > 0)
                process_filters(input, tempBuffer);

            handleMidiEvent (metadata.getMessage());
            startSample += samplesToNextMidiMessage;
            numSamples  -= samplesToNextMidiMessage;
        }

        std::for_each (midiIterator,
                       midiData.cend(),
                       [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
        
        input.clear();
        input.copyFrom(tempBuffer);
        tempBuffer.clear();
    }
    
    void note_on(int midi_note, int velocity) {
        
        // TODO: check if same note is already playing somewhere
        
        int voice_number;
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
        
        active_voices.push_back({voice_number, midi_note});
        
        filters[voice_number].note_on(mtof(midi_note));
    }
    
    void note_off(int midi_note, int velocity) {
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
            note_on(midi_note, 100);
        }
        else if (m.isNoteOff())
        {
            int midi_note = m.getNoteNumber();

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
    
    void process_filters(dsp::AudioBlock<float> input, dsp::AudioBlock<float> output)
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
    
    AudioBuffer<float> tempBuffer;
};
