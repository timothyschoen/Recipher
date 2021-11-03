#include "Processor.h"
#include "daisy.h"
#include "daisy_seed.h"

using namespace daisy;


DaisySeed seed;

Processor processor;

constexpr int num_potmeters = 11;

bool received_midi = false;

MidiHandler midi;

daisy::AnalogControl adcAnalog[num_potmeters + 1];

void fake_midi();
void update_parameters();

void audio_callback(float** in, float** out, size_t size) {
    
    update_parameters();
    
    // Move const input to output for 1 channel
    std::copy(in[1], in[1] + size, out[0]);
    
    //processor.lpf.set_cutoff(((ctrl1 * 10000.0f) + 20.0f));

    processor.process(out[0], size);

    
    // Duplicate output
    std::copy(out[0], out[0] + size, out[1]);
}




int main() {
    
    seed.Configure();
    seed.Init();
    
    
    float sample_rate = seed.AudioSampleRate();
    
    midi.Init(daisy::MidiHandler::INPUT_MODE_UART1, daisy::MidiHandler::OUTPUT_MODE_NONE);
    
    int block_size = 256;
   
    AdcChannelConfig adcConfig[num_potmeters + 1];
    
    for (int i = 0; i < num_potmeters; i++)
    {
        adcConfig[i].InitSingle (daisy::DaisySeed::GetPin (15 + i));
    }
    
    adcConfig[num_potmeters].InitSingle (daisy::DaisySeed::GetPin(28));
    
    seed.adc.Init (adcConfig, num_potmeters + 1);
    
    for (int i = 0; i < num_potmeters; i++)
    {
        adcAnalog[i].Init (seed.adc.GetPtr (i), sample_rate / block_size);
        adcAnalog[i].SetCoeff (0.5);
    }
    
    adcAnalog[num_potmeters].Init(seed.adc.GetPtr(num_potmeters), sample_rate);
    

    //Start reading values
    seed.adc.Start();
    
    processor.prepare(sample_rate, block_size);
    
    processor.filter_synth.set_q(20.0f);
    processor.filter_synth.set_shape(0.0);
    
    seed.SetAudioBlockSize(block_size);
    seed.StartAudio(audio_callback);
    


#define FAKE_MIDI 0

#if FAKE_MIDI
    fake_midi();
#else

    while(true) {
        // Handle MIDI Events
        midi.Listen();
        while (midi.HasEvents()) {
            double time = 0.;
            MidiEvent m = midi.PopEvent();
            using MT = daisy::MidiMessageType;
            if(m.type == MT::NoteOff)
            {
                processor.filter_synth.note_off(m.data[0]);
            }
            else if(m.type == MT::NoteOn)
            {
                //received_midi = true;
                processor.filter_synth.note_on(m.data[0], m.data[1]);
            }
        }
        
        update_parameters();
        
        System::Delay(10);
    }
#endif
    
  return 1;
}





void fake_midi() {
    /*
    std::vector<float> notes = {60, 63, 66, 68, 70};
    
    int time = 0;
    bool playing_note = false;
    
    int pos = 0;
    while(true) {
        
        float shift_mode = adcShift.Process();
        
        // Volume
        float ctrl1 = adcAnalog[0].Process();
        processor.filter_synth.set_volume(ctrl1);
        
        // Noise/live mix
        float ctrl2 = adcAnalog[1].Process();
        
        
        float ctrl3 = adcAnalog[2].Process();
        float ctrl4 = adcAnalog[3].Process();
        if(shift_mode > 0.1) {
            // Cutoff
            processor.lpf.set_cutoff(ctrl3 * 15000.0f);
            
            // Resonance
            processor.lpf.set_q(ctrl4);
        }
        else {
            processor.delay_line.set_delay_samples(ctrl3);
            processor.delay_line.set_feedback(ctrl4);
        }
        
        // Timbre
        float ctrl5 = adcAnalog[4].Process();
        processor.filter_synth.set_shape(ctrl5 * 3.0f);
        
        // Q
        float ctrl6 = adcAnalog[5].Process();
        processor.filter_synth.set_q(ctrl6 * 25.0f);
        
        // Sub
        float ctrl7 = adcAnalog[6].Process();
        processor.filter_synth.set_sub(ctrl7);
        
        
        // A
        float ctrl8 = adcAnalog[7].Process();
        processor.filter_synth.set_attack(std::min(ctrl8 * 4000.0f, 4000.0f));
        
        // D
        float ctrl9 = adcAnalog[8].Process();
        processor.filter_synth.set_decay(std::min(ctrl9 * 4000.0f, 4000.0f));
        
        // S
        float ctrl10 = adcAnalog[9].Process();
        processor.filter_synth.set_sustain(std::clamp(ctrl10, 0.0f, 1.0f));
        
        // R
        float ctrl11 = adcAnalog[10].Process();
        //processor.filter_synth.set_sub(ctrl11);
        processor.filter_synth.set_release(std::min(ctrl11 * 4000.0f, 4000.0f));
        
        System::Delay(10);
        time++;
        
        std::vector<int> playing_notes;
        
        if(time > 20 && playing_notes.size() > 0){
            processor.filter_synth.note_off(playing_notes[0]);
            playing_notes.erase(playing_notes.begin());
        }
        
        if(time > 50){
            time -= 50;
            playing_notes.push_back(notes[pos]);
            processor.filter_synth.note_on(notes[pos], 100);
            
            pos++;
            pos %= notes.size();
        }

    } */
}


void update_parameters() {
    bool shift = adcAnalog[num_potmeters].Process() > 0.3;
    
    float volume = adcAnalog[0].Process();
    float mix = adcAnalog[1].Process(); // mix
    float delay = adcAnalog[2].Process();
    float feedback = adcAnalog[3].Process();
    
    float shape = adcAnalog[4].Process();
    float q = adcAnalog[5].Process();
    float sub = adcAnalog[6].Process();
    
    float a = adcAnalog[7].Process();
    float d = adcAnalog[8].Process();
    float s = adcAnalog[9].Process();
    float r = adcAnalog[10].Process();
    
    if(shift) {
        float lpf_cutoff = delay * 15000.0f;
        float lpf_q = feedback;
        
        // Set lowpass parameters
        processor.lpf.set_cutoff(lpf_cutoff);
        processor.lpf.set_q(lpf_q);
    }
    else {
        // Set delay parameters
        processor.delay_line.set_delay_samples(delay * 10000.0f);
        processor.delay_line.set_feedback(feedback);
    }
    
    processor.set_volume(volume);
    processor.filter_synth.set_shape(shape * 3.0f);
    processor.filter_synth.set_q(q * 25.0f);
    processor.filter_synth.set_sub(sub);
    
    processor.filter_synth.set_attack(std::min(a * 4000.0f, 4000.0f));
    processor.filter_synth.set_decay(std::min(d * 4000.0f, 4000.0f));
    processor.filter_synth.set_sustain(std::clamp(s, 0.0f, 1.0f));
    processor.filter_synth.set_release(std::min(r * 4000.0f, 4000.0f));
}
