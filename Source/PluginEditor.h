/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Bandpass_hardwareAudioProcessorEditor  : public juce::AudioProcessorEditor, MidiKeyboardState::Listener
{
public:
    Bandpass_hardwareAudioProcessorEditor (Bandpass_hardwareAudioProcessor&);
    ~Bandpass_hardwareAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void handleNoteOn (MidiKeyboardState * keyState, int midiChannel, int midiNoteNumber,float velocity) override {
        audioProcessor.filter_synth.note_on(midiNoteNumber, velocity * 127.0f);

    }

    void handleNoteOff (MidiKeyboardState * keyState, int midiChannel, int midiNoteNumber,float velocity) override {
        audioProcessor.filter_synth.note_off(midiNoteNumber, velocity);

    }


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Bandpass_hardwareAudioProcessor& audioProcessor;
    
    MidiKeyboardComponent keyboard_component;
    TextButton toggle_buttons[4];
    Slider adsr_sliders[4];
    
    Slider q_slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bandpass_hardwareAudioProcessorEditor)
};
