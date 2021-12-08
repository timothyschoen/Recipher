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
Led led;


void fake_midi();
void update_parameters(bool shfit_mode, bool reset = false);

void audio_callback(float** in, float** out, size_t size) {
    bool shift = adcAnalog[num_potmeters].Process() > 0.3;
    
    update_parameters(shift);

    
    
    // Move const input to output for 1 channel
    std::copy(in[1], in[1] + size, out[0]);
    
    //processor.lpf.set_cutoff(((ctrl1 * 10000.0f) + 20.0f));

    processor.process(out[0], size);

    // Duplicate output
    std::copy(out[0], out[0] + size, out[1]);
    
    
    led.Set((rand() % 100) / 100.0f);
    led.Update();
}

// Default parameters for the not-selected page on startup
std::array<float, num_potmeters> page_1 = {0.9, 0.5, 0.6, 0.5, 0.75, 0.6, 0.3, 0.1, 0.2, 0.5, 0.1};
std::array<float, num_potmeters> page_2 = {0.0, 0.0, 0.1, 0.0, 0.5, 0.02, 0.6, 1.0, 0.0, 0.0};
std::array<bool, num_potmeters> touched;
bool last_shift = false;


int main() {
    
    seed.Configure();
    seed.Init();
    
    std::fill(touched.begin(), touched.end(), true);
    
    float sample_rate = seed.AudioSampleRate();
    int block_size = 256;
    
    midi.Init(daisy::MidiHandler::INPUT_MODE_UART1, daisy::MidiHandler::OUTPUT_MODE_NONE);
    
    led.Init(daisy::DaisySeed::GetPin (4), false, sample_rate / block_size);
    

   
    // Initialise potmeters
    AdcChannelConfig adcConfig[num_potmeters + 1];
    
    for (int i = 0; i < num_potmeters; i++)
    {
        adcConfig[i].InitSingle (daisy::DaisySeed::GetPin (15 + i));
    }
    
    
    
    // Initialise shift knob
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
    
    last_shift = adcAnalog[num_potmeters].Process() > 0.3;
    
    // Load default values on unselected page
    update_parameters(!last_shift, true);
    
    // Initialise audio on daisy, spawns a new thread
    seed.SetAudioBlockSize(block_size);
    seed.StartAudio(audio_callback);
    

    
    // Handle midi in a loop
    // TODO: is this thread-safe?
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
        
        System::Delay(10);
    }
    
  return 1;
}

void update_parameters(bool shift, bool reset) {
    
    // Logic for pickup-mode
    // When changing shift mode, set every knob to not-touched
    if(shift != last_shift || reset) {
        std::fill(touched.begin(), touched.end(), false);
    }
    
    last_shift = shift;

    auto& page = shift ? page_1 : page_2;
    
    for(int i = 0; i < num_potmeters; i++) {
        float value = adcAnalog[i].Process();
        if(abs(value - page[i]) < 0.05f) {
            touched[i] = true;
        }
        
        if(touched[i]) {
            page[i] = value;
        }
    }
    
    if(!shift) {
        processor.set_volume(page[0]);
        processor.set_mix(page[1]);
        
        // Set lowpass parameters
        processor.lpf.set_q(page[2] + 0.15f);
        processor.lpf.set_cutoff(page[3] * 15000.0);

        processor.filter_synth.set_shape(page[4] * 3.0f);
        processor.filter_synth.set_q((page[5] + 0.15f) * 10.0f);
        processor.filter_synth.set_sub(page[6]);
        
        processor.filter_synth.set_attack(std::min(page[7] * 4000.0f, 4000.0f));
        processor.filter_synth.set_decay(std::min(page[8] * 4000.0f, 4000.0f));
        processor.filter_synth.set_sustain(std::clamp(page[9], 0.0f, 1.0f));
        processor.filter_synth.set_release(std::min(page[10] * 4000.0f, 4000.0f));
        
    }
    else {
        processor.set_gain(page[1] * 3.0f);
        
        // Set delay parameters
        processor.delay_line.set_delay_samples(page[2] * 10000.0f);
        processor.delay_line.set_feedback(page[3]);
        
        processor.set_drive(page[4] * 128.0f);
        
        processor.set_lfo_shape(page[7] * 3);
        processor.set_lfo_freq(page[8] * 10.0f);
        processor.set_lfo_depth(page[9]);
        processor.set_lfo_dest(page[10] * 2.5);
    }
}
