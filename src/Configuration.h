
struct Configuration {
    
    bool initialised = true;
    uint8_t midi_channel = 0;
    uint8_t param_mode = 0;
    uint8_t lfo_dest[3] = {0};
};

Configuration* config = nullptr;

std::aligned_storage<sizeof(Configuration)> DSY_SDRAM_BSS config_memory;


bool init_configuration() {
    
    // In case there are no settings in memory we can load
    if(!reinterpret_cast<Configuration*>(std::addressof(config_memory))->initialised) {
        config = new (std::addressof(config_memory))Configuration();
        return false;
    }

    config = reinterpret_cast<Configuration*>(std::addressof(config_memory));
    
    if(config->midi_channel > 64) return false;
    if(config->param_mode > 1)    return false;
    if(config->lfo_dest[0] > 35)  return false;
    if(config->lfo_dest[1] > 35)  return false;
    if(config->lfo_dest[2] > 35)  return false;
    
    config->initialised = true;
    return true;
}

std::array<int, 5> read_settings() {
    return {config->midi_channel, config->param_mode, config->lfo_dest[0], config->lfo_dest[1], config->lfo_dest[2]};
}

void write_settings(int midi_channel, int param_mode, int lfo_dest_1, int lfo_dest_2, int lfo_dest_3)
{
    config->midi_channel = static_cast<uint8_t>(midi_channel);
    config->param_mode = static_cast<uint8_t>(param_mode);
    config->lfo_dest[0] = static_cast<uint8_t>(lfo_dest_1);
    config->lfo_dest[1] = static_cast<uint8_t>(lfo_dest_2);
    config->lfo_dest[2] = static_cast<uint8_t>(lfo_dest_3);
}
