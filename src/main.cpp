#define JUCE 0

#if JUCE
#include "../app/daisy_juce.h"
#else
#include "daisy_seed.h"
#endif

#include "daisysp.h"

#include "Octaver.h"
#include <chrono>

// Expose samplerate and sculpt interface to headers
using namespace daisysp;
using namespace daisy;

#if JUCE
constexpr float sample_rate = 44100.0f;
#else
constexpr float sample_rate = 32000.0f;
#endif

constexpr float block_size = 256;
constexpr int max_delay_samples = 32000;


int active_midi_channel = 1;

DaisySeed sculpt;

#include "ShapeFilter.h"
#include "Freeze.h"
#include "Parameters.h"
#include "LFO.h"


#if !JUCE
MidiUartHandler uart_midi;
MidiUsbHandler usb_midi;

Led led;
#endif

Svf filt;
LFO lfo = LFO(sample_rate, block_size);

Octaver shifter;
Overdrive drive;
Balance drive_balance;

// One second maximum for freeze length and delay time
Freeze<max_delay_samples> freeze;
DelayLine<float, max_delay_samples> delay;

class Voice
{
public:
    Voice() {}
    ~Voice() {}
    
    void init(float samplerate)
    {
        active = false;
        envgate = false;
        pedal_down = false;
        timestamp = 0;
        
        env.Init(samplerate);
        env.SetSustainLevel(0.5f);
        env.SetTime(ADSR_SEG_ATTACK, 0.25f);
        env.SetTime(ADSR_SEG_DECAY, 0.005f);
        env.SetTime(ADSR_SEG_RELEASE, 0.2f);
        filter.set_q(6.0f);
    }
    
    float process(float input)
    {
        if(active)
        {
            float amp, out;
            
            amp = env.Process(envgate);
            if(!env.IsRunning())
                active = false;
            
            out = filter.process(input);
            
            float y = out * (velocity / 127.f) * amp;
            
            return y;
        }
        else {
            
            //filter.clear_filters();
        }
        return 0.f;
    }
    
    void note_on(float midi_note, float vel)
    {
        note     = midi_note;
        velocity = vel;
        timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        filter.set_pitch(note);
                
        active  = true;
        envgate = true;
    }
    
    void set_bend(float bend_amt) {
        filter.set_bend(bend_amt);
        bend = bend_amt;
    }
    
    
    void note_off() {
        envgate = false;
    }
    
    inline bool  is_active() const { return active; }
    inline bool  is_released() { return env.GetCurrentSegment() == ADSR_SEG_RELEASE; }
    inline float get_note() const { return note; }
    
    void set_sustain_level(float sustain) {
        if(!pedal_down) {
            env.SetSustainLevel(sustain);
        }
        
        sustain_level = sustain;
    }
    
    void set_sustain_pedal(bool is_down) {
        
        if(is_down && !pedal_down) {
            env.SetSustainLevel(1.0f);
        }
        if(!is_down && pedal_down) {
            env.SetSustainLevel(sustain_level);
        }
        
        pedal_down = is_down;
    }
    
    ShapeFilter filter;
    Adsr       env;
    
    float bend = 0.0f;
    float octaver_level = 0.2;
    long unsigned int timestamp;
    
private:
    
    float sustain_level;
    
    float note, velocity;

    bool active;
    bool envgate;
    
    
    
    bool pedal_down;
};

template <size_t max_voices>
class VoiceManager
{
public:
    VoiceManager() {}
    ~VoiceManager() {}
    
