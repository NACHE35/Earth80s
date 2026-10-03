#pragma once
#include "PluginProcessor.h"

//==============================================================================
class EarthLNF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

//==============================================================================
// Menu cache (double-clic sur le titre)
class AboutPanel : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
private:
    juce::Rectangle<int> panelRect() const;
    juce::Rectangle<int> linkRect() const;
};

//==============================================================================
// Petit menu deroulant de presets
class PresetBar : public juce::Component
{
public:
    std::function<void()> onPrev, onNext, onMenu;
    juce::String text;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
};

//==============================================================================
class Earth80sEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Earth80sEditor (Earth80sProcessor&);
    ~Earth80sEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Un controle du panneau de droite (knob, micro-knob, interrupteur)
    struct Ctl
    {
        int param = 0;
        juce::String label;
        int tab = 0;
        int kind = 0;            // 0 knob, 1 micro-knob, 2 interrupteur etiquete, 3 interrupteur de section FX
        juce::Point<int> c;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::ToggleButton> button;
        std::unique_ptr<SA> sa;
        std::unique_ptr<BA> ba;
    };

    struct Caption { int tab; juce::String text; int y; };
    struct Section { juce::String name; juce::Rectangle<int> r; int enableParam; };

    void addCtl (int param, const char* label, int tab, int kind, int cx, int cy);
    void setTab (int t);
    void timerCallback() override;
    void buildGlobeGeometry();
    void renderGlobe (float rotationRad, float mix);
    float sampleMask (float fx, float fy) const;
    void drawMeter (juce::Graphics&, float y, float level);
    void drawRightPanel (juce::Graphics&);
    void drawWavePreview (juce::Graphics&, juce::Rectangle<int> r, int waveParam, int pwParam);
    juce::String valueText (int param) const;
    float rawValue (int param) const { return audio.apvts.getRawParameterValue (E80::kDefs[param].id)->load(); }

    void showPresetMenu();
    void stepPreset (int dir);
    void askSavePreset();

    Earth80sProcessor& audio;
    EarthLNF lnf;

    // Partie gauche
    juce::Slider volumeKnob, resoKnob, glideKnob, tuneKnob, cutoffRing;
    juce::ToggleButton powerBtn, hpBtn;
    std::unique_ptr<SA> volumeAtt, resoAtt, glideAtt, tuneAtt, cutoffAtt;
    std::unique_ptr<BA> powerAtt, hpAtt;
    PresetBar presetBar;
    AboutPanel about;

    // Partie droite
    std::vector<std::unique_ptr<Ctl>> ctls;
    std::vector<Caption> captions;
    std::vector<Section> sections;
    juce::Rectangle<int> tabRects[4];
    int currentTab = 0;
    juce::MidiKeyboardComponent keyboard;

    // Globe
    struct GlobePx { int x, y; float nx, nz, lonBase, lat, alpha; };
    std::vector<GlobePx> globePx;
    juce::Image globeImg;
    static constexpr int kGlobeD = 312;
    static constexpr float kGlobeR = 155.0f;
    static constexpr int kMaskW = 1024, kMaskH = 512;
    std::vector<std::uint8_t> mask;

    // Etoiles
    struct Star { float x, y, r, phase, speed, base; juce::Colour col; };
    std::vector<Star> stars;

    float rot = 0.0f, timeSec = 0.0f;
    double lastTime = 0.0;
    float dispL = 0.0f, dispR = 0.0f;
    int voiceMaskDisp = 0;

    juce::Rectangle<int> titleRect { 170, 36, 290, 52 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Earth80sEditor)
};
