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
    setSize (1000, 450);
    
    p.keystate.addListener(this);
    
    shape_slider.setRange(0, 3);
    shape_slider.setValue(2.0f);
    shape_slider.onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_shape(shape_slider.getValue());
    };
    addAndMakeVisible(shape_slider);
    
    q_slider.setRange(0, 60);
    q_slider.setValue(2.0f);
    q_slider.onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_q(q_slider.getValue());
    };
    addAndMakeVisible(q_slider);
    
    
    lpf_q.setRange(0, 3);
    lpf_q.setValue(1.0f);
    lpf_q.onValueChange = [this]() {
        audioProcessor.processor.lpf.set_q(lpf_q.getValue());
    };
    addAndMakeVisible(lpf_q);
    
    lpf_cutoff.setRange(0, 20000.0f);
    lpf_cutoff.setValue(5000.0f);
    lpf_cutoff.setSkewFactor(0.5);
    lpf_cutoff.onValueChange = [this]() {
        audioProcessor.processor.lpf.set_cutoff(lpf_cutoff.getValue());
    };
    addAndMakeVisible(lpf_cutoff);
    
    
    sub_slider.setRange(0, 1.0f);
    sub_slider.setValue(1.0f);
    sub_slider.onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_sub(sub_slider.getValue());
    };
    addAndMakeVisible(sub_slider);
    
    
    /*
    sample_speed.setRange(-7.0f, 7.0f);
    sample_speed.setValue(1.0f);
    sample_speed.onValueChange = [this]() {
        audioProcessor.processor.set_sample_speed(sample_speed.getValue());
    };
    addAndMakeVisible(sample_speed); */
    
    delay_time.setRange(0, 22050.0f);
    delay_time.setValue(1.0f);
    delay_time.onValueChange = [this]() {
        audioProcessor.processor.set_delay_time(delay_time.getValue());
    };
    addAndMakeVisible(delay_time);
    
    delay_fb.setRange(0.0f, 0.98f);
    delay_fb.setValue(0.3f);
    delay_fb.onValueChange = [this]() {
        audioProcessor.processor.set_delay_fb(delay_fb.getValue());
    };
    addAndMakeVisible(delay_fb);
    
    lfo_depth.setRange(0, 1.0f);
    lfo_depth.setValue(1.0f);
    lfo_depth.onValueChange = [this]() {
        audioProcessor.processor.set_lfo_depth(lfo_depth.getValue());
    };
    addAndMakeVisible(lfo_depth);
    
    
    lfo_rate.setRange(0, 16.0f);
    lfo_rate.setValue(1.0f);
    lfo_rate.onValueChange = [this]() {
        audioProcessor.processor.set_lfo_freq(lfo_rate.getValue());
    };
    addAndMakeVisible(lfo_rate);
    
    
    lfo_destination.setRange(0, 3.0f, 1);
    lfo_destination.setValue(1.0f);
    lfo_destination.onValueChange = [this]() {
        audioProcessor.processor.lfo_destination = lfo_destination.getValue();
    };
    addAndMakeVisible(lfo_destination);
    
    
    adsr_sliders[0].setRange(0., 3000, 1);
    adsr_sliders[0].setValue(50);
    adsr_sliders[0].onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_attack(adsr_sliders[0].getValue());
    };
    
    adsr_sliders[1].setRange(0., 3000, 1);
    adsr_sliders[1].setValue(1000);
    adsr_sliders[1].onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_decay(adsr_sliders[1].getValue());
    };
    
    adsr_sliders[2].setRange(0., 1.0, 0.1);
    adsr_sliders[2].setValue(0.2);
    adsr_sliders[2].onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_sustain(adsr_sliders[2].getValue());
    };
    
    adsr_sliders[3].setRange(0., 4000, 0.1);
    adsr_sliders[3].setValue(1000);
    adsr_sliders[3].onValueChange = [this]() {
        audioProcessor.processor.filter_synth.set_release(adsr_sliders[3].getValue());
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
    
    g.drawFittedText ("Shape", 280, 1*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Q", 280, 2*30, 50, 30, Justification::left, 1);
    
    g.drawFittedText ("Attack", 280, 4*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Decay", 280, 5*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Sustain", 280, 6*30, 50, 30, Justification::left, 1);
    g.drawFittedText ("Release", 280, 7*30, 50, 30, Justification::left, 1);
    
    g.drawFittedText ("LPF Cutoff", 600, 1*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("LPF Q", 600, 2*30, 80, 30, Justification::left, 1);
    
    g.drawFittedText ("SUB", 600, 4*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("STRETCH", 600, 5*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("DELAY TIME", 600, 6*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("DELAY FB", 600, 7*30, 80, 30, Justification::left, 1);
    
    g.drawFittedText ("LFO RATE", 920, 1*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("LFO DEPTH", 920, 2*30, 80, 30, Justification::left, 1);
    g.drawFittedText ("LFO DEST.", 920, 3*30, 80, 30, Justification::left, 1);
}

void Bandpass_hardwareAudioProcessorEditor::resized()
{
    
    keyboard_component.setBounds (5, getHeight()-100, getWidth()-10, 100);
    
    toggle_buttons[0].setBounds (20, 1*30, 61, 20);
    toggle_buttons[1].setBounds (80, 1*30, 61, 20);
    toggle_buttons[2].setBounds (140, 1*30, 61, 20);
    toggle_buttons[3].setBounds (200, 1*30, 61, 20);
    
    shape_slider.setBounds(20, 1*30, 250, 30);
    q_slider.setBounds (20, 2*30, 250, 30);
    
    adsr_sliders[0].setBounds (20, 4*30, 250, 30);
    adsr_sliders[1].setBounds (20, 5*30, 250, 30);
    adsr_sliders[2].setBounds (20, 6*30, 250, 30);
    adsr_sliders[3].setBounds (20, 7*30, 250, 30);
    
    lpf_cutoff.setBounds(340, 1*30, 250, 30);
    lpf_q.setBounds (340, 2*30, 250, 30);
    
    sub_slider.setBounds (340, 4*30, 250, 30);
    sample_speed.setBounds (340, 5*30, 250, 30);
    delay_time.setBounds (340, 6*30, 250, 30);
    delay_fb.setBounds (340, 7*30, 250, 30);
    
    lfo_rate.setBounds (660, 1*30, 250, 30);
    lfo_depth.setBounds (660, 2*30, 250, 30);
    lfo_destination.setBounds (660, 3*30, 250, 30);
}
