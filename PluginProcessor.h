#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"
#include "Presets.h"
#include "Engine.h"
#include "Effects.h"

class Earth80sProcessor : public juce::AudioProcessor
{
public:
    Earth80sProcessor();
    ~Earth80sProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Earth 80's"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- Presets ----
    juce::String getCurrentPresetName() const;
    int getCurrentPresetIndex() const;       // -1 : preset utilisateur / par defaut
    void loadFactoryPreset (int index);
    void loadUserPreset (const juce::File& file);
    bool saveUserPreset (const juce::String& name);
    juce::Array<juce::File> getUserPresetFiles() const;
    static juce::File getUserPresetDir();

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };
    std::atomic<int> voiceMask { 0 };

private:
    void setParamReal (int index, float realValue);
    float pv (int i) const noexcept { return raw[(size_t) i]->load(); }
    void updateBlockParams();
    void renderVoices (float* L, float* R, int from, int to);
    void handleMidi (const juce::MidiMessage&);
    void noteOn (int note, float vel);
    void noteOff (int note);
    void allNotesOff();
    int findVoiceForNote();

    std::array<std::atomic<float>*, (size_t) E80::kNumParams> raw {};

    double sr = 44100.0;
    E80::BP bp;
    E80::Voice voices[E80::kVoices];
    std::vector<int> noteStack;
    unsigned long long ageCounter = 0;
    float lastPitch = 60.0f;
    int voiceMode = 0, lastVoiceMode = 0;
    bool sustainPedal = false;
    float bendNorm = 0.0f, modWheel = 0.0f, bendRange = 2.0f;
    float voiceGain = 0.7f;
    juce::Random rng { 20261002 };

    // LFO global
    double lfoPhase = 0.0, lfoInc = 0.0;
    int lfoShape = 1;
    float shValue = 0.0f;

    // Effets
    E80::Compressor comp;
    E80::PhaserFx phaser;
    E80::FlangerFx flanger;
    E80::ChorusFx chorus;
    E80::DelayFx delayFx;
    juce::Reverb reverb;

    juce::SmoothedValue<float> volS, powerS;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Earth80sProcessor)
};
