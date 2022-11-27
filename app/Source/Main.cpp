#include <JuceHeader.h>

using namespace juce;

//==============================================================================
class MainComponent  : public Component, private MidiInputCallback, private Timer
{
    const Image logo = ImageFileFormat::loadFrom(BinaryData::clearcut_png, BinaryData::clearcut_pngSize);

    const Typeface::Ptr defaultTypeface = Typeface::createSystemTypefaceFor(BinaryData::IBMPlexMono_ttf, BinaryData::IBMPlexMono_ttfSize);
    
    // Unique ID to identify messages
    const uint8_t recipher_message_id_1 = 73;
    const uint8_t recipher_message_id_2 = 11;
    
    // All types of messages we can send or receive
    enum MessageType
    {
        Channel,
        ToggleBehaviour,
        LFODest,
        Dump
    };
    
    // LFO Destination happens to be the last parameter, which is very lucky
    StringArray lfo_destinations = {"MIX", "LPF Q", "LPF FREQ", "SHAPE", "BANDPASS Q", "OCTAVER SHIFT", "ATTACK", "DECAY", "SUSTAIN", "RELEASE", "INPUT GAIN", "FEEDBACK", "DELAY", "STRETCH", "DRIVE", "FREEZE SIZE", "LFO SHAPE", "LFO RATE", "LFO DEPTH"};
    
public:
    MainComponent()
    {
        auto* lnf = new LookAndFeel_V3();
        lnf->setColour(ComboBox::outlineColourId, Colours::black);
        lnf->setColour(ComboBox::focusedOutlineColourId, Colours::black);
        lnf->setColour(ComboBox::buttonColourId, Colours::black);
        lnf->setColour(PopupMenu::highlightedTextColourId, Colours::white);
        lnf->setColour(PopupMenu::highlightedBackgroundColourId, Colours::black);
        setLookAndFeel(lnf);

        
        getLookAndFeel().setDefaultSansSerifTypeface(defaultTypeface);
        setOpaque (true);
        
        // Try to establish a connection
        initialised = createConnection();
        
        send_message(Dump);
        
        // Start timer that will make sure the connection is maintained
        startTimer(initialised ? 300 : 100);
        
        // Resize window
        setSize (420, 270);
        
        // Add options for toggle combo
        shift_param_mode.addItem("PICKUP MODE", 1);
        shift_param_mode.addItem("TOUCH MODE", 2);
        
        // Add options for channel combo
        for(int i = 1; i < 65; i++) {
            channel_select.addItem(String(i), i);
        }
        
        channel_select.onChange = [this](){
            // Send channel change message
            send_message(Channel, {channel_select.getSelectedId()});
        };
        
        shift_param_mode.onChange = [this](){
            // Send toggle mode change message
            send_message(ToggleBehaviour, {shift_param_mode.getSelectedId() - 1});
        };
        

        // Add child components
        addAndMakeVisible(channel_select);
        addAndMakeVisible(shift_param_mode);
        
        int idx = 15;
        for(auto destination : lfo_destinations) {
            lfo_dest_1.addItem(destination, idx);
            lfo_dest_2.addItem(destination, idx);
            lfo_dest_3.addItem(destination, idx);
            idx++;
        }
        
        // Set default values: these should be overridden when connecting the Recipher
        // So doesn't really matter, but it's nicer than empty comboboxes when nothing is connected
        channel_select.setSelectedId(1);
        shift_param_mode.setSelectedId(1);
        
        lfo_dest_1.setSelectedId(17);
        lfo_dest_2.setSelectedId(27);
        lfo_dest_3.setSelectedId(30);
        
        addAndMakeVisible(lfo_dest_1);
        addAndMakeVisible(lfo_dest_2);
        addAndMakeVisible(lfo_dest_3);
        
        lfo_dest_1.onChange = [this](){
            // Send channel change message
            send_message(LFODest, {0, lfo_dest_1.getSelectedId()});
        };
        
        lfo_dest_2.onChange = [this](){
            // Send toggle mode change message
            send_message(LFODest, {1, lfo_dest_2.getSelectedId()});
        };
        
        lfo_dest_3.onChange = [this](){
            // Send toggle mode change message
            send_message(LFODest, {2, lfo_dest_3.getSelectedId()});
        };
    }
    
    
    ~MainComponent() override
    {
        // Close midi input if still opened
        if(last_input.isNotEmpty()) {
            device_manager.removeMidiInputDeviceCallback(last_input, this);
        }
        
        if(auto* lnf = &getLookAndFeel()) {
            setLookAndFeel(nullptr);
            delete lnf;
        }
    }
    
    void timerCallback() override {
         
        auto allDevices = MidiOutput::getAvailableDevices();
        auto* midiOut = device_manager.getDefaultMidiOutput();
        
        bool wasInitialised = initialised;
        
        if(!midiOut || !midiOut->getName().startsWith("Recipher") || !allDevices.contains(midiOut->getDeviceInfo())) {
            initialised = false;
        }
        
        // If we don't have a connection with Recipher, try to create one
        if(!initialised) {
            initialised = createConnection();
        }
        
        if(initialised != wasInitialised) {
            repaint();
        }
        
        startTimer(initialised ? 300 : 100);
        
        // Tell recipher to dump current parameters so we can copy them to the gui
        send_message(Dump);
    }
    
