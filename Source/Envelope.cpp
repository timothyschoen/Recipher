#include <unistd.h>
#include <iostream>
#include <cmath>
#include "Envelope.h"

//Constructor class
Envelope::Envelope(float attack, float decay, float sustain, float release, float sr) {
    
    adsr = {attack, decay, sustain, release};
    
    sample_rate = sr;

    increments[0] = calc_rate(1.0f, attack, sample_rate);
    increments[1] = calc_rate(-1.0f, decay, sample_rate);
    increments[2] = calc_rate(-sustain, release, sample_rate);
    
    target[1] = sustain;
    
    released = true;
}

Envelope::~Envelope()
{
}

// Move the Envelopee to the next value
float Envelope::tick() {
        
    //Never go above 1!
    if (level > 1) {
        level = 1.0f;
    }
    // Add the increment if we are playing (attack, decay and release parts)
    if (playing) {
        level += increments[state];
    }
    // If we have arrived at our attack or release target, move to the next phase
    if((level >= target[state] && state == 0) || (level <= target[state] && state == 2)) {
        state++;
    }
    // If we reach our sustain phase, stop moving and hold until note-off
    else if (state == 1 && level <= target[state]) {
        playing = false;
        state++;
    }
    // When release is done, stop playing and send a release signal to voice manager
    if (state > 2) {
        //level = 0;
        playing = false;
        level = 0.0f;
        released = true;
    }
    
    return level;
    
}
// Move to release phase immediately
void Envelope::noteOff() {
    auto& [a, d, s, r] = adsr;
    playing = true;
    increments[2] = calc_rate(-level, r, sample_rate);
    state = 2;
}
//Note-on starts the Envelopee at the beginning again
void Envelope::noteOn() {
    playing = true;
    released = false;
    state = 0;
}
// Set out Envelope values
//1/x leads to the correct increment we need when we poll every 1ms
// Add a small number to make sure we never divide by 0!!! (infinite volume hurts your ears!)
void Envelope::set_attack(float attack){
    auto& [a, d, s, r] = adsr;
    a = attack;
    increments[0] = calc_rate(1.0f, attack, sample_rate);
}
void Envelope::set_decay(float decay){
    auto& [a, d, s, r] = adsr;
    d = decay;
    increments[1] = calc_rate(-1.0f, decay, sample_rate);
}
void Envelope::set_sustain(float sustain){
    auto& [a, d, s, r] = adsr;
    s = sustain;
    target[1] = sustain;
}
void Envelope::set_release(float release){
    auto& [a, d, s, r] = adsr;
    r = release;
    increments[2] = calc_rate(-s, release, sample_rate);
}
// Check if the Envelopee is done, to see if the voice can be freed
bool Envelope::getReleased() {
    return released;
}
