
struct Configuration {
    
    bool initialised;
    uint8_t midi_channel;
    uint8_t param_mode;
    uint8_t lfo_dest[3];
};

uint8_t DSY_QSPI_BSS config[6];

void init_configuration() {
    
    auto* config_data = static_cast<uint8_t*>(sculpt.qspi.GetData());

    uint8_t new_config[6];

    for(int i = 0; i < 6; i++) new_config[i] = config_data[i];
    // In case there are no settings in memory we can load
    if(!config_data[0]) {
        new_config[0] = true;
        new_config[1] = 0;
        new_config[2] = 0;
        new_config[3] = static_cast<int>(ParameterPin::LPF_NOTE);
        new_config[4] = static_cast<int>(ParameterPin::DELAY);
        new_config[5] = static_cast<int>(ParameterPin::FREEZE_SIZE);
    }
    
    if(config[1] > 64) new_config[1] = 0;
    if(config[2] > 1)  new_config[2] = 0;
    if(config[3] > 35) new_config[3] = static_cast<int>(ParameterPin::LPF_NOTE);
    if(config[4] > 35) new_config[4] = static_cast<int>(ParameterPin::DELAY);
    if(config[5] > 35) new_config[5] = static_cast<int>(ParameterPin::FREEZE_SIZE);

    size_t size = sizeof(config[0]) * 6;
    size_t address = (size_t)config;
    /** Erase qspi and then write that wave */
    sculpt.qspi.Erase(address, address + size);
    sculpt.qspi.Write(address, size, (uint8_t*)new_config);
}

std::array<int, 5> read_settings() {
    auto* config_data = static_cast<uint8_t*>(sculpt.qspi.GetData());
    return {config_data[1], config_data[2], config_data[3], config_data[4], config_data[5]};
}

void write_settings(int midi_channel, int param_mode, int lfo_dest_1, int lfo_dest_2, int lfo_dest_3)
{
    uint8_t new_config[6];

    new_config[1] = static_cast<uint8_t>(midi_channel);
    new_config[2] = static_cast<uint8_t>(param_mode);
    new_config[3] = static_cast<uint8_t>(lfo_dest_1);
    new_config[4] = static_cast<uint8_t>(lfo_dest_2);
    new_config[5] = static_cast<uint8_t>(lfo_dest_3);

    size_t size = sizeof(config[0]) * 6;
    size_t address = (size_t)config;
    /** Erase qspi and then write that wave */
    sculpt.qspi.Erase(address, address + size);
    sculpt.qspi.Write(address, size, (uint8_t*)new_config);


}
