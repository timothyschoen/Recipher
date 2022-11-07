/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "per/adc.h"
#include "per/dac.h"
#include "hid/parameter.h"
#include "hid/MidiEvent.h"

namespace daisy
{

class Switch
{
  public:

    /**
    Called at update_rate to debounce and handle timing for the switch.
    In order for events not to be missed, its important that the Edge/Pressed checks
    be made at the same rate as the debounce function is being called.
    */
    void Debounce() {};

    /** \return true if the button is held down (or if the toggle is on) */
    inline bool Pressed() const { return state; }
    
    
    bool state = false;

};

struct DaisySeed
{
    void Configure() {};
    
    void Init() {};
    
    void SetAudioBlockSize(int) {};
    
    void SetAudioSampleRate(int) {};
    
    int GetPin(int) { return 0; };
    
    void StartAudio(std::function<void(float*, float*, int)> cb) {};
    
    size_t AudioBlockSize();

    float AudioSampleRate();
    
    AdcHandle adc;
    DacHandle dac;
};

}
