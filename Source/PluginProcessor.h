/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <aubio/aubio.h>
#include "FilterSynth.h"
#include "PhaseVocoder.h"
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

    void set_sample_speed(float stretch) {
        phase_vocoder->set_stretch(stretch);
    }

    MidiKeyboardState keystate;
    FilterSynth filter_synth;
    
    std::unique_ptr<PhaseVocoder> phase_vocoder;
    
    dsp::DelayLine<float> delay_line = dsp::DelayLine<float>(88200);
    
    LowpassFilter<2> lpf;
    
    Oscillator lfo;
    int lfo_destination = 1;

private:
    
    
    std::vector<float> input_buffer;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bandpass_hardwareAudioProcessor)
};
