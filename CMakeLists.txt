cmake_minimum_required(VERSION 3.22)

project(LowFreqVST VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# Fetch JUCE 7.0.12 automatically. No submodules to manage, no manual JUCE
# install needed. CMake will download it during configure.
# ----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# ----------------------------------------------------------------------------
# Embed the filmstrip PNG (and any other assets) into the binary using
# JUCE's binary data generator, so we don't have to ship/load loose files
# at runtime. Anything placed in Assets/ gets compiled in.
# ----------------------------------------------------------------------------
juce_add_binary_data(LowFreqVSTAssets
    SOURCES
        Assets/KnobFilmstrip.png
)

# ----------------------------------------------------------------------------
# The actual plugin target.
# ----------------------------------------------------------------------------
juce_add_plugin(LowFreqVST
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ycmp
    PLUGIN_CODE Lfv1
    FORMATS VST3
    PRODUCT_NAME "LowFreqVST"
    VST3_CATEGORIES "Fx" "Filter"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(LowFreqVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/FilmstripRotarySlider.cpp
)

target_compile_definitions(LowFreqVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LowFreqVST
    PRIVATE
        LowFreqVSTAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
