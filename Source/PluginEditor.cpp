/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Bandpass_hardwareAudioProcessorEditor::Bandpass_hardwareAudioProcessorEditor (Bandpass_hardwareAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), keyboard_component(p.keystate, MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 450);
    
    p.keystate.addListener(this);
    
    int edges[4] = {2, 3, 3, 1};
    StringArray button_text = {"Sine", "Square", "Saw", "Triangle"};
    
    for(int i = 0; i < 4; i++) {
        auto& button = toggle_buttons[i];
        addAndMakeVisible(button);
        button.setClickingTogglesState(true);
        button.setRadioGroupId(1001);
        button.setButtonText(button_text[i]);
        button.setConnectedEdges(edges[i]);
        button.onClick = [this, i]() mutable {
            audioProcessor.filter_synth.set_shape(i);
        };
    }
    
    q_slider.setRange(0, 50);
    q_slider.setValue(2.0f);
    q_slider.onValueChange = [this]() {
        audioProcessor.filter_synth.set_q(q_slider.getValue());
    };
    addAndMakeVisible(q_slider);
    
    adsr_sliders[0].setRange(0., 3000, 1);
    adsr_sliders[0].setValue(50);
    adsr_sliders[0].onValueChange = [this]() {
        audioProcessor.filter_synth.set_attack(adsr_sliders[0].getValue());
    };
    
    adsr_sliders[1].setRange(0., 3000, 1);
    adsr_sliders[1].setValue(1000);
    adsr_sliders[1].onValueChange = [this]() {
        audioProcessor.filter_synth.set_decay(adsr_sliders[1].getValue());
    };
    
    adsr_sliders[2].setRange(0., 1.0, 0.1);
    adsr_sliders[2].setValue(0.2);
    adsr_sliders[2].onValueChange = [this]() {
        audioProcessor.filter_synth.set_sustain(adsr_sliders[2].getValue());
    };
    
    adsr_sliders[3].setRange(0., 4000, 0.1);
    adsr_sliders[3].setValue(1000);
    adsr_sliders[3].onValueChange = [this]() {
        audioProcessor.filter_synth.set_release(adsr_sliders[3].getValue());
    };
    
    
    for(auto& slider : adsr_sliders) addAndMakeVisible(slider);
    
    
    addAndMakeVisible(keyboard_component);
    
    
}

Bandpass_hardwareAudioProcessorEditor::~Bandpass_hardwareAudioProcessorEditor()
{
}

//==============================================================================
void Bandpass_hardwareAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour(Colours::white);
    
    g.drawFittedText ("Q", 340, 2*30, 50, 30, Justification::left, 1);
    
    g.drawFittedText ("Attack", 340, 4*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Decay", 340, 5*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Sustain", 340, 6*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Release", 340, 7*30, 50, 30, Justification::left, 1);
}

void Bandpass_hardwareAudioProcessorEditor::resized()
{
    
    keyboard_component.setBounds (5, getHeight()-100, getWidth()-10, 100);
    
    toggle_buttons[0].setBounds (80, 1*30, 61, 20);
    toggle_buttons[1].setBounds (140, 1*30, 61, 20);
    toggle_buttons[2].setBounds (200, 1*30, 61, 20);
    toggle_buttons[3].setBounds (260, 1*30, 61, 20);
    
    q_slider.setBounds (80, 2*30, 250, 30);
    
    adsr_sliders[0].setBounds (80, 4*30, 250, 30);
    adsr_sliders[1].setBounds (80, 5*30, 250, 30);
    adsr_sliders[2].setBounds (80, 6*30, 250, 30);
    adsr_sliders[3].setBounds (80, 7*30, 250, 30);
}