    // Tries to connect to Recipher and returns true if it succeeds
    bool createConnection() {
        bool foundMidiIn = false;
        bool foundMidiOut = false;
        
        auto midiInputs = MidiInput::getAvailableDevices();
        auto midiOutputs = MidiOutput::getAvailableDevices();
        
        // Find Reciphers MIDI Input to receive initial parameter values
        for (auto input : midiInputs)
        {
            if(input.name.startsWith("Recipher")) {
    
                if (!device_manager.isMidiInputDeviceEnabled (input.identifier))
                    device_manager.setMidiInputDeviceEnabled (input.identifier, true);

                device_manager.addMidiInputDeviceCallback (input.identifier, this);
                
                last_input = input.identifier;
                
                foundMidiIn = true;
            }
        }
        
        // Find Reciphers MIDI Output to push settings to
        for (auto output : midiOutputs)
        {
            if(output.name.startsWith("Recipher")) {
                device_manager.setDefaultMidiOutputDevice(output.identifier);
                device_manager.getDefaultMidiOutput()->startBackgroundThread();
                
                foundMidiOut = true;
            }
        }
        
        return foundMidiIn && foundMidiOut;
    }
    
    void paint (Graphics& g) override
    {
        g.fillAll(Colours::white);
        
        // Draw circle indicating connection status
        g.setColour(initialised ? Colours::green : Colours::red);
        g.fillEllipse(Rectangle<float>(200, 20, 10, 10));
        
        auto channelTextBounds = channel_select.getBounds().translated(-190, 0);
        auto paramModeTextBounds = shift_param_mode.getBounds().translated(-190, 0);
        
        g.setColour(Colours::black);
        g.drawText("CONNECTION STATUS", Rectangle<float>(10, 17, 200, 15), Justification::left);
        
        g.drawText("MIDI CHANNEL", channelTextBounds, Justification::left);
        g.drawText("SHIFT PARAMETER MODE", paramModeTextBounds, Justification::left);
        
        g.drawText("LFO DESTINATION 1", lfo_dest_1.getBounds().translated(-190, 0), Justification::left);
        g.drawText("LFO DESTINATION 2", lfo_dest_2.getBounds().translated(-190, 0), Justification::left);
        g.drawText("LFO DESTINATION 3", lfo_dest_3.getBounds().translated(-190, 0), Justification::left);
        
        int logoWidth = logo.getBounds().proportionOfWidth(0.1f);
        int logoHeight = logo.getBounds().proportionOfHeight(0.1f);
        Rectangle<int> logoBounds = {getWidth() - logoWidth - 10, getHeight() - logoHeight - 10, logoWidth, logoHeight};
        
        g.drawImage(logo, logoBounds.toFloat());
    }
    
    void resized() override
    {
        channel_select.setBounds(200, 50, 200, 20);
        shift_param_mode.setBounds(200, 80, 200, 20);
        
        lfo_dest_1.setBounds(200, 110, 200, 20);
        lfo_dest_2.setBounds(200, 140, 200, 20);
        lfo_dest_3.setBounds(200, 170, 200, 20);
    }
    
private:
    
    // These methods handle callbacks from the midi device + on-screen keyboard..
    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override
    {
        // Receive midi on message thread because it could interact with the gui
        MessageManager::callAsync([this, m = message]() mutable {
            receive_message(m);
        });
    }
    
    // Send messages to Recipher
    void send_message(const MessageType& message_type, std::vector<int> values = {}) {
        
        // Don't do anything if we're not initialised
        if(!initialised) return;
        
        auto total_size = values.size() + 3;
        
        // Create sysex message
        uint8_t message[4];
        
        // Write Recipher's ID
        message[0] = static_cast<uint8_t>(recipher_message_id_1);
        message[1] = static_cast<uint8_t>(recipher_message_id_2);
        
        // Write the message type
        message[2] = static_cast<uint8_t>(message_type);
        
        // Write values
        for(int i = 0; i < values.size(); i++) {
            message[3 + i] = static_cast<uint8_t>(values[i]);
        }
        
        // Create the MIDI message from data
        auto m = MidiMessage::createSysExMessage(message, static_cast<int>(total_size * sizeof(uint8_t)));
        
        // Send message from output
        if(auto* midiOut = device_manager.getDefaultMidiOutput()) {
            midiOut->sendMessageNow(m);
        }
    }
    
    // Process a MIDI message from Recipher
    void receive_message(MidiMessage& m) {
        
        // Check if sysex message
         if(m.isSysEx()) {
            
            int size = m.getSysExDataSize();
            if(size < 6) return;
            
            auto* data = m.getSysExData();
            
            // Check if first int contains the Recipger ID
            if(data[0] != recipher_message_id_1) return;
            if(data[1] != recipher_message_id_2) return;
            
            // Check message type
            int type = data[2];
            if(type == MessageType::Dump) {
                // Read the parameter dump and set values in gui
                int channel = data[3];
                int toggle = data[4];
                int dest_1 = data[5];
                int dest_2 = data[6];
                int dest_3 = data[7];
                
                channel_select.setSelectedId(channel);
                shift_param_mode.setSelectedId(toggle + 1);
                
                lfo_dest_1.setSelectedId(dest_1);
                lfo_dest_2.setSelectedId(dest_2);
                lfo_dest_3.setSelectedId(dest_3);
            }
        }
    }
    
    // Device Manager to hangle MIDI I/O
    AudioDeviceManager device_manager;
    
    // Comboboxes for channel and toggle switch behaviour (pickup or touch mode)
    ComboBox channel_select;
    ComboBox shift_param_mode;
    
    ComboBox lfo_dest_1;
    ComboBox lfo_dest_2;
    ComboBox lfo_dest_3;
    
    // Variable indiating connection status with Recipher
    bool initialised = false;
    
    // Last opened MIDI input so we can correctly close the port
    String last_input;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};



//==============================================================================
class RecipherToolApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    RecipherToolApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (RecipherToolApplication)
