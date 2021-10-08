#include "Processor.h"

#include "daisy.h"
#include "daisy_seed.h"

using namespace daisy;


DaisySeed seed;

Processor processor;


void audio_callback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    
    float ctrl1 = seed.adc.GetFloat(0);
    
    // Move const input to output for 1 channel
    std::copy(in[0], in[0] + size, out[0]);
    
    processor.process(out[0], size);
    
    // Duplicate output
    std::copy(out[0], out[0] + size, out[1]);
}



int main() {

    seed.Configure();
    seed.Init();
    float sample_rate = seed.AudioSampleRate();
    
    AdcChannelConfig adcConfig;
    //Configure pin 21 as an ADC input. This is where we'll read the knob.
    adcConfig.InitSingle(seed.GetPin(21));

    //Initialize the adc with the config we just made
    seed.adc.Init(&adcConfig, 1);
    //Start reading values
    seed.adc.Start();
    
    //seed.SetBlockSize(128);
    seed.StartAudio(audio_callback);

#define FAKE_MIDI 1

#if FAKE_MIDI
    
    std::vector<float> notes = {60, 64, 67, 69, 71};
    
    int pos = 0;
    while(true) {
        
        processor.note_on(notes[pos], 100, 0);
        
        System::Delay(400);
        
        processor.note_off(notes[pos], 0);
        
        System::Delay(10);
        
        pos++;
        pos %= notes.size();
    }

#else

    MidiUartHandler midi;
    MidiUartHandler::Config config;
    midi.Init(config);
   
    while(true) {
        // Handle MIDI Events
        midi.Listen();
        while (midi.HasEvents()) {
            
            double time = 0.;
            MidiEvent m = midi.PopEvent();
            switch(m.type) {
                    
                case MidiMessageType::NoteOff: {
                    auto p = m.AsNoteOff();
                    processor.note_off(p.note, time);
                    break;
                }
                    
                case MidiMessageType::NoteOn: {
                    auto p = m.AsNoteOn();
                    processor.note_on(p.note, p.velocity, time);
                   
                   // handleKeyOn(time, p.channel, p.note, p.velocity);
                    break;
                }
            }
        }
    }
#endif
    
  return 1;
}

