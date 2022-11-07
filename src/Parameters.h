#pragma once


// page 1:          page 2:
// 0: mix           gain
// 1: lpf q         feedback
// 2: lpf cutoff    delay
// 3: shape         stretch
// 4: q             drive
// 5: octaver       freeze length
// 6: attack        LFO Shape
// 7: decay         LFO Depth
// 8: sustain       LFO Rate
// 9: release       LFO Dest

enum ParameterPin
{
    MIX = 15,
    LPF_Q,
    LPF_NOTE,
    SHAPE,
    Q,
    OCTAVER,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    
    GAIN = 25,
    FEEDBACK,
    DELAY,
    STRETCH,
    DRIVE,
    FREEZE_SIZE,
    LFO_SHAPE,
    LFO_RATE,
    LFO_DEPTH,
    LFO_DEST
};

enum ParameterMode
{
    PICKUP,
    TOUCH
};

constexpr Parameter::Curve Linear = Parameter::Curve::LINEAR;
constexpr Parameter::Curve LogScale = Parameter::Curve::LOGARITHMIC;
constexpr Parameter::Curve ExpScale = Parameter::Curve::EXPONENTIAL;

using SingleParameter = std::tuple<ParameterPin, float, float, float, daisy::Parameter::Curve, std::vector<float>>;

using ParameterInit = std::vector<SingleParameter>;

static inline ParameterMode parameter_mode = PICKUP;

struct SculptParameter
{

    SculptParameter(ParameterInit init){
        
        // Get parameter pin, range, init and scaling
        auto [pin1, min1, max1, init1, scale1, deadzones1] = init[0];
        auto [pin2, min2, max2, init2, scale2, deadzones2] = init[1];
        
#if !JUCE
        // Initialise ADC
        control.Init(sculpt.adc.GetPtr((int)pin1 - 15), sculpt.AudioSampleRate() / 256.0f);
        control.SetCoeff (0.5f);
        
        
        float control_value = control.GetRawFloat();
        // load initial value
        if(shift) {
            last_value[1] = control_value;
            last_value[0] = init1;
        }
        else {
            last_value[1] = init2;
            last_value[0] = control_value;
        }
#else
        last_value[1] = init1;
        last_value[0] = init2;
        set_value = init1;
        
#endif
        
        deadzones[0] = deadzones1;
        deadzones[1] = deadzones2;
        
        // Initialise Daisy control scaling
        parameters[0].Init(min1, max1, scale1);
        parameters[1].Init(min2, max2, scale2);
        
        set_fc(0.5f);
        
        touched_value = control_value;
    }
    
    float process_pickup(float knob_position) {
        
        // If it's close the the last recorded value, set touched to true
        if(abs(last_value[shift] - knob_position) < 0.1f) touched = true;
        
        // If the value hasn't been touched since the last shift change, return the last known value
        if(!touched) {
            // Return last value with scaling
            return parameters[shift].Process(apply_deadzones(last_value[shift], shift));
        }
                    
        // If it has been touched, set last value to the current value
        last_value[shift] = knob_position;
        
        // Return the current adc value with scaling
        return parameters[shift].Process(apply_deadzones(knob_position, shift));

    }
    
    float process_touch(float knob_position) {
        if(abs(touched_value - knob_position) > 0.03f) touched = true;
        
        // If the value hasn't been touched since the last shift change, return the last known value
        if(!touched) {
            // Return last value with scaling
            return parameters[shift].Process(apply_deadzones(last_value[shift], shift));
        }
        
        // If it has been touched, set last value to the current value
        last_value[shift] = knob_position;
        
        // Return the current adc value with scaling
        return parameters[shift].Process(apply_deadzones(knob_position, shift));
    }
    
    void set_touch_value() {
        touched_value = control.Process();
    }
    
    // Gets parameter value
    float process(bool wanted_shift) {
        
        // If the current shift is wanted, read current position
        if(wanted_shift == shift) {
            
#if JUCE
            float knob_position = set_value;
#else
            // Get value scaled from 0-1 from adc
            float knob_position = control.Process();
#endif
            
            if(parameter_mode == ParameterMode::TOUCH)
            {
                return process_touch(knob_position);
            }
            else {
                return process_pickup(knob_position);
            }
            
        }
        // If the unselected shift is wanted, return the last known value
        else {
            // Return last value with scaling
            return parameters[wanted_shift].Process(apply_deadzones(last_value[wanted_shift], wanted_shift));
        }
    }
    
    
    inline void set_fc(double Fc) {
        b1 = exp(-2.0 * M_PI * Fc);
        a0 = 1.0 - b1;
    }
    
