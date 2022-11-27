#pragma once

#include <tuple>
#include <algorithm>


enum Shape
{
    Sine,
    Triangle,
    Square,
    Saw,
    NumShapes
};

template <typename Type>
Type map(Type sourceValue, Type sourceRangeMin, Type sourceRangeMax, Type targetRangeMin, Type targetRangeMax) {
    return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

template <typename Type>
constexpr Type map(Type value0To1, Type targetRangeMin, Type targetRangeMax) {
    return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
}

using FilterState = std::pair<float, float>;

struct ShapeFilter
{
    
    ShapeFilter() {
        // Initialise harmonics for each shape
        for(int i = 0; i < num_harmonics; i++) {
            int overtone = i + 1;
            shape_harmonics[(int)Shape::Sine][i] = !i;
            shape_harmonics[(int)Shape::Square][i] = (0.66f / overtone) * (overtone & 1);
            shape_harmonics[(int)Shape::Saw][i] = 0.5f / overtone;
            
            // triangle is a square wave with the amplitude values squared
            shape_harmonics[(int)Shape::Triangle][i] = 0.75f / (overtone) * (overtone & 1);
            shape_harmonics[(int)Shape::Triangle][i] *= shape_harmonics[(int)Shape::Triangle][i];
        }
                
        set_q(6.05);
        set_shape(0.0f);
        
        clear_filters();
        update_filter();
    }
    
    void clear_filters() {
        
        for(int i = 0; i < num_harmonics; i++) {
            auto& [s1, s2] = svf[0][i];
            
            s1 = 0.0f;
            s2 = 0.0f;
            
            // Apply to cascaded filter
            for(int c = 0; c < cascade; c++) {
                svf[c][i] = svf[0][i];
            }
            
        }
    }
    
    void set_pitch(float midi_note) {
        note = midi_note;
    }
    
    void set_shape(float shp) {
        shape = shp;
    }
    
    void set_stretch(float stretch_amt) {
        stretch = stretch_amt;
    }
    
    void set_stretch_mod(float mod) {
        stretch_mod = mod;
    }
    
    void set_q(float new_q) {
        q = std::clamp(new_q, 0.1f, 30.0f);
    }
    
    void set_bend(float bend_amt) {
        pitch_bend = bend_amt;
        
    }
    
    
    template <typename FloatType>
    static FloatType fast_tan (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = x * (-135135 + x2 * (17325 + x2 * (-378 + x2)));
        auto denominator = -135135 + x2 * (62370 + x2 * (-3150 + 28 * x2));
        return numerator / denominator;
    }
    
    void update_filter() {
        
        R2 = 1.0f / q;
        gain = R2;
        
        // Update filters
        for(int i = 0; i < num_harmonics; i++) {
            float total_stretch = std::clamp(stretch + stretch_mod, 0.1f, 2.0f);
            float stretch_factor = i * total_stretch;
            
            float frequency = mtof(note + pitch_bend) * (stretch_factor + 1.0f);
            
            if(frequency > (sample_rate / 2.0f)) break;
            
            g[i] = fast_tan(M_PI * frequency / sample_rate);
            h[i] = 1.0f / (1.0f + R2 * g[i] + g[i] * g[i]);
        }
    }
    
    float process(float input) {
        
        float output = 0.0f;
        // Calculate shape position
        float total_shape = std::clamp(shape, 0.0f, 2.9f);
        int low_shape = total_shape;
        int high_shape = low_shape + 1;
        float distance = total_shape - low_shape;
        
        for(int hr = 0; hr < num_harmonics; hr++) {
            // Calculate volume of current harmonic
            float current_harmonic = map(distance, shape_harmonics[low_shape][hr], shape_harmonics[high_shape][hr]);
            
            if(current_harmonic) {
                // Apply cascaded filters
                float filtered = input;
                
                for(int c = 0; c < cascade; c++) {
                    filtered = apply_filter(filtered, c, hr);
                    
                    if(!std::isfinite(filtered)) {
                        filtered = input;
                    }
                }
                
                output += filtered * current_harmonic;
            }
        }
        
        return output;
    }
    
    float apply_filter(float input, int c, int hr) {
        /*
         An IIR filter band-pass filter with 12 dB of attenuation per octave, using a TPT structure, designed
         for fast modulation (see Vadim Zavalishin's documentation about TPT
         structures for more information). Its behaviour is based on the analog
         state variable filter circuit.
         */
        
        auto& [s1, s2] = svf[c][hr];
        
        auto yHP = h[hr] * (input - s1 * (g[hr] + R2) - s2);
        auto yBP = yHP * g[hr] + s1;
        
        s1      = yHP * g[hr] + yBP;
        s2      = yBP * g[hr] + (yBP * g[hr] + s2);
        
        return yBP * gain;
    }

private:
    
    static constexpr int num_harmonics = 7;
    static constexpr int cascade = 3;
    
    float shape_harmonics[(int)Shape::NumShapes][num_harmonics];
    
    float note = 60.f;
    float q = 2.0f;
    float shape = 0.5f;
    float stretch = 1.0f;
    float stretch_mod = 0.0f;
    
    float pitch_bend = 0.0f;
    
    FilterState svf[cascade][num_harmonics];
    
    // Filter variables
    float g[num_harmonics];
    float h[num_harmonics];
    float R2;
    float gain;
};