    void init(float samplerate)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].init(samplerate);
        }
    }
    
    float process(float input)
    {
        float sum = 0.0f;
        for(size_t i = 0; i < max_voices; i++)
        {
            sum += voices[i].process(input);
        }
        
        return sum * q_gain;
    }
    
    void note_on(float notenumber, float velocity)
    {
        Voice *v = find_voice(notenumber);
        if(v == NULL)
            return;
        v->note_on(notenumber, velocity);
    }
    
    void note_off(float notenumber, float velocity)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            Voice *v = &voices[i];
            if(v->is_active() && v->get_note() == notenumber)
            {
                v->note_off();
            }
        }
    }
    
    void free_voices()
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].note_off();
        }
    }
    
    void set_stretch(float all_val) {
        for(auto& voice : voices) voice.filter.set_stretch(all_val);
    }
    
    void set_shape(float all_val) {
        for(auto& voice : voices) voice.filter.set_shape(all_val);
    }
    
    void set_q(float all_val) {
        for(auto& voice : voices) voice.filter.set_q(all_val);
        q_gain = sqrt(all_val);
    }
    
    void set_attack(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_ATTACK, all_val / 1000.0f);
    }
    
    void set_decay(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_DECAY, all_val / 1000.0f);
    }
    
    void set_sustain(float all_val) {
        for(auto& voice : voices) voice.set_sustain_level(all_val);
    }
    
    void set_release(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_RELEASE, all_val / 1000.0f);
    }
    
    void set_bend(float pitch_bend) {
        for(auto& voice : voices) voice.set_bend(pitch_bend);
    }
    
    void update_filters() {
        for(auto& voice : voices) voice.filter.update_filter();
    }
    
    void set_sustain_pedal(bool pedal_down) {
        for(auto& voice : voices) voice.set_sustain_pedal(pedal_down);
    }
    
private:
    Voice  voices[max_voices];
    Voice* find_voice(float note)
    {
        // Check if the same note is already playing
        for(size_t i = 0; i < max_voices; i++)
        {
            if(voices[i].get_note() == note)
            {
                return &voices[i];
            }
        }
        
        // Check for free voices
        for(size_t i = 0; i < max_voices; i++)
        {
            if(!voices[i].is_active())
            {
                return &voices[i];
            }
        }
        
        // Check for voices that are in the release stage
        for(size_t i = 0; i < max_voices; i++)
        {
            if(voices[i].is_released())
            {
                return &voices[i];
            }
        }
        
        // Otherwise steal a note
        
        // Find lowest and highest note, we don't want to steal those
        int highest_note = 0;
        int lowest_note = 127;
        
        int lowest_idx = 0;
        int highest_idx = 0;
        
        for(size_t i = 0; i < max_voices; i++)
        {
            int note = voices[i].get_note();
            if(note < lowest_note) {
                lowest_idx = i;
                lowest_note = note;
            }
            if(note > highest_note) {
                highest_idx = i;
                highest_note = note;
            }
        }
        
        long unsigned int oldest_timestamp = -1;
        int oldest_idx = 0;
        
        for(int i = 0; i < static_cast<int>(max_voices); i++)
        {
            if(i != lowest_idx && i != highest_idx) {
                if(voices[i].timestamp < oldest_timestamp) {
                    oldest_idx = i;
                    oldest_timestamp = voices[i].timestamp;
                }
            }
        }
        
        return &voices[oldest_idx];
    }
    
    float q_gain = 1.0f;
};

static VoiceManager<8> voice_handler;

static float trig = 0;

static auto generator = std::default_random_engine();  // Generates random integers
static auto distribution = std::uniform_real_distribution<float>(-0.999, +0.999);

constexpr int num_potmeters = 10;
constexpr int num_switches = 2;

Switch switches[num_switches];

float smooth_time;
float smooth_cutoff;

float drive_amt = 1.0f;
float lpf_mod = 0.0f;
float delay_mod = 0.0f;
float stretch_mod = 0.0f;

float lfo_depth = 1.0f;
float lfo_destination = 1.0f;

ParameterPin mod_targets[3] = {ParameterPin::LPF_NOTE, ParameterPin::DELAY, ParameterPin::FREEZE_SIZE};

