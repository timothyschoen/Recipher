
template<int max_length>
struct Freeze
{
    Freeze() {
        max_sample_len = max_length;
        read_head = 0;
        write_head = 0;
        prev_input = 0;
        sample_loaded = false;
        set_freeze_size(std::min(512, max_length));
    }
    
    
    void set_freeze(bool frozen) {
        if(frozen != freeze) {
            freeze = frozen;
            sample_loaded = false;
            write_en = false;
            sample_req = true;
        }
        
    }
    
    void set_freeze_size(int grain_samples) {
        freeze_len = std::min<int>(grain_samples, max_sample_len);
    }
    
    float process(float input)
    {
        float output = 0;
        
        if(!freeze) {
            prev_input = input;
            return input;
        }

        if (sample_req) {
            // only begin capture on zero cross
            if ((input < 0.0f && prev_input >= 0.0f) ||
                (input >= 0.0f && prev_input < 0.0f)) {
                write_en = true;
                write_head = 0;
                read_head = 0;
                sample_req = false;
                
            } else {
                prev_input = input;
            }
        }
        
        if (write_en) {
            sample_bank[write_head++] = input;
            if (write_head >= freeze_len)     sample_loaded = true;
            if (write_head >= max_sample_len) write_en = false;
        }
        
        if (sample_loaded) {
            if (read_head >= freeze_len) {
                read_head -= freeze_len;
            }
            
            read_head++;
            output = sample_bank[read_head];
        }
        
        return output;
    }

    float sample_bank[max_length];
    int max_sample_len;
    int write_head;
    int read_head;
    int freeze_len;
    float prev_input;
    
    bool sample_loaded;
    bool write_en;
    bool sample_req;
    bool freeze;
};
