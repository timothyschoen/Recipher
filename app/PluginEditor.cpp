/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RecipherAudioProcessorEditor::RecipherAudioProcessorEditor (RecipherAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    
    for(int i = 0; i < 11; i++) {
        auto& dial = dials[i];
        dial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        dial.setRange(0.0f, 1.0f);
        dial.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        dial.onValueChange = [this, i]() mutable {
            audioProcessor.params[i] = dials[i].getValue();
        };
        addAndMakeVisible(dial);
    }
    
    for(auto& toggle : toggles) {
        toggle.setClickingTogglesState(true);
        addAndMakeVisible(toggle);
    }
    
    toggles[0].onClick = [this](){
        audioProcessor.shift = toggles[0].getToggleState();
    };
    toggles[1].onClick = [this](){
        audioProcessor.freeze = toggles[0].getToggleState();
    };
}

RecipherAudioProcessorEditor::~RecipherAudioProcessorEditor()
{
}

//==============================================================================
void RecipherAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void RecipherAudioProcessorEditor::resized()
{
    int rowHeight = 80;
    
    for(int i = 0; i < 4; i++) {
        float x = juce::jmap(i / 4.0f, 50.0f, getWidth() - 50.0f);
        dials[i].setBounds(x, rowHeight, 50, 50);
        dials[i + 7].setBounds(x, rowHeight * 3, 50, 50);
    }
    for(int i = 0; i < 3; i++) {
        float x = juce::jmap(i / 3.0f, 80.0f, getWidth() - 80.0f);
        dials[i+4].setBounds(x, rowHeight * 2, 50, 50);
    }
    
    int toggleY = rowHeight * 2 + 10;
    toggles[0].setBounds(50, toggleY, 25, 25);
    toggles[1].setBounds(getWidth() - 50.0f, toggleY, 25, 25);
   
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
