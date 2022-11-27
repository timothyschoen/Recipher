cmake_minimum_required(VERSION 3.15)

project(RecipherSettingsTool VERSION 0.9)

add_subdirectory(JUCE)

juce_add_gui_app(RecipherSettingsTool
    ICON_BIG Source/cci_logo.png                 
    COMPANY_NAME "Clear Cut Instruments"                  # Specify the name of the app's author
    PRODUCT_NAME "Recipher Settings Tool")     # The name of the final executable, which can differ from the target name

juce_generate_juce_header(RecipherSettingsTool)

target_sources(RecipherSettingsTool
    PRIVATE
        Source/Main.cpp)

target_compile_definitions(RecipherSettingsTool
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RecipherSettingsTool,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RecipherSettingsTool,JUCE_VERSION>")

juce_add_binary_data(RecipherBinaryData SOURCES Source/clearcut.png Source/IBMPlexMono.ttf)

target_link_libraries(RecipherSettingsTool
    PRIVATE
        RecipherBinaryData
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
        
add_custom_command(TARGET RecipherSettingsTool POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "archive.zip" --format=zip)

