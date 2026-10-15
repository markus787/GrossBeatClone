cmake_minimum_required(VERSION 3.22)
project(GrossBeatClone VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GrossBeatClone
    COMPANY_NAME "Eigenbau"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Eigb
    PLUGIN_CODE Gbc1
    FORMATS VST3
    PRODUCT_NAME "GrossBeatClone"
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(GrossBeatClone PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/Curves.h)

target_compile_features(GrossBeatClone PRIVATE cxx_std_17)

target_compile_definitions(GrossBeatClone PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

target_link_libraries(GrossBeatClone
    PRIVATE juce::juce_audio_utils
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
