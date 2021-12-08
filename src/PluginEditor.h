/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

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
    
    void handleNoteOn (MidiKeyboardState* keyState, int midi_channel, int midi_note,float velocity) override {
        audioProcessor.processor.filter_synth.note_on(midi_note, velocity * 127.0f);
    }
    
    void handleNoteOff (MidiKeyboardState* keyState, int midi_channel, int midi_note, float velocity) override {
        audioProcessor.processor.filter_synth.note_off(midi_note);
    }
    
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Bandpass_hardwareAudioProcessor& audioProcessor;
    
    MidiKeyboardComponent keyboard_component;
    TextButton toggle_buttons[4];
    Slider adsr_sliders[4];
    
    Slider shape_slider;
    Slider q_slider;
    
    TextButton freeze_on;
    Slider freeze_size;
    
    Slider lpf_q, lpf_cutoff;
    
    Slider sub_slider;
    Slider stretch;
    Slider delay_time, delay_fb;
    
    Slider lfo_depth, lfo_rate, lfo_destination;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bandpass_hardwareAudioProcessorEditor)
};
