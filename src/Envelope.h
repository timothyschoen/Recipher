#include <cmath>
#pragma once


class Envelope {


  float attack, decay, sustain, release;

  int state = 0;
  bool playing = false;
  float target[3] = {1, 0.3, 0};
  float increments[3] = {0.01, -0.2, -0.01};
  bool released;

  float level = 0.0f;

  float sample_rate;

public:
  Envelope(float attack, float decay, float sustain, float release, float sample_rate);
  ~Envelope();

  float tick();

  void note_off();

  void note_on(float velocity);

  void set_attack(float value);

  void set_decay(float value);

  void set_sustain(float value);

  void set_release(float value);

  // Calculate attack, decay and release constants
  float calc_rate(float distance, float ms_time, float sr) {
    return ms_time > 0.0f ? (float) (distance / ((ms_time / 1000.0f) * sr)) : -1.0f;
  }

  //Checks if voice is free
  bool is_released();
};
