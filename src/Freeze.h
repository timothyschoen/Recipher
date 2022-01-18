
struct Freeze
{
    
    Freeze(int max_length) {
        
        max_sample_len = max_length;
        read_head = 0;
        write_head = 0;
        prev_input = 0;
        playpack_rate = 65536;
        accumulator = 0;
        sample_loaded = false;
        sample_bank.resize(max_length);
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
            if (write_head >= freeze_len) {
                sample_loaded = true;
            }
            if (write_head >= max_sample_len) {
                write_en = false;
            }
        }
        if (sample_loaded) {
            
            if (playpack_rate >= 0) {
                accumulator += playpack_rate;
                read_head = accumulator >> 16;
            }
            if (read_head >= freeze_len) {
                accumulator = 0;
                read_head = 0;
            }
            output = sample_bank[read_head];
        }
        
        return output;
    }

    
    //std::array<float, 512> inputQueueArray[1];
    
    
    std::vector<float> sample_bank;
    int playpack_rate;
    int accumulator;
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
