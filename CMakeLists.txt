cmake_minimum_required(VERSION 3.22)

project(Earth80s VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(E80_FORMATS VST3 AU Standalone)
else()
    set(E80_FORMATS VST3 Standalone)
endif()

# Note : PRODUCT_NAME sans apostrophe pour eviter tout souci de nom de fichier ("Earth 80s.vst3").
# Le titre affiche dans l'interface reste "EARTH 80'S".
juce_add_plugin(Earth80s
    COMPANY_NAME "NACHE"
    PLUGIN_MANUFACTURER_CODE Nach
    PLUGIN_CODE Er80
    FORMATS ${E80_FORMATS}
    PRODUCT_NAME "Earth 80s"
    BUNDLE_ID com.nache.earth80s
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(Earth80s PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Presets.cpp)

target_compile_definitions(Earth80s PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Police optionnelle "Thriller" : si Resources/Thriller.ttf existe, elle est integree.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Thriller.ttf")
    juce_add_binary_data(Earth80sAssets SOURCES Resources/Thriller.ttf)
    target_link_libraries(Earth80s PRIVATE Earth80sAssets)
    target_compile_definitions(Earth80s PUBLIC EARTH_HAS_THRILLER_FONT=1)
endif()

target_link_libraries(Earth80s
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
