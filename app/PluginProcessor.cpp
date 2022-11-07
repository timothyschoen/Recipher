/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "daisy_juce.h"



void init(float rate, int blocksize);
void audio_callback(const float* const* in, float** out, size_t size);
void HandleMidiMessage(daisy::MidiEvent m);
void set_parameter(int idx, float value, bool shift);

//==============================================================================
RecipherAudioProcessor::RecipherAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

RecipherAudioProcessor::~RecipherAudioProcessor()
{
}

//==============================================================================
const juce::String RecipherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RecipherAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RecipherAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RecipherAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RecipherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RecipherAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RecipherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RecipherAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RecipherAudioProcessor::getProgramName (int index)
{
    return {};
}

void RecipherAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RecipherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    init(sampleRate, samplesPerBlock);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RecipherAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RecipherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RecipherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    for(auto message : midiMessages) {
        daisy::MidiEvent event;
        auto m = message.getMessage();
        
        event.channel = m.getChannel();
        event.data[0] = m.getNoteNumber();
        event.data[1] = m.getVelocity();
        event.type = m.isNoteOn() ? daisy::MidiMessageType::NoteOn : daisy::MidiMessageType::NoteOff;
        
        HandleMidiMessage(event);
    }
    
    for(int i = 0; i < 11; i++) {
        set_parameter(i, params[i + 1], shift);
    }

    audio_callback(buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers(), buffer.getNumSamples());
}

//==============================================================================
bool RecipherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RecipherAudioProcessor::createEditor()
{
    return new RecipherAudioProcessorEditor (*this);
}

//==============================================================================
void RecipherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RecipherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RecipherAudioProcessor();
}
