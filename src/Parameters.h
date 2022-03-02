#pragma once

// page 1:          page 2:
// 0: mix           gain
// 1: lpf q         feedback
// 2: lpf cutoff    delay
// 3: shape         stretch
// 4: q             freeze size
// 5: sub           drive
// 6: attack        LFO Shape
// 7: decay         LFO Depth
// 8: sustain       LFO Rate
// 9: release       LFO Dest

enum ParameterPin
{
    MIX = 15,
    LPF_Q,
    LPF_HZ,
    SHAPE,
    Q,
    SUB,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    
    GAIN = 25,
    FEEDBACK,
    DELAY,
    STRETCH,
    FREEZE_SIZE,
    DRIVE,
    LFO_SHAPE,
    LFO_RATE,
    LFO_DEPTH,
    LFO_DEST
};

using namespace daisy;

constexpr Parameter::Curve Linear = Parameter::Curve::LINEAR;
constexpr Parameter::Curve LogScale = Parameter::Curve::LOGARITHMIC;
constexpr Parameter::Curve ExpScale = Parameter::Curve::EXPONENTIAL;

using SingleParameter = std::tuple<ParameterPin, float, float, float, daisy::Parameter::Curve>;

using ParameterInit = std::vector<SingleParameter>;

extern DaisySeed sculpt;

struct SculptParameter
{
    SculptParameter(ParameterInit init){
        
        // Get parameter pin, range, init and scaling
        auto [pin1, min1, max1, init1, scale1] = init[0];
        auto [pin2, min2, max2, init2, scale2] = init[1];
        
        // Initialise ADC
        control.Init(sculpt.adc.GetPtr((int)pin1 - 15), sculpt.AudioSampleRate() / 256.0f);
        control.SetCoeff (0.5f);
        
        // Initialise Daisy control scaling
        parameters[0].Init(control, min1, max1, scale1);
        parameters[1].Init(control, min2, max2, scale2);
        
        // load initial value
        if(shift) {
            last_value[1] = init1;
            last_value[0] = control.Process();
        }
        else {
            last_value[1] = control.Process();
            last_value[0] = init2;
        }
    }
    

    // Gets parameter value
    float process(bool wanted_shift) {
        
        // If the current shift is wanted
        if(wanted_shift == shift) {
            
            // Get value scaled from 0-1 from adc
            float in_val = control.Process();
            
            // If it's close the the last recorded value, set touched to true
            if(abs(last_value[shift] - in_val) < 0.1) touched = true;
            
            // If the value hasn't been touched since the last shift change, return the last known value
            if(!touched) {
                // Return last value with scaling
                return parameters[shift].Process(last_value[shift]);
            }
                        
            // If it has been touched, set last value to the current value
            last_value[shift] = in_val;
            
            // Return the current adc value with scaling
            return parameters[shift].Process(in_val);
        }
        // If the unselected shift is wanted, return the last known value
        else {
            // Return last value with scaling
            return parameters[wanted_shift].Process(last_value[wanted_shift]);
        }
    }
    
    static inline bool shift = false;
    bool touched = true;
    
    Parameter parameters[2];
    
private:
    
    AnalogControl control;

    float last_value[2];
};

struct SculptParameters
{
    static inline std::vector<SculptParameter> sculpt_parameters = std::vector<SculptParameter>();
    
    static void init(bool shift) {
        
        SculptParameter::shift = shift;
        
        // Initialise parameters
        // Make sure the adc is initialised before calling this!
        sculpt_parameters = {
            SculptParameter({{MIX, 0.0f, 1.0f, 0.5f, Linear},            {GAIN, 1.0f, 4.0f, 0.5f, Linear}}),
            SculptParameter({{LPF_Q, 0.0f, 0.99f, 0.5f, Linear},         {FEEDBACK, 0.0f, 0.99f, 0.0f, Linear}}),
            SculptParameter({{LPF_HZ, 30.0f, 9000.0f, 0.7f, ExpScale},   {DELAY, 128.0f, 20000.0f, 0.1f, Linear}}),
            SculptParameter({{SHAPE, 0.0f, 3.0f, 0.75f, Linear},         {STRETCH, 0.0f, 2.0f, 0.5f, Linear}}),
            SculptParameter({{Q, 0.4f, 30.0f, 0.5f, ExpScale},           {FREEZE_SIZE, 64.0f, 8192.0f, 0.5f, Linear}}),
            SculptParameter({{SUB, 0.0f, 1.0f, 0.5f, Linear},            {DRIVE, 1.4f, 128.0f, 0.0f, Linear}}),
            SculptParameter({{ATTACK, 5.0f, 4000.0f, 0.02f, ExpScale},   {LFO_SHAPE, 0.0f, 2.0f, 0.5f, Linear}}),
            SculptParameter({{DECAY, 5.0f, 4000.0f, 0.4f, ExpScale},     {LFO_RATE, 0.5f, 20.0f, 0.2f, Linear}}),
            SculptParameter({{SUSTAIN, 0.0f, 1.0f, 0.3f, Linear},        {LFO_DEPTH, -1.0f, 1.0f, 0.5f, Linear}}),
            SculptParameter({{RELEASE, 5.0f, 4000.0f, 0.2f, ExpScale},   {LFO_DEST, 0.0f, 2.0f, 0.5f, Linear}})};
    }
   
    static void set_shift(bool shift) {
        
        // Check if changed
        if(SculptParameter::shift == shift) return;
        
        // When changing shift
        for(auto& param : sculpt_parameters) {
            param.touched = false;
        }
        
        SculptParameter::shift = shift;
    }

    static float get_value(ParameterPin pin) {
        
        if(pin >= 25)  {
            return sculpt_parameters[(int)pin - 25].process(true);
        }
        else {
            return sculpt_parameters[(int)pin - 15].process(false);
        }
    }

};

