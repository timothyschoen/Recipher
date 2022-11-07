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
class RecipherAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RecipherAudioProcessorEditor (RecipherAudioProcessor&);
    ~RecipherAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RecipherAudioProcessor& audioProcessor;
    
    juce::Slider dials[11];
    juce::TextButton toggles[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecipherAudioProcessorEditor)
};