void apply_lfo() {
    
    float lfo_value = lfo.tick();

#if !JUCE
    led.Set(lfo_value > 0.0f);
#endif
    
    // Split modulator between sources when the knob is inbetween positions
    int first_target = lfo_destination;
    int second_target = first_target == 2 ? 0 : lfo_destination + 1;
    float diff = lfo_destination - first_target;
    
    float mod_1 = lfo_value * abs(lfo_depth) * (1.0f - diff);
    float mod_2 = lfo_value * lfo_depth * diff;
    
    for(int i = ParameterPin::MIX; i <= ParameterPin::LFO_DEST; i++) {
        auto pin = static_cast<ParameterPin>(i);
        
        if(i == mod_targets[first_target]) {
            SculptParameters::apply_modulation(pin, mod_1 * 0.5f);
        }
        else if(i == mod_targets[second_target]) {
            SculptParameters::apply_modulation(pin, mod_2 * 0.5f);
        }
        else {
            SculptParameters::apply_modulation(pin, 0.0f);
        }
    }
    
}

float noise_mix = 0.5f;
float input_gain = 1.0f;
float feedback = 0.0f;

float delay_samples = 1.0f;
float lpf_cutoff = 18000.0f;

float sub_octave = 0.0f;

void update_parameters() {
    
    bool shift = switches[0].RawState();
    
    SculptParameters::set_shift(shift);
    
    freeze.set_freeze(switches[1].RawState());
    
    noise_mix = SculptParameters::get_value(MIX);
    
    filt.SetRes(SculptParameters::get_value(LPF_Q));
    lpf_cutoff = mtof(SculptParameters::get_value(LPF_NOTE));
    
    voice_handler.set_q(SculptParameters::get_value(Q));
    voice_handler.set_shape(SculptParameters::get_value(SHAPE));
    sub_octave = SculptParameters::get_value(OCTAVER);
    
    if(sub_octave > 0.0f) {
        shifter.setShift(2.0f);
    }
    else {
        shifter.setShift(0.5f);
    }
    
    voice_handler.set_attack(SculptParameters::get_value(ATTACK));
    voice_handler.set_decay(SculptParameters::get_value(DECAY));
    voice_handler.set_sustain(SculptParameters::get_value(SUSTAIN));
    voice_handler.set_release(SculptParameters::get_value(RELEASE));
    
    input_gain = SculptParameters::get_value(GAIN);
    feedback = SculptParameters::get_value(FEEDBACK);
    delay_samples = SculptParameters::get_value(DELAY);
    voice_handler.set_stretch(SculptParameters::get_value(STRETCH));
    freeze.set_freeze_size(SculptParameters::get_value(FREEZE_SIZE));
    
    drive_amt = SculptParameters::get_value(DRIVE);
    drive.SetDrive(drive_amt);
    
    lfo.set_shape(SculptParameters::get_value(LFO_SHAPE));
    lfo.set_frequency(SculptParameters::get_value(LFO_RATE));
    lfo_depth = SculptParameters::get_value(LFO_DEPTH);
    lfo_destination = SculptParameters::get_value(LFO_DEST);

    voice_handler.update_filters();
}

// Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    if(m.channel != (active_midi_channel - 1)) return;
    
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_handler.note_on(p.note, p.velocity);
            break;
        }
            
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_handler.note_off(p.note, p.velocity);
            break;
        }
            
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            if(p.control_number == 4) { // sustain pedal
                voice_handler.set_sustain_pedal(p.value != 0);
            }
            break;
        }
        case PitchBend:
        {
            PitchBendEvent p = m.AsPitchBend();
            
            float range = 12.0f; // range in semitones
            float bend = p.value * (1.0f / 8192.0f) * range;
            
            voice_handler.set_bend(bend);
            break;
        }
        default: break;
    }
}

