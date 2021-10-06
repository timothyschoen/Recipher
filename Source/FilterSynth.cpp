#include "FilterSynth.h"


FilterSynth::FilterSynth (MidiKeyboardState& key_state)  : keyboard_state (key_state)
{
    active_voices.reserve(num_voices);
}


void FilterSynth::prepare(dsp::ProcessSpec spec) {
    temp_buffer.resize(spec.maximumBlockSize * spec.numChannels);
    
}


void FilterSynth::process (std::vector<float>& input, const MidiBuffer& midi_data)
{
    const int num_channels = 1;
    
    int start_sample = 0;
    int num_samples = input.size();
    auto midi_iterator = midi_data.findNextSamplePosition (start_sample);
    
    bool first_event = true;
    
    const ScopedLock sl (lock);
    
    // MIDI handling from JUCE Synthesiser class
    for (; num_samples > 0; ++midi_iterator)
    {
        if (midi_iterator == midi_data.cend())
        {
            process_filters(input, temp_buffer, start_sample, num_samples);
            
            std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
            std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
            return;
        }
        
        const auto metadata = *midi_iterator;
        const int samples_to_next = metadata.samplePosition - start_sample;
        
        if (samples_to_next >= num_samples)
        {
            process_filters(input, temp_buffer, start_sample, num_samples);
            
            handleMidiEvent (metadata.getMessage());
            break;
        }
        
        if (samples_to_next < ((first_event && ! strict_subblocks) ? 1 : min_block_size))
        {
            handleMidiEvent (metadata.getMessage());
            continue;
        }
        
        first_event = false;
        
        process_filters(input, temp_buffer,  start_sample,  samples_to_next);
        
        handleMidiEvent (metadata.getMessage());
        start_sample += samples_to_next;
        num_samples  -= samples_to_next;
    }
    
    std::for_each (midi_iterator,
                   midi_data.cend(),
                   [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
    
    std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
    std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
}

void FilterSynth::note_on(int midi_note, int velocity) {
    
    // Check if note is already being played
    auto note_iter = std::find_if(active_voices.begin(), active_voices.end(), [midi_note](const std::tuple<float, float>& x) mutable {
        return std::get<1>(x) == midi_note;
    });
    
    // If so, use the same voice number
    int voice_number;
    if(note_iter != active_voices.end()) {
        voice_number = std::get<0>(*note_iter);
    }
    else {
        // Look for free voices
        bool found = false;
        for(voice_number = 0; voice_number < num_voices; voice_number++){
            if(filters[voice_number].envelope.is_released())  {
                found = true;
                break;
            }
        }
        // If we dont find a voice, steal the oldest voice
        if(!found) {
            voice_number = std::get<0>(active_voices[0]);
        }
    }
    
    // Save voice number for the note
    active_voices.push_back({voice_number, midi_note});
    
    float freq = mtof(midi_note);
    
    // Send note on to filter
    filters[voice_number].note_on(freq, velocity);
    filters[voice_number].sub_osc.set_frequency(freq / 2.0f);
}

void FilterSynth::note_off(int midi_note) {
    int idx = 0;
    // Find voice that is currently playing this note
    for(auto& [voice_number, note] : active_voices) {
        if(note == midi_note) {
            // Send note-off
            filters[voice_number].note_off();
            // Remove from active voices
            active_voices.erase(active_voices.begin() + idx);
            return;
        }
        idx++;
    }
}

void FilterSynth::handleMidiEvent(MidiMessage m) {
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
        //const int wheelPos = m.getPitchWheelValue();
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

void FilterSynth::process_filters(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples)
{
    for(auto& filter : filters) {
        filter.process(input, output, start_sample, num_samples);
    }
}

void FilterSynth::set_q(float q) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.set_q(q + 0.1);
    }
}

void FilterSynth::set_shape(float shape) {
    for(auto& filter : filters) {
        filter.set_shape(shape);
    }
}

void FilterSynth::set_attack(float attack) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.envelope.set_attack(attack);
    }
}
void FilterSynth::set_decay(float decay) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.envelope.set_decay(decay);
    }
}
void FilterSynth::set_sustain(float sustain) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.envelope.set_sustain(sustain);
    }
}
void FilterSynth::set_release(float release) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.envelope.set_release(release);
    }
}

void FilterSynth::set_sub(float sub) {
    const ScopedLock sl (lock);
    for(auto& filter : filters) {
        filter.set_sub(sub);
    }
}
