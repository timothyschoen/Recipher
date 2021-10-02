/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <aubio/aubio.h>
#include "FilterSynth.h"
//==============================================================================
/**
*/
class Bandpass_hardwareAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Bandpass_hardwareAudioProcessor();
    ~Bandpass_hardwareAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    MidiKeyboardState keystate;
    FilterSynth filter_synth;
    
    fvec_t* phasevoc_in;
    cvec_t* fftgrain;
    fvec_t* phasevoc_out;
    
    aubio_pvoc_t* phase_vocoder[2];
    
private:
    
   
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bandpass_hardwareAudioProcessor)
};
