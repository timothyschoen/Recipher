#pragma once

/*
 static inline uint32_t hash_xs32(uint32_t x)
 {
 x ^= x << 13;
 x ^= x >> 17;
 x ^= x << 5;
 return x;
 }
 
 inline uint32_t myrand()
 {
 static uint32_t seed = 1;
 seed                 = hash_xs32(seed);
 return seed;
 }
 
 class Octaver
 {
 public:
 Octaver() {}
 ~Octaver() {}
 // Initialize pitch shifter
 void Init(float sr)
 {
 force_recalc_ = false;
 sr_           = sr;
 mod_freq_     = 5.0f;
 SetSemitones();
 for(uint8_t i = 0; i < 2; i++)
 {
 gain_[i] = 0.0f;
 d_[i].Init();
 phs_[i].Init(sr, 50, i == 0 ? 0 : PI_F);
 }
 shift_up_ = true;
 del_size_ = SHIFT_BUFFER_SIZE;
 SetDelSize(del_size_);
 fun_ = 0.0f;
 }
 
 // process pitch shifter
 float Process(float &in)
 {
 float val, fade1, fade2;
 // First Process delay mod/crossfade
 fade1 = phs_[0].Process();
 fade2 = phs_[1].Process();
 if(prev_phs_a_ > fade1)
 {
 mod_a_amt_ = fun_ * ((float)(myrand() % 255) / 255.0f)
 * (del_size_ * 0.5f);
 mod_coeff_[0]
 = 0.0002f + (((float)(myrand() % 255) / 255.0f) * 0.001f);
 }
 if(prev_phs_b_ > fade2)
 {
 mod_b_amt_ = fun_ * ((float)(myrand() % 255) / 255.0f)
 * (del_size_ * 0.5f);
 mod_coeff_[1]
 = 0.0002f + (((float)(myrand() % 255) / 255.0f) * 0.001f);
 }
 slewed_mod_[0] += mod_coeff_[0] * (mod_a_amt_ - slewed_mod_[0]);
 slewed_mod_[1] += mod_coeff_[1] * (mod_b_amt_ - slewed_mod_[1]);
 prev_phs_a_ = fade1;
 prev_phs_b_ = fade2;
 if(shift_up_)
 {
 fade1 = 1.0f - fade1;
 fade2 = 1.0f - fade2;
 }
 mod_[0] = fade1 * (del_size_ - 1);
 mod_[1] = fade2 * (del_size_ - 1);
 #ifdef USE_ARM_DSP
 gain_[0] = arm_sin_f32(fade1 * (float)M_PI);
 gain_[1] = arm_sin_f32(fade2 * (float)M_PI);
 #else
 gain_[0] = sinf(fade1 * PI_F);
 gain_[1] = sinf(fade2 * PI_F);
 #endif
 
 // Handle Delay Writing
 d_[0].Write(in);
 d_[1].Write(in);
 // Modulate Delay Lines
 //mod_a_amt = mod_b_amt = 0.0f;
 d_[0].SetDelay(mod_[0] + mod_a_amt_);
 d_[1].SetDelay(mod_[1] + mod_b_amt_);
 d_[0].SetDelay(mod_[0] + slewed_mod_[0]);
 d_[1].SetDelay(mod_[1] + slewed_mod_[1]);
 val = 0.0f;
 val += (d_[0].Read() * gain_[0]);
 val += (d_[1].Read() * gain_[1]);
 return val;
 }
 
 // sets transposition in semitones
 void SetTransposition(const float &transpose)
 {
 float   ratio;
 uint8_t idx;
 if(transpose_ != transpose || force_recalc_)
 {
 transpose_ = transpose;
 idx        = (uint8_t)fabsf(transpose);
 ratio      = semitone_ratios_[idx % 12];
 ratio *= (uint8_t)(transpose / 12) + 1;
 if(transpose > 0.0f)
 {
 shift_up_ = true;
 }
 else
 {
 shift_up_ = false;
 }
 mod_freq_ = ((ratio - 1.0f) * sr_) / del_size_;
 
 phs_[0].SetFreq(mod_freq_);
 phs_[1].SetFreq(mod_freq_);
 if(force_recalc_)
 {
 force_recalc_ = false;
 }
 }
 }
 
 // sets delay size changing the timbre of the pitchshifting
 void SetDelSize(uint32_t size)
 {
 del_size_     = size < SHIFT_BUFFER_SIZE ? size : SHIFT_BUFFER_SIZE;
 force_recalc_ = true;
 SetTransposition(transpose_);
 }
 
 // sets an amount of internal random modulation, kind of sounds like tape-flutter
 inline void SetFun(float f) { fun_ = f; }
 
 private:
 inline void SetSemitones()
 {
 for(size_t i = 0; i < 12; i++)
 {
 semitone_ratios_[i] = powf(2.0f, (float)i / 12);
 }
 }
 typedef daisysp::DelayLine<float, SHIFT_BUFFER_SIZE> ShiftDelay;
 ShiftDelay                                  d_[2];
 float                                       pitch_shift_, mod_freq_;
 uint32_t                                    del_size_;
 // lfo stuff
 
 
 bool   force_recalc_;
 float  sr_;
 bool   shift_up_;
 daisysp::Phasor phs_[2];
 float  gain_[2], mod_[2], transpose_;
 float  fun_, mod_a_amt_, mod_b_amt_, prev_phs_a_, prev_phs_b_;
 float  slewed_mod_[2], mod_coeff_[2];
 // pitch stuff
 float semitone_ratios_[12];
 };
 */

struct Octaver
{
    Octaver()
    {
        delaylength = max_delay - 24;
        half_length = delaylength / 2;
        delay[0] = 12;
        delay[1] = max_delay / 2;
        
        delayline[0].Init();
        delayline[1].Init();

        delayline[0].SetDelay(delay[0]);
        delayline[1].SetDelay(delay[1]);
        rate = 1.0;
    }

    
    void setShift(float shift)
    {
        if (shift < 1.0) {
            rate = 1.0 - shift;
        }
        else if (shift > 1.0) {
            rate = 1.0 - shift;
        }
        else {
            rate = 0.0;
            delay[0] = half_length + 12;
        }
    }
    
    
    inline float process(float input)
    {
        // Calculate the two delay length values, keeping them within the
        // range 12 to max_delay-12.
        delay[0] += rate;
        while (delay[0] > max_delay-12) delay[0] -= delaylength;
        while (delay[0] < 12) delay[0] += delaylength;
        
        delay[1] = delay[0] + half_length;
        while (delay[1] > max_delay-12) delay[1] -= delaylength;
        while (delay[1] < 12) delay[1] += delaylength;
        
        // Set the new delay line lengths.
        delayline[0].SetDelay(delay[0]);
        delayline[1].SetDelay(delay[1]);
        
        // Calculate a triangular envelope.
        env[1] = fabs((delay[0] - half_length + 12) * (1.0 / (half_length + 12)));
        env[0] = 1.0 - env[1];
        
        // Delay input and apply envelope.
        delayline[0].Write(input);
        delayline[1].Write(input);
        
        last_frame =  env[0] * delayline[0].Read();
        last_frame += env[1] * delayline[1].Read();
        
        return last_frame;
    }
    
    static constexpr int max_delay = 3072;
    daisysp::DelayLine<float, max_delay> delayline[2];
    
    float delay[2];
    float env[2];
    float last_frame;
    float rate;
    unsigned long delaylength;
    unsigned long half_length;


};
