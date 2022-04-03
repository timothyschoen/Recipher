#include "daisysp.h"
#include "daisy_seed.h"

// Expose samplerate and sculpt interface to headers
using namespace daisysp;
using namespace daisy;

constexpr float sample_rate = 32000.0f;
constexpr int max_delay_samples = 32000;
DaisySeed sculpt;

#include "ShapeFilter.h"
#include "Freeze.h"
#include "Parameters.h"
#include "LFO.h"

MidiHandler<MidiUartTransport> midi;
Led led;


Svf filt;
LFO lfo;

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
        env.Init(samplerate);
        env.SetSustainLevel(0.5f);
        env.SetTime(ADSR_SEG_ATTACK, 0.25f);
        env.SetTime(ADSR_SEG_DECAY, 0.005f);
        env.SetTime(ADSR_SEG_RELEASE, 0.2f);
        sub_1.Init(samplerate);
        sub_2.Init(samplerate);
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
            out += sub_1.Process() * sub_level * 0.1f;
            out += sub_2.Process() * sub_level * 0.08f;
            
            float y = out * (velocity / 127.f) * amp;
            
            return y;
        }
        else {
            filter.clear_filters();
        }
        return 0.f;
    }

    void note_on(float midi_note, float vel)
    {
        note     = midi_note;
        velocity = vel;
        
        sub_1.SetFreq(mtof(note - 12 + bend));
        sub_2.SetFreq(mtof(note - 24 + bend));
        
        filter.set_pitch(note);
        
        active  = true;
        envgate = true;
    }
    
    void set_bend(float bend_amt) {
        filter.set_bend(bend_amt);
        bend = bend_amt;
    }
    

    void note_off() { envgate = false; }

    inline bool  is_active() const { return active; }
    inline bool  is_released() { return env.GetCurrentSegment() == ADSR_SEG_RELEASE; }
    inline float get_note() const { return note; }

    ShapeFilter filter;
    Adsr       env;
    
    float bend = 0.0f;
    float sub_level = 0.2;
    
  private:
    
    Oscillator sub_1;
    Oscillator sub_2;

    float      note, velocity;
    bool       active;
    bool       envgate;
};

template <size_t max_voices>
class VoiceManager
{
  public:
    VoiceManager() {}
    ~VoiceManager() {}

    void Init(float samplerate)
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
        
        
        return sum;
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
    
    void set_stretch_mod(float all_val) {
        for(auto& voice : voices) voice.filter.set_stretch_mod(all_val);
    }
    
    
    void set_shape(float all_val) {
        for(auto& voice : voices) voice.filter.set_shape(all_val);
    }
    
    void set_q(float all_val) {
        for(auto& voice : voices) voice.filter.set_q(all_val);
    }
    
    void set_attack(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_ATTACK, all_val / 1000.0f);
    }
    
    void set_decay(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_DECAY, all_val / 1000.0f);
    }
    
    void set_sustain(float all_val) {
        for(auto& voice : voices) voice.env.SetSustainLevel(all_val);
    }
    
    void set_release(float all_val) {
        for(auto& voice : voices) voice.env.SetTime(ADSR_SEG_RELEASE, all_val / 1000.0f);
    }
    
    void set_sub(float all_val) {
        for(auto& voice : voices) voice.sub_level = all_val;
    }
    
    void set_bend(float pitch_bend) {
        for(auto& voice : voices) voice.set_bend(pitch_bend);
    }
    
    void update_filters() {
        for(auto& voice : voices) voice.filter.update_filter();
    }

  private:
    Voice  voices[max_voices];
    Voice* find_voice(float note)
    {
        Voice *v = NULL;
        
        for(size_t i = 0; i < max_voices; i++)
        {
            if(voices[i].get_note() == note)
            {
                return &voices[i];
            }
        }
        
        for(size_t i = 0; i < max_voices; i++)
        {
            if(!voices[i].is_active())
            {
                return &voices[i];
            }
        }
        
        for(size_t i = 0; i < max_voices; i++)
        {
            if(voices[i].is_released())
            {
                return &voices[i];
            }
        }
        

        
        return v;
    }
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