    inline float apply_filter(float in, bool shift) {
        return z1[shift] = in * a0 + z1[shift] * b1;
    }
    
    inline void apply_modulation(float mod_value, bool shift)
    {
        modulation_value[shift] = mod_value;
    }
    
    inline float apply_deadzones(float value, bool shift) {
        for(auto& deadzone : deadzones[shift]) {
            if(abs(value - deadzone) < deadzone_size) {
                return std::clamp(apply_filter(deadzone, shift) + modulation_value[shift], 0.0f, 1.0f);
            }
        }
        
        // just to keep filter state up-to-date
        apply_filter(value, shift);
        
        return std::clamp(value + modulation_value[shift], 0.0f, 1.0f);
    }
    
    static inline bool shift = false;
    bool touched = true;
    float touched_value;
    
private:
    
    Parameter parameters[2];
    std::vector<float> deadzones[2];
    
    float last_value[2];
    double a0, b1, z1[2];
    
    float modulation_value[2];
    
    float deadzone_size = 0.03f;
    
#if !JUCE
    AnalogControl control;
#else
public:
    float set_value = 0;
#endif
};

struct SculptParameters
{
    static inline std::vector<SculptParameter> sculpt_parameters = std::vector<SculptParameter>();
    
    static void init(bool shift) {
        
        SculptParameter::shift = shift;
        
        // Initialise parameters
        // Make sure the adc is initialised before calling this!

        sculpt_parameters = {
            SculptParameter({{MIX, 0.0f, 1.0f, 0.5f, Linear, {}},            {GAIN, 1.0f, 4.0f, 0.5f, Linear, {}}}),
            SculptParameter({{LPF_Q, 0.0f, 0.99f, 0.5f, Linear, {}},         {FEEDBACK, 0.0f, 0.99f, 0.0f, Linear, {}}}),
            SculptParameter({{LPF_NOTE, 23.0f, 132.0f, 0.7f, Linear, {}},    {DELAY, 128.0f, (sample_rate / 2.0f), 0.1f, Linear, {}}}),
            SculptParameter({{SHAPE, 0.0f, 3.0f, 0.75f, Linear, {}},         {STRETCH, 0.0f, 2.0f, 0.5f, Linear, {0.5f}}}),
            SculptParameter({{Q, 1.0f, 30.0f, 0.5f, ExpScale, {}},           {DRIVE, 0.1f, 1.0f, 0.5f, Linear, {}}}),
            SculptParameter({{OCTAVER, -1.0f, 1.0f, 0.5f, Linear, {0.5f}},   {FREEZE_SIZE, 64.0f, 8192.0f, 0.0f, ExpScale, {}}}),
            SculptParameter({{ATTACK, 5.0f, 4000.0f, 0.02f, ExpScale, {}},   {LFO_SHAPE, 0.0f, 2.0f, 0.5f, Linear, {}}}),
            SculptParameter({{DECAY, 5.0f, 4000.0f, 0.4f, ExpScale, {}},     {LFO_RATE, 0.5f, 20.0f, 0.2f, Linear, {}}}),
            SculptParameter({{SUSTAIN, 0.0f, 1.0f, 0.3f, Linear, {}},        {LFO_DEPTH, -1.0f, 1.0f, 0.5f, Linear, {0.5f}}}),
            SculptParameter({{RELEASE, 5.0f, 4000.0f, 0.2f, ExpScale, {}},   {LFO_DEST, 0.0f, 2.0f, 0.5f, Linear, {}}})
        };
        
    }
    
    static void set_shift(bool shift) {
        
        // Check if changed
        if(SculptParameter::shift == shift) return;
        
        // When changing shift
        for(auto& param : sculpt_parameters) {
            param.touched = false;
            
            if(parameter_mode == TOUCH) {
                param.set_touch_value();
            }
        }

        SculptParameter::shift = shift;
    }
    
    static void apply_modulation(ParameterPin pin, float value) {
        if(pin >= 25)  {
            return sculpt_parameters[(int)pin - 25].apply_modulation(value, true);
        }
        else {
            return sculpt_parameters[(int)pin - 15].apply_modulation(value, false);
        }
    }
    
    static float get_value(ParameterPin pin) {
        
        if(pin >= 25)  {
            return sculpt_parameters[(int)pin - 25].process(true);
        }
        else {
            return sculpt_parameters[(int)pin - 15].process(false);
        }
    }
    
#if JUCE
    static float set_value(ParameterPin pin, float new_value) {
        
        if(pin >= 25)  {
            return sculpt_parameters[(int)pin - 25].set_value = new_value;
        }
        else {
            return sculpt_parameters[(int)pin - 15].set_value = new_value;
        }
    }
#endif
};

