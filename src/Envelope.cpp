#include <math.h>
#include "Envelope.h"

//Constructor class
Envelope::Envelope(float a, float d, float s, float r, float sr) {

    attack = a;
    decay = d;
    sustain = s;
    release = r;

    sample_rate = sr;

    increments[0] = calc_rate(1.0f, attack, sample_rate);
    increments[1] = calc_rate(1.0f, decay, sample_rate) * -1.0f;
    increments[2] = calc_rate(sustain, release, sample_rate) * -1.0f;

    target[1] = sustain;

    released = true;
    releasing = false;
}

Envelope::~Envelope()
{
}

// Move the Envelopee to the next value
float Envelope::tick() {

    //Never go above 1!
    if (level > 1.0f) {
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
        releasing = false;
        on_release();
    }

    return level;

}
// Move to release phase immediately
void Envelope::note_off() {
    playing = true;
    releasing = true;
    increments[2] = calc_rate(level, release, sample_rate) * -1.0f;
    state = 2;
}

//Note-on starts the Envelopee at the beginning again
void Envelope::note_on(float velocity) {
    velocity /= 127.0f;
    target[0] = velocity;
    target[1] = velocity * sustain;

    increments[0] = calc_rate(velocity, attack, sample_rate);
    increments[1] = calc_rate(velocity, decay, sample_rate) * -1.0f;

    playing = true;
    released = false;
    releasing = false;
    state = 0;
}

void Envelope::set_attack(float a){
    attack = std::fmax(a, 1.0f);
    increments[0] = calc_rate(1.0f, attack, sample_rate);
}

void Envelope::set_decay(float d){
    decay = std::fmax(d, 1.0f);
    increments[1] = calc_rate(1.0f, decay, sample_rate) * -1.0f;
}

void Envelope::set_sustain(float s){
    sustain = std::fmax(s, 0.01f);
    target[1] = sustain;
}

void Envelope::set_release(float r){
    release = r;
    increments[2] = calc_rate(sustain, release, sample_rate) * -1.0f;
}

// Check if the Envelopee is done, to see if the voice can be freed
bool Envelope::is_released() {
    return released;
}

bool Envelope::is_releasing() {
    return releasing;
}