float lpf_mod = 0.0f;
float delay_mod = 0.0f;
float stretch_mod = 0.0f;

float lfo_depth = 1.0f;
float lfo_destination = 1.0f;


void apply_lfo() {
    
    float lfo_value = lfo.tick();
    
    led.Set(lfo_value > 0.0f);
    
    // Modulator functions
    static const std::function<void(float)> mod_targets[3] = {
        [](float mod) {
            delay_mod = mod * 5000.0f;
    },
        [](float mod) {
            lpf_mod = mod * 8000.0f;
            
    },
        [](float mod) {
            stretch_mod = mod;
    }
    };
    
    // Split modulator between sources when the knob is inbetween positions
    int first_target = lfo_destination;
    int second_target = first_target == 2 ? 0 : lfo_destination + 1;
    float diff = lfo_destination - first_target;

    float mod_1 = lfo_value * abs(lfo_depth) * (1.0f - diff);
    float mod_2 = lfo_value * lfo_depth * diff;
    
    mod_targets[first_target](mod_1);
    mod_targets[second_target](mod_2);
}

float noise_mix = 0.5f;
float input_gain = 1.0f;
float drive = 1.0f;
float feedback = 0.0f;

float delay_samples = 1.0f;
float lpf_cutoff = 18000.0f;

void update_parameters() {
    switches[0].Debounce();
    switches[1].Debounce();

    bool shift = switches[0].Pressed();
    
    SculptParameters::set_shift(shift);
    
        freeze.set_freeze(switches[1].Pressed());
        
        noise_mix = SculptParameters::get_value(MIX);
        
        filt.SetRes(SculptParameters::get_value(LPF_Q));
        lpf_cutoff = SculptParameters::get_value(LPF_HZ);
        
        voice_handler.set_q(SculptParameters::get_value(Q));
        voice_handler.set_shape(SculptParameters::get_value(SHAPE));
        voice_handler.set_sub(SculptParameters::get_value(SUB));
        
        voice_handler.set_attack(SculptParameters::get_value(ATTACK));
        voice_handler.set_decay(SculptParameters::get_value(DECAY));
        voice_handler.set_sustain(SculptParameters::get_value(SUSTAIN));
        voice_handler.set_release(SculptParameters::get_value(RELEASE));

        input_gain = SculptParameters::get_value(GAIN);
        feedback = SculptParameters::get_value(FEEDBACK);
        delay_samples = SculptParameters::get_value(DELAY);
        voice_handler.set_stretch(SculptParameters::get_value(STRETCH) + stretch_mod);
        freeze.set_freeze_size(SculptParameters::get_value(FREEZE_SIZE));
        drive = SculptParameters::get_value(DRIVE);
        lfo.set_shape(SculptParameters::get_value(LFO_SHAPE));
        lfo.set_frequency(SculptParameters::get_value(LFO_RATE));
        lfo_depth = SculptParameters::get_value(LFO_DEPTH);
        lfo_destination = SculptParameters::get_value(LFO_DEST);
    
    voice_handler.update_filters();
}

// Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            // Note Off can come in as Note On w/ 0 Velocity
            if(p.velocity == 0.f)
            {
                voice_handler.note_off(p.note, p.velocity);
            }
            else
            {
                voice_handler.note_on(p.note, p.velocity);
            }
        }
        break;
        
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_handler.note_off(p.note, p.velocity);
        }
        break;
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            if(p.control_number == 4) { // sustain pedal
                
            }
            if(p.control_number > 4 && p.control_number < 16) {
                // control change
            }
        }
        case PitchBend:
        {
            PitchBendEvent p = m.AsPitchBend();
            float scale = (p.value / 16384.f) - 0.5f; // TODO: not correct yet!
            float range = 12.0f;
            voice_handler.set_bend(scale * range);
            
        }
        break;
        default: break;
    }
}

