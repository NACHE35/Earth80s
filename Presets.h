#pragma once
#include <array>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"

// 1000 presets d'usine, generes de facon deterministe :
// 10 categories x 5 styles x 20 presets.
namespace E80
{
    constexpr int kNumCategories = 10;
    constexpr int kNumStyles = 5;
    constexpr int kPresetsPerStyle = 20;
    constexpr int kNumFactoryPresets = kNumCategories * kNumStyles * kPresetsPerStyle;   // 1000

    using PresetValues = std::array<float, (size_t) kNumParams>;

    const char* categoryName (int cat);
    const char* styleName (int style);
    juce::String presetName (int index);
    PresetValues makePreset (int index);
}