void audio_callback(const float* const* in, float** out, size_t size)
{
    float synth_out;
    
#if !JUCE
    uart_midi.Listen();
    while(uart_midi.HasEvents())
    {
        HandleMidiMessage(uart_midi.PopEvent());
    }
    
    usb_midi.Listen();
    while(usb_midi.HasEvents())
    {
        HandleMidiMessage(usb_midi.PopEvent());
    }

    
    led.Update();
#endif
    
    apply_lfo();
    update_parameters();
    

    for(size_t i = 0; i < size; i++)
    {
        float input = (in[0][i] * input_gain * noise_mix) + distribution(generator) * (1.0f - noise_mix);
        
        input = freeze.process(input);
        
        synth_out = voice_handler.process(input);
        
        synth_out += shifter.process(synth_out) * abs(sub_octave);
        
        fonepole(smooth_time, delay_samples + delay_mod, 0.0005f);
        synth_out += delay.ReadHermite(std::clamp(smooth_time, 1.0f, static_cast<float>(max_delay_samples)));
        
        delay.Write(synth_out * feedback);
        
        // Apply distortion
        float clean_out = synth_out;
        
        synth_out = drive.Process(synth_out);
        synth_out = drive_balance.Process(synth_out, clean_out);
        
        fonepole(smooth_cutoff, lpf_cutoff + lpf_mod, 0.0005f);
        filt.SetFreq(std::clamp(smooth_cutoff, 20.0f, static_cast<float>(max_delay_samples)));
        
        filt.Process(synth_out);
        synth_out = filt.Low();
        
        // Output
        out[0][i] = synth_out * 2.0f;
        trig = 0.0;
    }
}

#if JUCE

void init(float rate, int blocksize) {
    filt.Init(sample_rate);
    filt.SetFreq(6000.f);
    filt.SetRes(0.6f);
    filt.SetDrive(0.8f);
    
    delay.Init();
    
    voice_handler.init(sample_rate);
    
    SculptParameters::init(false);
    
    jassert(rate == sample_rate);
    jassert(blocksize == block_size);
    
    update_parameters();
}

void set_parameter(int idx, float value, bool shift) {
    int offset = shift ? 15 : 25;
    SculptParameters::set_value(static_cast<ParameterPin>(idx + offset), value);
    SculptParameters::set_shift(shift);
    switches[0].state = shift;
}
#else

int main(void)
{
    sculpt.Configure();
    sculpt.Init();
    
    sculpt.SetAudioBlockSize(block_size);
    
    sculpt.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);
    
    led.Init(DaisySeed::GetPin (4), false, sample_rate);
    
    switches[0].Init(DaisySeed::GetPin(25), 0, Switch::TYPE_TOGGLE, Switch::POLARITY_NORMAL, Switch::PULL_DOWN);
    switches[1].Init(DaisySeed::GetPin(28), 0, Switch::TYPE_TOGGLE, Switch::POLARITY_NORMAL, Switch::PULL_DOWN);
    
    // Initialise potmeters
    AdcChannelConfig adcConfig[num_potmeters];
    
    for (int i = 0; i < num_potmeters; i++)
    {
        adcConfig[i].InitSingle (daisy::DaisySeed::GetPin (15 + i));
    }
    
    sculpt.adc.Init (adcConfig, num_potmeters);
    sculpt.adc.Start();
    
    SculptParameters::init(switches[0].RawState());
    
    auto uart_config = MidiUartHandler::Config();
    uart_midi.Init(uart_config);
    
    auto usb_config = MidiUsbHandler::Config();
    
    usb_config.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    usb_midi.Init(usb_config);
    
    filt.Init(sample_rate);
    filt.SetFreq(6000.f);
    filt.SetRes(0.6f);
    filt.SetDrive(0.8f);
    
    delay.Init();
    drive.Init();
    drive_balance.Init(sample_rate);
    
    voice_handler.init(sample_rate);
    
    uart_midi.StartReceive();
    usb_midi.StartReceive();
    
    // start callback
    sculpt.StartAudio(audio_callback);
    
    
    while(true) {
        System::Delay(250);
    }
    
}
#endif
