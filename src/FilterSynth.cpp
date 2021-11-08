#include "FilterSynth.h"


FilterSynth::FilterSynth ()
{
    active_voices.reserve(num_voices);
}


void FilterSynth::prepare(float sample_rate, int block_size) {
    temp_buffer.resize(block_size);
}


void FilterSynth::process(std::vector<float>& input)
{

    
    /*
     int start_position = 0;
     int idx = 0;
     
    for(auto& [position, type, note, velocity] : message_queue) {
        
        int num_samples = position - start_position;
        
        process_filters(input, temp_buffer, start_position, num_samples);
        
        type ? note_on(note, velocity) : note_off(note);
        
        // Very inefficient!!
        message_queue.erase(message_queue.begin() + idx);
        
        start_position += position;
        idx++;
        if(start_position > input.size()) break;
    } */
    
    int num_samples = input.size();
    
    //process_filters(input, temp_buffer, start_position, num_samples);
    
    //int num_left = input.size() - start_position;
    
    process_filters(input, temp_buffer, 0, num_samples);
    
    std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
    std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
    
}


void FilterSynth::note_on(int midi_note, int velocity) {
    
    //remove_duplicates(active_voices);
    // Check if note is already being played
    
        
    for(int v = 0; v < num_voices; v++){
        if(filters[v].current_note == midi_note) {
            filters[v].retrigger(velocity);
            return;
        }
    }
    
    for(int v = 0; v < num_voices; v++){
        if(filters[v].current_note == -1) {
            // Send note on to filter
            filters[v].note_on(midi_note, velocity);
            active_voices.push_back(v);
            return;
        }
    }
    
    for(int v = 0; v < num_voices; v++){
        if(filters[v].envelope.is_releasing()) {
            filters[v].note_on(midi_note, velocity);
            return;
        }
    }
    
    filters[active_voices[0]].note_on(midi_note, velocity);
    std::rotate(active_voices.begin(), active_voices.begin() + 1, active_voices.end());
    return;
}

void FilterSynth::note_off(int midi_note) {
    // Find voice that is currently playing this note
    for(int v = 0; v < num_voices; v++){
        if(filters[v].current_note == midi_note) {
            filters[v].note_off();
            
            for(int a = 0; a < active_voices.size(); a++) {
                if(active_voices[a] == v) {
                    active_voices.erase(active_voices.begin() + a);
                }
            }
            
        }
    }
}

/*
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
} */

void FilterSynth::process_filters(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples)
{
    for(auto& filter : filters) {
        filter.process(input, output, start_sample, num_samples);
    }
}

void FilterSynth::set_q(float q) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.set_q(q);
    }
    //audio_lock.unlock();
}

void FilterSynth::set_shape(float shape) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.set_shape(shape);
    }
    //audio_lock.unlock();
}

void FilterSynth::set_shape_mod(float shape) {
    for(auto& filter : filters) {
        filter.set_shape_mod(shape);
    }
}

void FilterSynth::set_attack(float attack) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.envelope.set_attack(attack);
    }
    //audio_lock.unlock();
}
void FilterSynth::set_decay(float decay) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.envelope.set_decay(decay);
    }
    //audio_lock.unlock();
}
void FilterSynth::set_sustain(float sustain) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.envelope.set_sustain(sustain);
    }
    //audio_lock.unlock();
}
void FilterSynth::set_release(float release) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.envelope.set_release(release);
    }
    //audio_lock.unlock();
}

void FilterSynth::set_sub(float sub) {
    //audio_lock.lock();
    for(auto& filter : filters) {
        filter.set_sub(sub);
    }
    //audio_lock.unlock();
}
