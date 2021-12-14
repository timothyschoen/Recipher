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
    int num_samples = input.size();

    process_filters(input, temp_buffer, 0, num_samples);
    
    std::copy(temp_buffer.begin(), temp_buffer.begin() + input.size(), input.begin());
    std::fill(temp_buffer.begin(), temp_buffer.end(), 0.0f);
}


void FilterSynth::note_on(int midi_note, int velocity) {
    // Voice management logic
    
    // Check if note is already being played
    for(int v = 0; v < num_voices; v++){
        if(filters[v].current_note == midi_note) {
            filters[v].retrigger(velocity);
            return;
        }
    }
    
    // Find a freed voice
    for(int v = 0; v < num_voices; v++){
        if(filters[v].current_note == -1) {
            // Send note on to filter
            filters[v].note_on(midi_note, velocity);
            active_voices.push_back(v);
            return;
        }
    }
    
    // Otherwise take a note that is in the release state
    for(int v = 0; v < num_voices; v++){
        if(filters[v].envelope.is_releasing()) {
            filters[v].note_on(midi_note, velocity);
            return;
        }
    }
    
    // Final way out: use the oldest note
    filters[active_voices[0]].note_on(midi_note, velocity);
    
    if(active_voices.size() >= 2) {
        std::rotate(active_voices.begin(), active_voices.begin() + 1, active_voices.end()); // inefficient...
    }
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



void FilterSynth::process_filters(const std::vector<float>& input, std::vector<float>& output, int start_sample, int num_samples)
{
    for(auto& filter : filters) {
        filter.process(input, output, start_sample, num_samples);
    }
}

void FilterSynth::set_q(float q) {
    for(auto& filter : filters) {
        filter.set_q(q);
    }
}

void FilterSynth::set_shape(float shape) {
    for(auto& filter : filters) {
        filter.set_shape(shape);
    }
}

void FilterSynth::set_shape_mod(float shape) {
    for(auto& filter : filters) {
        filter.set_shape_mod(shape);
    }
}

void FilterSynth::set_attack(float attack) {
    for(auto& filter : filters) {
        filter.envelope.set_attack(attack);
    }
}
void FilterSynth::set_decay(float decay) {
    for(auto& filter : filters) {
        filter.envelope.set_decay(decay);
    }
}
void FilterSynth::set_sustain(float sustain) {
    for(auto& filter : filters) {
        filter.envelope.set_sustain(sustain);
    }
}
void FilterSynth::set_release(float release) {
    for(auto& filter : filters) {
        filter.envelope.set_release(release);
    }
}

void FilterSynth::set_sub(float sub) {
    for(auto& filter : filters) {
        filter.set_sub(sub);
    }
}

void FilterSynth::set_stretch(float stretch_amt) {
    for(auto& filter : filters) {
        filter.set_stretch(stretch_amt);
    }
}
