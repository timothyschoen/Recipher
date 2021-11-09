/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "AudioFile.h"



//==============================================================================
Bandpass_hardwareAudioProcessor::Bandpass_hardwareAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

Bandpass_hardwareAudioProcessor::~Bandpass_hardwareAudioProcessor()
{
}

//==============================================================================
const juce::String Bandpass_hardwareAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Bandpass_hardwareAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Bandpass_hardwareAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Bandpass_hardwareAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Bandpass_hardwareAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Bandpass_hardwareAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Bandpass_hardwareAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Bandpass_hardwareAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Bandpass_hardwareAudioProcessor::getProgramName (int index)
{
    return {};
}

void Bandpass_hardwareAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Bandpass_hardwareAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processor.prepare(sampleRate, samplesPerBlock);
}

void Bandpass_hardwareAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Bandpass_hardwareAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Bandpass_hardwareAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto* l_ptr = buffer.getWritePointer(0);
    auto* r_ptr = buffer.getWritePointer(1);
    
    for(auto message : midiMessages) {
        auto mess = message.getMessage();
        
        if(message.getMessage().isNoteOn()) {
            //processor.note_on(mess.getNoteNumber(), mess.getVelocity(), message.samplePosition);
        }
        if(message.getMessage().isNoteOff()) {
            //processor.note_off(mess.getNoteNumber(), message.samplePosition);
        }
    }
    
    processor.process(l_ptr, buffer.getNumSamples());
    
    
    // Fake stereo output
    for(int n = 0; n < buffer.getNumSamples(); n++) {
        r_ptr[n] = l_ptr[n];
    }
}

//==============================================================================
bool Bandpass_hardwareAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Bandpass_hardwareAudioProcessor::createEditor()
{
    return new Bandpass_hardwareAudioProcessorEditor (*this);
}

//==============================================================================
void Bandpass_hardwareAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Bandpass_hardwareAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Bandpass_hardwareAudioProcessor();
}