template <typename FloatType>
    static FloatType fast_tanh (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = x * (135135 + x2 * (17325 + x2 * (378 + x2)));
        auto denominator = 135135 + x2 * (62370 + x2 * (3150 + 28 * x2));
        return numerator / denominator;
    }

void audio_callback(const float* const* in, float** out, size_t size)
{
    float synth_out;
    
    midi.Listen();
    while(midi.HasEvents())
    {
        HandleMidiMessage(midi.PopEvent());
    }
    
    update_parameters();
    led.Update();
    
    
    for(size_t i = 0; i < size; i++)
    {
        float input = (in[0][i] * input_gain * (1.0f - noise_mix)) + distribution(generator) * noise_mix;
        
        input = freeze.process(input);
        
        synth_out = voice_handler.process(input);

        apply_lfo();
        
        fonepole(smooth_time, delay_samples + delay_mod, 0.0005f);
        synth_out += delay.ReadHermite(std::clamp(smooth_time, 1.0f, static_cast<float>(max_delay_samples)));
        
        delay.Write(synth_out * feedback);
        
        // Apply distortion and compensate volume
        synth_out = (fast_tanh(synth_out * drive) * (1.0f / sqrt(drive)));
        
        fonepole(smooth_cutoff, lpf_cutoff + lpf_mod, 0.0005f);
        filt.SetFreq(std::clamp(smooth_cutoff, 20.0f, static_cast<float>(max_delay_samples)));
        
        filt.Process(synth_out);
        synth_out = filt.Low();
        
        // Output
        out[0][i] = synth_out * 1.5f;
        trig = 0.0;
    }
}

uint8_t buffer[128];
bool MsgRcvd = false;
uint8_t MsgSize = 0;

void UsbCallback(uint8_t* buf, uint32_t* len)
 {
   if(!buf || !len)
     return;
   uint8_t tmpSize;
   if(*len < 128U)
     tmpSize = *len;
   else
     tmpSize = 128U;
   for(size_t i = 0; i < tmpSize; i++)
   {
     buffer[i] = buf[i];
   }
   MsgRcvd = true;
   MsgSize = *len;
 }

int main(void)
{
    sculpt.Configure();
    sculpt.Init();
    
    sculpt.SetAudioBlockSize(256);
    
    sculpt.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);
    
    led.Init(DaisySeed::GetPin (4), false, sample_rate);

    switches[0].Init(DaisySeed::GetPin(25), 0, Switch::TYPE_TOGGLE, Switch::POLARITY_NORMAL, Switch::PULL_DOWN);
    switches[1].Init(DaisySeed::GetPin(28), 0, Switch::TYPE_TOGGLE, Switch::POLARITY_NORMAL, Switch::PULL_DOWN);
    
    //sculpt.usb_handle.Init(UsbHandle::FS_INTERNAL);
    //dsy_system_delay(250);
    //sculpt.usb_handle.SetReceiveCallback(UsbCallback, UsbHandle::FS_INTERNAL);
    
    // Initialise potmeters
    AdcChannelConfig adcConfig[num_potmeters];
    
    for (int i = 0; i < num_potmeters; i++)
    {
        adcConfig[i].InitSingle (daisy::DaisySeed::GetPin (15 + i));
    }
    
    sculpt.adc.Init (adcConfig, num_potmeters);
    sculpt.adc.Start();
        
    SculptParameters::init(switches[0].RawState());
    
    auto config = daisy::MidiHandler<MidiUartTransport>::Config();
    midi.Init(config);
    
    filt.Init(sample_rate);
    filt.SetFreq(6000.f);
    filt.SetRes(0.6f);
    filt.SetDrive(0.8f);

    delay.Init();
    
    voice_handler.Init(sample_rate);
    
    midi.StartReceive();
    
    // start callback
    sculpt.StartAudio(audio_callback);


    while(true) {
        System::Delay(250);
    }
    
}
