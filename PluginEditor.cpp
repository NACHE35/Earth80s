#include "PluginEditor.h"
#include "EarthMap.h"
#include <cmath>

#if EARTH_HAS_THRILLER_FONT
 #include <BinaryData.h>
#endif

using namespace E80;

namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;
    constexpr float kStart = kPi * 1.25f;   // 7h30
    constexpr float kEnd   = kPi * 2.75f;   // 4h30

    // ---- Constantes faciles a modifier ----
    const char* kTitle    = "EARTH 80'S";
    const char* kVersion  = "1.0";
    const char* kDate     = "2 October 2026";
    const char* kLinkText = "Learn more about NACHE";
    const char* kLinkUrl  = "https://linktr.ee/nachevhs";

    const juce::Colour kAccent (0xff62c8ff);
    const juce::Colour kLabel  (0xff8fb8f0);

    constexpr int kLeftW = 633;      // partie gauche : identique a Earth Chorus / Earth Phaser
    constexpr int kWinW = 1190, kWinH = 640;

    juce::Font mkFont (float h, bool bold, float kern = 0.0f)
    {
        juce::Font f (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
        return f.withExtraKerningFactor (kern);
    }

    juce::Font signatureFont()
    {
       #if EARTH_HAS_THRILLER_FONT
        if (auto tf = juce::Typeface::createSystemTypefaceFor (BinaryData::Thriller_ttf, (size_t) BinaryData::Thriller_ttfSize))
            return juce::Font (juce::FontOptions (tf)).withHeight (18.0f);
       #endif
        juce::Font f (juce::FontOptions (juce::Font::getDefaultSerifFontName(), 17.0f, juce::Font::bold));
        return f.withExtraKerningFactor (0.14f);
    }

    float smoothstep (float a, float b, float x)
    {
        const float t = juce::jlimit (0.0f, 1.0f, (x - a) / (b - a));
        return t * t * (3.0f - 2.0f * t);
    }

    float textWidth (const juce::Font& f, const juce::String& s)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (f, s, 0.0f, 0.0f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }
}

//==============================================================================
void EarthLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                 float a0, float a1, juce::Slider& s)
{
    if (s.getComponentID() == "globe") return;   // dessine par l'editeur

    const int size = (int) s.getProperties()["size"];      // 0 grand, 1 mini, 2 micro
    const bool bipolar = (bool) s.getProperties()["bipolar"];

    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float ang = a0 + pos * (a1 - a0);
    const float aMid = 0.5f * (a0 + a1);
    const float R  = size == 0 ? 46.0f : (size == 1 ? 21.0f : 15.0f);
    const float Rt = size == 0 ? 55.0f : (size == 1 ? 27.0f : 19.5f);
    const float th = size == 0 ? 3.0f  : (size == 1 ? 2.4f  : 2.0f);

    juce::Path track;
    track.addCentredArc (cx, cy, Rt, Rt, 0.0f, a0, a1, true);
    g.setColour (juce::Colour (0x334a78c8));
    g.strokePath (track, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float from = bipolar ? aMid : a0;
    if (std::abs (ang - from) > 0.01f)
    {
        juce::Path val;
        val.addCentredArc (cx, cy, Rt, Rt, 0.0f, juce::jmin (from, ang), juce::jmax (from, ang), true);
        g.setColour (kAccent);
        g.strokePath (val, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    if (bipolar)
    {
        g.setColour (juce::Colour (0x88aacaff));
        g.fillEllipse (cx + std::sin (aMid) * Rt - 1.5f, cy - std::cos (aMid) * Rt - 1.5f, 3.0f, 3.0f);
    }

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b4a78), cx, cy - R,
                                             juce::Colour (0xff0b1226), cx, cy + R, false));
    g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
    g.setColour (juce::Colour (0x55a9c6ff));
    g.drawEllipse (cx - R, cy - R, 2 * R, 2 * R, 1.2f);

    const float r0 = size == 0 ? 14.0f : (size == 1 ? 6.0f : 4.5f);
    const float r1 = size == 0 ? 33.0f : (size == 1 ? 18.0f : 12.5f);
    juce::Path ind;
    ind.startNewSubPath (cx + std::sin (ang) * r0, cy - std::cos (ang) * r0);
    ind.lineTo (cx + std::sin (ang) * r1, cy - std::cos (ang) * r1);
    g.setColour (juce::Colours::white);
    g.strokePath (ind, juce::PathStrokeType (size == 0 ? 3.0f : (size == 1 ? 2.2f : 1.8f),
                                             juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void EarthLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const auto id = b.getComponentID();
    const bool on = b.getToggleState();

    if (id == "hp")   // selecteur LP | HP
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.5f);
        g.setColour (juce::Colour (0x33ffffff));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (juce::Colour (0x66aabbdd));
        g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.2f);

        auto half = r.withWidth (r.getWidth() * 0.5f);
        if (on) half = half.translated (r.getWidth() * 0.5f, 0.0f);
        g.setColour (juce::Colour (0x555ac8fa));
        g.fillRoundedRectangle (half.reduced (2.0f), half.getHeight() * 0.5f - 2.0f);
        g.setColour (kAccent);
        g.drawRoundedRectangle (half.reduced (2.0f), half.getHeight() * 0.5f - 2.0f, 1.2f);

        g.setFont (mkFont (11.0f, true, 0.1f));
        g.setColour (on ? juce::Colour (0xff8899bb) : juce::Colours::white);
        g.drawText ("LP", r.withWidth (r.getWidth() * 0.5f).toNearestInt(), juce::Justification::centred, false);
        g.setColour (on ? juce::Colours::white : juce::Colour (0xff8899bb));
        g.drawText ("HP", r.withTrimmedLeft (r.getWidth() * 0.5f).toNearestInt(), juce::Justification::centred, false);
        return;
    }

    auto r = b.getLocalBounds().toFloat().reduced (2.0f);
    const float rad = r.getHeight() * 0.5f;
    g.setColour (on ? juce::Colour (0x445ac8fa) : juce::Colour (0x33ffffff));
    g.fillRoundedRectangle (r, rad);
    g.setColour (on ? kAccent : juce::Colour (0x66aabbdd));
    g.drawRoundedRectangle (r, rad, 1.2f);
    const float d = r.getHeight() - 4.0f;
    const float kx = on ? r.getRight() - d - 2.0f : r.getX() + 2.0f;
    g.setColour (on ? juce::Colours::white : juce::Colour (0xff8899bb));
    g.fillEllipse (kx, r.getY() + 2.0f, d, d);
}

//==============================================================================
juce::Rectangle<int> AboutPanel::panelRect() const
{
    return juce::Rectangle<int> (0, 0, 380, 270).withCentre (getLocalBounds().getCentre());
}

juce::Rectangle<int> AboutPanel::linkRect() const
{
    return juce::Rectangle<int> (0, 0, 260, 26).withCentre ({ getWidth() / 2, panelRect().getY() + 190 });
}

void AboutPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xd8030812));
    auto p = panelRect().toFloat();
    g.setColour (juce::Colour (0xff0b1830));
    g.fillRoundedRectangle (p, 14.0f);
    g.setColour (kAccent.withAlpha (0.7f));
    g.drawRoundedRectangle (p, 14.0f, 1.5f);

    const int cx = getWidth() / 2, top = panelRect().getY();
    g.setColour (juce::Colour (0xffe4f0ff));
    g.setFont (mkFont (24.0f, true, 0.4f));
    g.drawText (kTitle, cx - 180, top + 24, 360, 34, juce::Justification::centred, false);

    g.setColour (kLabel);
    g.setFont (mkFont (12.0f, false, 0.05f));
    g.drawText ("Juno-inspired polysynth", cx - 180, top + 60, 360, 20, juce::Justification::centred, false);
    g.setFont (mkFont (15.0f, true));
    g.drawText (juce::String ("Version ") + kVersion, cx - 180, top + 94, 360, 22, juce::Justification::centred, false);
    g.setFont (mkFont (14.0f, false));
    g.drawText (juce::String ("Release date: ") + kDate, cx - 180, top + 120, 360, 22, juce::Justification::centred, false);

    auto lr = linkRect();
    g.setColour (kAccent);
    g.setFont (mkFont (16.0f, true));
    g.drawText (kLinkText, lr, juce::Justification::centred, false);
    g.drawLine ((float) lr.getX() + 30.0f, (float) lr.getBottom() - 2.0f, (float) lr.getRight() - 30.0f, (float) lr.getBottom() - 2.0f, 1.0f);

    g.setColour (juce::Colour (0x778fb8f0));
    g.setFont (mkFont (11.0f, false));
    g.drawText ("click anywhere else to close", cx - 180, top + 234, 360, 18, juce::Justification::centred, false);
}

void AboutPanel::mouseMove (const juce::MouseEvent& e)
{
    setMouseCursor (linkRect().contains (e.getPosition()) ? juce::MouseCursor::PointingHandCursor
                                                          : juce::MouseCursor::NormalCursor);
}

void AboutPanel::mouseDown (const juce::MouseEvent& e)
{
    if (linkRect().contains (e.getPosition()))
        juce::URL (kLinkUrl).launchInDefaultBrowser();
    else
        setVisible (false);
}

//==============================================================================
void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a2850), 0, r.getY(),
                                             juce::Colour (0xff0b1226), 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0x665ac8fa));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    const float cy = r.getCentreY();
    g.setColour (kLabel);
    juce::Path l, rt;
    l.addTriangle (r.getX() + 15, cy, r.getX() + 21, cy - 4.5f, r.getX() + 21, cy + 4.5f);
    rt.addTriangle (r.getRight() - 15, cy, r.getRight() - 21, cy - 4.5f, r.getRight() - 21, cy + 4.5f);
    g.fillPath (l); g.fillPath (rt);

    g.setColour (juce::Colour (0xffe6eefc));
    g.setFont (mkFont (13.0f, true, 0.05f));
    g.drawFittedText (text, getLocalBounds().reduced (30, 0).withTrimmedRight (10), juce::Justification::centred, 1);

    juce::Path caret;
    caret.addTriangle (r.getRight() - 40, cy - 2.0f, r.getRight() - 32, cy - 2.0f, r.getRight() - 36, cy + 3.0f);
    g.setColour (kAccent);
    g.fillPath (caret);
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (! contains (e.getPosition())) return;
    if (e.x < 26)                   { if (onPrev) onPrev(); }
    else if (e.x > getWidth() - 26) { if (onNext) onNext(); }
    else                            { if (onMenu) onMenu(); }
}

//==============================================================================
Earth80sEditor::Earth80sEditor (Earth80sProcessor& p)
    : juce::AudioProcessorEditor (&p), audio (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setOpaque (true);
    setSize (kWinW, kWinH);

    // ---------------- Partie gauche (meme disposition que Earth Chorus / Earth Phaser) ----------------
    auto setup = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (kStart, kEnd, true);
        s.setMouseDragSensitivity (220);
        s.setLookAndFeel (&lnf);
        s.getProperties().set ("size", 0);
        addAndMakeVisible (s);
    };
    setup (volumeKnob); setup (resoKnob); setup (glideKnob); setup (tuneKnob); setup (cutoffRing);
    cutoffRing.setComponentID ("globe");
    cutoffRing.setMouseDragSensitivity (300);
    tuneKnob.getProperties().set ("bipolar", true);

    powerBtn.setComponentID ("power");
    hpBtn.setComponentID ("hp");
    powerBtn.setLookAndFeel (&lnf);
    hpBtn.setLookAndFeel (&lnf);
    addAndMakeVisible (powerBtn);
    addAndMakeVisible (hpBtn);

    volumeAtt = std::make_unique<SA> (audio.apvts, "volume",  volumeKnob);
    resoAtt   = std::make_unique<SA> (audio.apvts, "reso",    resoKnob);
    glideAtt  = std::make_unique<SA> (audio.apvts, "glide",   glideKnob);
    tuneAtt   = std::make_unique<SA> (audio.apvts, "tune",    tuneKnob);
    cutoffAtt = std::make_unique<SA> (audio.apvts, "cutoff",  cutoffRing);
    powerAtt  = std::make_unique<BA> (audio.apvts, "power",   powerBtn);
    hpAtt     = std::make_unique<BA> (audio.apvts, "hp",      hpBtn);

    presetBar.onMenu = [this] { showPresetMenu(); };
    presetBar.onPrev = [this] { stepPreset (-1); };
    presetBar.onNext = [this] { stepPreset (+1); };
    addAndMakeVisible (presetBar);

    // ---------------- Partie droite : 4 onglets ----------------
    auto X = [] (int col) { return 714 + 79 * col; };
    const int r0 = 150, r1 = 258, r2 = 366, r3 = 474;

    // MASTER
    captions.push_back ({ 0, "AMPLIFIER  /  VOICE", r0 - 62 });
    captions.push_back ({ 0, "FILTER ENVELOPE  /  TRACKING", r1 - 62 });
    captions.push_back ({ 0, "LFO", r2 - 62 });
    captions.push_back ({ 0, "SOUND  /  PERFORMANCE", r3 - 62 });
    addCtl (P_aA, "ATTACK", 0, 0, X (0), r0);   addCtl (P_aD, "DECAY", 0, 0, X (1), r0);
    addCtl (P_aS, "SUSTAIN", 0, 0, X (2), r0);  addCtl (P_aR, "RELEASE", 0, 0, X (3), r0);
    addCtl (P_voice, "VOICE", 0, 0, X (4), r0); addCtl (P_uniDet, "UNI DETUNE", 0, 0, X (5), r0);
    addCtl (P_fA, "ATTACK", 0, 0, X (0), r1);   addCtl (P_fD, "DECAY", 0, 0, X (1), r1);
    addCtl (P_fS, "SUSTAIN", 0, 0, X (2), r1);  addCtl (P_fR, "RELEASE", 0, 0, X (3), r1);
    addCtl (P_envAmt, "ENV AMT", 0, 0, X (4), r1); addCtl (P_keyTrk, "KEY TRACK", 0, 0, X (5), r1);
    addCtl (P_lfoShape, "SHAPE", 0, 0, X (0), r2); addCtl (P_lfoRate, "RATE", 0, 0, X (1), r2);
    addCtl (P_lfoDelay, "DELAY", 0, 0, X (2), r2); addCtl (P_lfoPitch, "PITCH", 0, 0, X (3), r2);
    addCtl (P_lfoFilt, "FILTER", 0, 0, X (4), r2);
    addCtl (P_drive, "DRIVE", 0, 0, X (0), r3); addCtl (P_spread, "SPREAD", 0, 0, X (1), r3);
    addCtl (P_bend, "BEND RANGE", 0, 0, X (2), r3);

    // OSC 1 et OSC 2
    const int o0 = 250, o1 = 360;
    captions.push_back ({ 1, "OSCILLATOR 1", 76 });
    captions.push_back ({ 1, "PITCH  /  SHAPE", o0 - 62 });
    captions.push_back ({ 1, "LEVELS", o1 - 62 });
    addCtl (P_o1wave, "WAVE", 1, 0, X (0), o0);  addCtl (P_o1oct, "OCTAVE", 1, 0, X (1), o0);
    addCtl (P_o1semi, "SEMITONE", 1, 0, X (2), o0); addCtl (P_o1fine, "FINE", 1, 0, X (3), o0);
    addCtl (P_o1pw, "PULSE WIDTH", 1, 0, X (4), o0); addCtl (P_o1pwm, "PWM (LFO)", 1, 0, X (5), o0);
    addCtl (P_o1lvl, "LEVEL", 1, 0, X (0), o1);  addCtl (P_sub, "SUB OSC", 1, 0, X (1), o1);
    addCtl (P_noise, "NOISE", 1, 0, X (2), o1);

    captions.push_back ({ 2, "OSCILLATOR 2", 76 });
    captions.push_back ({ 2, "PITCH  /  SHAPE", o0 - 62 });
    captions.push_back ({ 2, "LEVELS  /  SYNC", o1 - 62 });
    addCtl (P_o2wave, "WAVE", 2, 0, X (0), o0);  addCtl (P_o2oct, "OCTAVE", 2, 0, X (1), o0);
    addCtl (P_o2semi, "SEMITONE", 2, 0, X (2), o0); addCtl (P_o2fine, "DETUNE", 2, 0, X (3), o0);
    addCtl (P_o2pw, "PULSE WIDTH", 2, 0, X (4), o0); addCtl (P_o2pwm, "PWM (LFO)", 2, 0, X (5), o0);
    addCtl (P_o2lvl, "LEVEL", 2, 0, X (0), o1);  addCtl (P_o2env, "ENV > PITCH", 2, 0, X (1), o1);
    addCtl (P_sync, "SYNC", 2, 2, X (2), o1);

    // FX : 6 sections (2 colonnes x 3 lignes), dans l'ordre du signal
    struct FxSec { const char* name; int enable; int col, row; std::vector<std::pair<int, const char*>> knobs; };
    const std::vector<FxSec> fx = {
        { "COMPRESSOR", P_fxComp,   0, 0, { { P_cThr, "THRESH" }, { P_cRatio, "RATIO" }, { P_cAtk, "ATTACK" }, { P_cRel, "RELEASE" }, { P_cGain, "MAKEUP" } } },
        { "PHASER",     P_fxPhase,  1, 0, { { P_phRate, "RATE" }, { P_phDepth, "DEPTH" }, { P_phFb, "FEEDBACK" }, { P_phMix, "MIX" } } },
        { "FLANGER",    P_fxFlange, 0, 1, { { P_flRate, "RATE" }, { P_flDepth, "DEPTH" }, { P_flFb, "FEEDBACK" }, { P_flMix, "MIX" } } },
        { "CHORUS",     P_fxChorus, 1, 1, { { P_chMode, "MODE" }, { P_chDepth, "DEPTH" }, { P_chMix, "MIX" } } },
        { "DELAY",      P_fxDelay,  0, 2, { { P_dlTime, "TIME" }, { P_dlFb, "FEEDBACK" }, { P_dlTone, "TONE" }, { P_dlMix, "MIX" } } },
        { "REVERB",     P_fxRev,    1, 2, { { P_rvSize, "SIZE" }, { P_rvDamp, "DAMPING" }, { P_rvMix, "MIX" } } }
    };
    for (const auto& s : fx)
    {
        const int x0 = 675 + s.col * 240, y0 = 72 + s.row * 150;
        sections.push_back ({ s.name, { x0, y0, 235, 138 }, s.enable });
        addCtl (s.enable, "", 3, 3, x0 + 235 - 26, y0 + 17);
        int i = 0;
        for (auto& k : s.knobs) { addCtl (k.first, k.second, 3, 1, x0 + 24 + i * 47, y0 + 82); ++i; }
    }

    // Clavier a l'ecran (pratique en Standalone)
    keyboard.setAvailableRange (36, 96);
    keyboard.setKeyWidth (475.0f / 36.0f);
    keyboard.setScrollButtonsVisible (false);
    keyboard.setLowestVisibleKey (36);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffdfe9ff));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff0a1226));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff1b2a4a));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour (0x3362c8ff));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0x9962c8ff));
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, juce::Colour (0xff1b2a4a));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (keyboard);

    about.setVisible (false);
    addChildComponent (about);

    // Etoiles (sur toute la fenetre)
    juce::Random rnd (20261002);
    for (int i = 0; i < 300; ++i)
    {
        Star s;
        s.x = rnd.nextFloat() * (float) kWinW;
        s.y = rnd.nextFloat() * (float) kWinH;
        s.r = 0.7f + rnd.nextFloat() * 1.3f;
        s.phase = rnd.nextFloat() * 6.28f;
        s.speed = 0.6f + rnd.nextFloat() * 3.2f;
        s.base = 0.45f + rnd.nextFloat() * 0.55f;
        const float k = rnd.nextFloat();
        s.col = k < 0.2f ? juce::Colour (0xffffe2b8) : (k < 0.5f ? juce::Colour (0xffb8d4ff) : juce::Colours::white);
        stars.push_back (s);
    }

    mask = EarthMap::buildMask (kMaskW, kMaskH);
    globeImg = juce::Image (juce::Image::ARGB, kGlobeD, kGlobeD, true);
    buildGlobeGeometry();

    resized();
    setTab (0);
    lastTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    renderGlobe (0.0f, (float) cutoffRing.valueToProportionOfLength (cutoffRing.getValue()));
    startTimerHz (30);
}

Earth80sEditor::~Earth80sEditor()
{
    stopTimer();
    for (auto* s : { &volumeKnob, &resoKnob, &glideKnob, &tuneKnob, &cutoffRing })
        s->setLookAndFeel (nullptr);
    powerBtn.setLookAndFeel (nullptr);
    hpBtn.setLookAndFeel (nullptr);
    for (auto& c : ctls)
    {
        if (c->slider) c->slider->setLookAndFeel (nullptr);
        if (c->button) c->button->setLookAndFeel (nullptr);
    }
}

void Earth80sEditor::addCtl (int param, const char* label, int tab, int kind, int cx, int cy)
{
    auto c = std::make_unique<Ctl>();
    c->param = param; c->label = label; c->tab = tab; c->kind = kind; c->c = { cx, cy };
    const auto& d = kDefs[param];

    if (kind == 2 || kind == 3)
    {
        c->button = std::make_unique<juce::ToggleButton>();
        c->button->setComponentID (kind == 2 ? "pill" : "fx");
        c->button->setLookAndFeel (&lnf);
        addChildComponent (*c->button);
        c->ba = std::make_unique<BA> (audio.apvts, d.id, *c->button);
    }
    else
    {
        c->slider = std::make_unique<juce::Slider>();
        auto& s = *c->slider;
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (kStart, kEnd, true);
        s.setMouseDragSensitivity (d.kind == Kind::Choice ? 110 : 180);
        s.setLookAndFeel (&lnf);
        s.getProperties().set ("size", kind == 0 ? 1 : 2);
        if (d.bipolar) s.getProperties().set ("bipolar", true);
        addChildComponent (s);
        c->sa = std::make_unique<SA> (audio.apvts, d.id, s);
    }
    ctls.push_back (std::move (c));
}

void Earth80sEditor::setTab (int t)
{
    currentTab = t;
    for (auto& c : ctls)
    {
        const bool vis = (c->tab == t);
        if (c->slider) c->slider->setVisible (vis);
        if (c->button) c->button->setVisible (vis);
    }
    repaint();
}

void Earth80sEditor::resized()
{
    auto place = [] (juce::Component& c, int cx, int cy, int w, int h) { c.setBounds (cx - w / 2, cy - h / 2, w, h); };
    place (volumeKnob, 79, 151, 120, 120);
    place (resoKnob,  551, 151, 120, 120);
    place (glideKnob,  79, 533, 120, 120);
    place (tuneKnob,  551, 533, 120, 120);
    place (cutoffRing, 316, 337, 340, 340);
    powerBtn.setBounds (587, 12, 38, 20);
    place (hpBtn, 551, 312, 64, 26);
    presetBar.setBounds (216, 598, 200, 26);

    for (auto& c : ctls)
    {
        if (c->slider) place (*c->slider, c->c.x, c->c.y, c->kind == 0 ? 64 : 46, c->kind == 0 ? 64 : 46);
        if (c->button) place (*c->button, c->c.x, c->c.y, c->kind == 2 ? 64 : 34, c->kind == 2 ? 22 : 16);
    }
    keyboard.setBounds (675, 552, 475, 62);

    for (int i = 0; i < 4; ++i) tabRects[i] = { 675 + i * 120, 28, 112, 26 };
    about.setBounds (getLocalBounds());
}

void Earth80sEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getEventRelativeTo (this).getPosition();
    for (int i = 0; i < 4; ++i)
        if (tabRects[i].contains (p)) { setTab (i); return; }
}

void Earth80sEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (titleRect.contains (e.getEventRelativeTo (this).getPosition()))
    {
        about.setVisible (true);
        about.toFront (true);
    }
}

//==============================================================================
// Globe
void Earth80sEditor::buildGlobeGeometry()
{
    globePx.clear();
    const float tilt = juce::degreesToRadians (20.0f);
    const float ct = std::cos (tilt), st = std::sin (tilt);
    const float c0 = (float) kGlobeD * 0.5f;

    for (int y = 0; y < kGlobeD; ++y)
        for (int x = 0; x < kGlobeD; ++x)
        {
            const float dx = (float) x + 0.5f - c0, dy = (float) y + 0.5f - c0;
            const float dist = std::sqrt (dx * dx + dy * dy);
            if (dist > kGlobeR + 1.0f) continue;

            float nx = dx / kGlobeR, ny = -dy / kGlobeR;
            const float len = std::sqrt (nx * nx + ny * ny);
            if (len > 1.0f) { nx /= len; ny /= len; }
            const float nz = std::sqrt (juce::jmax (0.0f, 1.0f - nx * nx - ny * ny));

            const float yy = ny * ct + nz * st;
            const float zz = -ny * st + nz * ct;

            GlobePx p;
            p.x = x; p.y = y; p.nx = nx; p.nz = nz;
            p.lat = std::asin (juce::jlimit (-1.0f, 1.0f, yy));
            p.lonBase = std::atan2 (nx, zz);
            p.alpha = juce::jlimit (0.0f, 1.0f, kGlobeR + 0.5f - dist);
            globePx.push_back (p);
        }
}

float Earth80sEditor::sampleMask (float fx, float fy) const
{
    int x0 = (int) std::floor (fx), y0 = (int) std::floor (fy);
    const float tx = fx - (float) x0, ty = fy - (float) y0;
    int x1 = x0 + 1;
    x0 = ((x0 % kMaskW) + kMaskW) % kMaskW;
    x1 = ((x1 % kMaskW) + kMaskW) % kMaskW;
    int y1 = juce::jlimit (0, kMaskH - 1, y0 + 1);
    y0 = juce::jlimit (0, kMaskH - 1, y0);
    const float a = mask[(size_t) (y0 * kMaskW + x0)], b = mask[(size_t) (y0 * kMaskW + x1)];
    const float c = mask[(size_t) (y1 * kMaskW + x0)], d = mask[(size_t) (y1 * kMaskW + x1)];
    const float top = a + (b - a) * tx, bot = c + (d - c) * tx;
    return (top + (bot - top) * ty) / 255.0f;
}

void Earth80sEditor::renderGlobe (float rotRad, float mix)
{
    // Le soleil passe de "derriere la Terre" (filtre ferme = nuit totale) a "face a nous" (filtre ouvert = jour complet)
    const float sunA = (1.0f - juce::jlimit (0.0f, 1.0f, mix)) * kPi;
    const float sx = std::sin (sunA), sz = std::cos (sunA);
    const float centerLon = juce::degreesToRadians (100.0f);
    const float twoPi = 2.0f * kPi;

    juce::Image::BitmapData bd (globeImg, juce::Image::BitmapData::writeOnly);

    for (const auto& p : globePx)
    {
        const float lon = p.lonBase + centerLon - rotRad;
        float u = lon / twoPi + 0.5f;
        u -= std::floor (u);
        const float fx = u * (float) kMaskW;
        const float fy = (0.5f - p.lat / kPi) * (float) (kMaskH - 1);
        const float land = sampleMask (fx, fy);

        const float n = 0.5f + 0.5f * std::sin (lon * 9.0f + p.lat * 4.0f) * std::cos (p.lat * 13.0f + lon * 3.0f);
        const float ice = smoothstep (1.05f, 1.25f, std::abs (p.lat));
        float lr = 0.20f + 0.25f * n, lg = 0.52f + 0.03f * n, lb = 0.22f + 0.03f * n;
        lr += (0.92f - lr) * ice; lg += (0.96f - lg) * ice; lb += (1.0f - lb) * ice;

        const float oz = std::pow (p.nz, 0.7f);
        float orr = 0.03f + 0.10f * oz, og = 0.16f + 0.29f * oz, ob = 0.41f + 0.40f * oz;
        const float oice = smoothstep (1.30f, 1.45f, std::abs (p.lat));
        orr += (0.85f - orr) * oice; og += (0.92f - og) * oice; ob += (1.0f - ob) * oice;

        float r = orr + (lr - orr) * land;
        float g = og  + (lg - og)  * land;
        float b = ob  + (lb - ob)  * land;

        const float d = p.nx * sx + p.nz * sz;
        const float lit = smoothstep (-0.05f, 0.35f, d);
        float inten = 0.05f + lit * (0.4f + 0.6f * juce::jmax (0.0f, d));
        inten = juce::jmin (1.0f, inten);
        r *= inten; g *= inten; b *= inten;

        const float hx = sx, hz = sz + 1.0f;
        const float hl = std::sqrt (hx * hx + hz * hz) + 1.0e-4f;
        const float spec = std::pow (juce::jmax (0.0f, (p.nx * hx + p.nz * hz) / hl), 70.0f) * 0.55f * lit * (1.0f - land);
        r += spec; g += spec; b += spec;

        const float dusk = std::exp (-(d * d) / 0.012f) * 0.16f;
        r += dusk * 0.9f; g += dusk * 0.45f; b += dusk * 0.15f;
        const float rim = std::pow (1.0f - p.nz, 2.5f) * (0.15f + 0.85f * lit) * 0.9f;
        r += 0.25f * rim; g += 0.55f * rim; b += 1.0f * rim;

        bd.setPixelColour (p.x, p.y, juce::Colour::fromFloatRGBA (juce::jlimit (0.0f, 1.0f, r),
                                                                  juce::jlimit (0.0f, 1.0f, g),
                                                                  juce::jlimit (0.0f, 1.0f, b), p.alpha));
    }
}

//==============================================================================
void Earth80sEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float dt = juce::jlimit (0.0f, 0.2f, (float) (now - lastTime));
    lastTime = now;
    timeSec = (float) now;

    rot += dt * juce::degreesToRadians (6.0f + 1.5f * rawValue (P_lfoRate));

    const float lv = audio.levelL.exchange (0.0f), rv = audio.levelR.exchange (0.0f);
    dispL = juce::jmax (lv, dispL * 0.86f);
    dispR = juce::jmax (rv, dispR * 0.86f);
    voiceMaskDisp = audio.voiceMask.load();

    renderGlobe (rot, (float) cutoffRing.valueToProportionOfLength (cutoffRing.getValue()));
    presetBar.text = audio.getCurrentPresetName();
    repaint();
}

juce::String Earth80sEditor::valueText (int param) const
{
    const auto& d = kDefs[param];
    const float v = rawValue (param);
    auto sgn = [] (float x) { return x > 0.0f ? juce::String ("+") : juce::String(); };

    switch (d.unit)
    {
        case Unit::Percent:
        {
            const int n = (int) std::lround (v * 100.0f);
            return (d.bipolar ? sgn ((float) n) : juce::String()) + juce::String (n) + "%";
        }
        case Unit::Hz:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 1) + " kHz";
            if (v >= 100.0f)  return juce::String ((int) std::lround (v)) + " Hz";
            if (v >= 10.0f)   return juce::String (v, 1) + " Hz";
            return juce::String (v, 2) + " Hz";
        case Unit::Sec:
            if (param == P_glide && v < 0.005f) return "OFF";
            if (v < 1.0f) return juce::String ((int) std::lround (v * 1000.0f)) + " ms";
            return juce::String (v, 2) + " s";
        case Unit::Ms:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 2) + " s";
            return juce::String ((int) std::lround (v)) + " ms";
        case Unit::Cents:  return sgn (v) + juce::String (v, 1) + " ct";
        case Unit::Semi:   return (d.bipolar ? sgn (v) : juce::String()) + juce::String ((int) std::lround (v)) + " st";
        case Unit::Db:     return juce::String (v, 1) + " dB";
        case Unit::Ratio:  return juce::String (v, 1) + ":1";
        case Unit::Choice: return audio.apvts.getParameter (d.id)->getCurrentValueAsText();
        default:           return juce::String (v, 2);
    }
}

void Earth80sEditor::drawMeter (juce::Graphics& g, float y, float level)
{
    const int segs = 28;
    const float w = 6.0f, gap = 1.8f;
    const float total = (float) segs * (w + gap) - gap;
    const float x0 = ((float) kLeftW - total) * 0.5f;
    const float db = juce::Decibels::gainToDecibels (level, -60.0f);
    for (int i = 0; i < segs; ++i)
    {
        const float thr = -48.0f + (float) i * (48.0f / (float) (segs - 1));
        const juce::Colour c = i < 20 ? juce::Colour (0xff3ddc84) : (i < 25 ? juce::Colour (0xffffb020) : juce::Colour (0xffff3b30));
        g.setColour (db >= thr ? c : c.withAlpha (0.14f));
        g.fillRoundedRectangle (x0 + (float) i * (w + gap), y, w, 5.0f, 1.0f);
    }
}

void Earth80sEditor::drawWavePreview (juce::Graphics& g, juce::Rectangle<int> r, int waveParam, int pwParam)
{
    g.setColour (juce::Colour (0x33101c3a));
    g.fillRoundedRectangle (r.toFloat(), 8.0f);
    g.setColour (juce::Colour (0x445ac8fa));
    g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

    const int wave = (int) std::lround (rawValue (waveParam));
    const float pw = rawValue (pwParam);
    const float cy = (float) r.getCentreY(), amp = (float) r.getHeight() * 0.34f;

    g.setColour (juce::Colour (0x22aadcff));
    g.drawLine ((float) r.getX() + 8.0f, cy, (float) r.getRight() - 8.0f, cy, 1.0f);

    juce::Path p;
    const int N = r.getWidth() - 24;
    for (int i = 0; i <= N; ++i)
    {
        const float t = std::fmod ((float) i / (float) N * 2.0f, 1.0f);
        const float saw = 2.0f * t - 1.0f, pul = t < pw ? 1.0f : -1.0f;
        float y;
        switch (wave)
        {
            case 0:  y = saw; break;
            case 1:  y = pul; break;
            case 2:  y = 0.5f * (saw + pul); break;
            case 3:  y = 4.0f * std::abs (t - 0.5f) - 1.0f; break;
            default: y = std::sin (2.0f * kPi * t); break;
        }
        const float px = (float) r.getX() + 12.0f + (float) i, py = cy - y * amp;
        if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
    }
    g.setColour (kAccent.withAlpha (0.22f));
    g.strokePath (p, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kAccent);
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void Earth80sEditor::drawRightPanel (juce::Graphics& g)
{
    const juce::Rectangle<float> panel (655.0f, 18.0f, 515.0f, 604.0f);
    g.setColour (juce::Colour (0x66070d1e));
    g.fillRoundedRectangle (panel, 14.0f);
    g.setColour (kAccent.withAlpha (0.3f));
    g.drawRoundedRectangle (panel, 14.0f, 1.2f);

    // Onglets
    static const char* names[4] = { "MASTER", "OSC 1", "OSC 2", "FX" };
    for (int i = 0; i < 4; ++i)
    {
        const auto r = tabRects[i].toFloat();
        const bool act = (i == currentTab);
        if (act)
        {
            g.setColour (kAccent.withAlpha (0.18f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (kAccent.withAlpha (0.9f));
            g.drawRoundedRectangle (r, 8.0f, 1.3f);
        }
        else
        {
            g.setColour (juce::Colour (0x445ac8fa));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
        }
        g.setColour (act ? juce::Colours::white : kLabel);
        g.setFont (mkFont (12.0f, true, 0.25f));
        g.drawText (names[i], tabRects[i], juce::Justification::centred, false);
    }
    g.setColour (juce::Colour (0x225ac8fa));
    g.drawLine (675.0f, 62.0f, 1150.0f, 62.0f, 1.0f);

    // Legendes de sections
    const auto capFont = mkFont (10.0f, true, 0.3f);
    for (const auto& c : captions)
    {
        if (c.tab != currentTab) continue;
        g.setFont (capFont);
        g.setColour (kLabel.withAlpha (0.85f));
        g.drawText (c.text, 675, c.y - 8, 400, 16, juce::Justification::centredLeft, false);
        const float tw = textWidth (capFont, c.text);
        g.setColour (juce::Colour (0x225ac8fa));
        g.drawLine (675.0f + tw + 10.0f, (float) c.y, 1150.0f, (float) c.y, 1.0f);
    }

    // Apercu de forme d'onde
    if (currentTab == 1) drawWavePreview (g, { 675, 90, 475, 84 }, P_o1wave, P_o1pw);
    if (currentTab == 2) drawWavePreview (g, { 675, 90, 475, 84 }, P_o2wave, P_o2pw);

    // Sections d'effets
    if (currentTab == 3)
        for (const auto& s : sections)
        {
            const bool on = rawValue (s.enableParam) > 0.5f;
            g.setColour (juce::Colour (on ? 0x44101c3a : 0x22101c3a));
            g.fillRoundedRectangle (s.r.toFloat(), 10.0f);
            g.setColour (kAccent.withAlpha (on ? 0.45f : 0.2f));
            g.drawRoundedRectangle (s.r.toFloat(), 10.0f, 1.0f);
            g.setColour (on ? juce::Colour (0xffe4f0ff) : kLabel.withAlpha (0.6f));
            g.setFont (mkFont (11.5f, true, 0.3f));
            g.drawText (s.name, s.r.getX() + 12, s.r.getY() + 7, 150, 20, juce::Justification::centredLeft, false);
        }

    // Etiquettes et valeurs
    for (const auto& c : ctls)
    {
        if (c->tab != currentTab || c->kind == 3) continue;
        g.setColour (kLabel);
        if (c->kind == 1)
        {
            g.setFont (mkFont (8.5f, true, 0.05f));
            g.drawText (c->label, c->c.x - 30, c->c.y - 36, 60, 14, juce::Justification::centred, false);
            g.setColour (juce::Colour (0xffe6eefc));
            g.setFont (mkFont (10.0f, false));
            g.drawText (valueText (c->param), c->c.x - 30, c->c.y + 20, 60, 14, juce::Justification::centred, false);
        }
        else
        {
            g.setFont (mkFont (10.0f, true, 0.1f));
            g.drawText (c->label, c->c.x - 39, c->c.y - 42, 78, 16, juce::Justification::centred, false);
            if (c->kind == 0)
            {
                g.setColour (juce::Colour (0xffe6eefc));
                g.setFont (mkFont (11.5f, false));
                g.drawText (valueText (c->param), c->c.x - 39, c->c.y + 28, 78, 16, juce::Justification::centred, false);
            }
        }
    }
}

void Earth80sEditor::paint (juce::Graphics& g)
{
    const float H = (float) getHeight();
    const float cx = 316.0f, cy = 337.0f;

    // Fond
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff02030a), 0, 0, juce::Colour (0xff0a1735), 0, H, false));
    g.fillAll();

    // Etoiles scintillantes
    for (const auto& s : stars)
    {
        float tw = 0.5f + 0.5f * std::sin (timeSec * s.speed + s.phase);
        tw *= tw;
        const float a = juce::jlimit (0.0f, 1.0f, s.base * (0.12f + 0.88f * tw));
        g.setColour (s.col.withAlpha (a));
        g.fillEllipse (s.x - s.r * 0.5f, s.y - s.r * 0.5f, s.r, s.r);
        if (s.r > 1.5f && tw > 0.75f)
        {
            g.setColour (s.col.withAlpha (a * 0.5f));
            g.drawLine (s.x - 3.5f, s.y, s.x + 3.5f, s.y, 0.7f);
            g.drawLine (s.x, s.y - 3.5f, s.x, s.y + 3.5f, 0.7f);
        }
    }

    // Halo
    {
        juce::ColourGradient glow (juce::Colour (0x005ac8fa), cx, cy, juce::Colour (0x005ac8fa), cx + 225.0f, cy, true);
        glow.addColour (0.70, juce::Colour (0x345ac8fa));
        g.setGradientFill (glow);
        g.fillEllipse (cx - 225.0f, cy - 225.0f, 450.0f, 450.0f);
    }

    // Anneau : cutoff du filtre MS-20
    {
        const float pr = (float) cutoffRing.valueToProportionOfLength (cutoffRing.getValue());
        const float ang = kStart + pr * (kEnd - kStart);
        const float Rr = 170.0f;

        juce::Path track;
        track.addCentredArc (cx, cy, Rr, Rr, 0.0f, kStart, kEnd, true);
        g.setColour (juce::Colour (0x30507ab8));
        g.strokePath (track, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (pr > 0.001f)
        {
            juce::Path val;
            val.addCentredArc (cx, cy, Rr, Rr, 0.0f, kStart, ang, true);
            g.setColour (kAccent.withAlpha (0.25f));
            g.strokePath (val, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (kAccent);
            g.strokePath (val, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (juce::Colours::white);
        g.fillEllipse (cx + std::sin (ang) * Rr - 5.0f, cy - std::cos (ang) * Rr - 5.0f, 10.0f, 10.0f);
    }

    // Globe
    g.drawImageAt (globeImg, (int) (cx - (float) kGlobeD * 0.5f), (int) (cy - (float) kGlobeD * 0.5f));
    g.setColour (juce::Colour (0x66aadcff));
    g.drawEllipse (cx - kGlobeR, cy - kGlobeR, kGlobeR * 2, kGlobeR * 2, 1.2f);
    g.setColour (juce::Colour (0x22aadcff));
    g.drawEllipse (cx - kGlobeR - 5.0f, cy - kGlobeR - 5.0f, (kGlobeR + 5.0f) * 2, (kGlobeR + 5.0f) * 2, 1.0f);

    // VU-metre (style Xone : rangees de LED horizontales)
    drawMeter (g, 9.0f, dispL);
    drawMeter (g, 16.0f, dispR);
    g.setColour (juce::Colour (0x998fb8f0));
    g.setFont (mkFont (8.0f, true));
    const float mx0 = ((float) kLeftW - (28.0f * 7.8f - 1.8f)) * 0.5f;
    g.drawText ("L", (int) mx0 - 14, 8, 10, 7, juce::Justification::centredRight, false);
    g.drawText ("R", (int) mx0 - 14, 15, 10, 7, juce::Justification::centredRight, false);

    // Titre
    const juce::Rectangle<int> tr (0, 36, kLeftW, 34);
    g.setFont (mkFont (24.0f, true, 0.55f));
    g.setColour (kAccent.withAlpha (0.28f));
    g.drawText (kTitle, tr.translated (0, 1), juce::Justification::centred, false);
    g.setColour (juce::Colour (0xffe4f0ff));
    g.drawText (kTitle, tr, juce::Justification::centred, false);

    // "by NACHE"
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffc46b), 0, 71.0f, juce::Colour (0xffe2431d), 0, 89.0f, false));
    g.setFont (signatureFont());
    g.drawText ("by NACHE", juce::Rectangle<int> (0, 70, kLeftW, 22), juce::Justification::centred, false);

    // Libelles des 4 boutons de coin
    auto label = [&] (const char* t, int x, int y)
    {
        g.setColour (kLabel);
        g.setFont (mkFont (11.5f, true, 0.12f));
        g.drawText (t, x - 60, y - 9, 120, 18, juce::Justification::centred, false);
    };
    auto value = [&] (const juce::String& t, int x, int y)
    {
        g.setColour (juce::Colour (0xffe6eefc));
        g.setFont (mkFont (13.0f, false));
        g.drawText (t, x - 60, y - 9, 120, 18, juce::Justification::centred, false);
    };
    label ("VOLUME", 79, 102);       value (valueText (P_volume), 79, 255);
    label ("RESONANCE", 551, 102);   value (valueText (P_reso), 551, 255);
    label ("PORTAMENTO", 79, 456);   value (valueText (P_glide), 79, 609);
    label ("TUNE", 551, 456);        value (valueText (P_tune), 551, 609);

    // Interrupteur LP/HP + voyants de voix
    g.setColour (kLabel);
    g.setFont (mkFont (9.5f, true, 0.15f));
    g.drawText ("FILTER TYPE", 551 - 50, 281, 100, 14, juce::Justification::centred, false);
    g.drawText ("VOICES", 79 - 50, 281, 100, 14, juce::Justification::centred, false);
    for (int i = 0; i < kVoices; ++i)
    {
        const bool on = (voiceMaskDisp & (1 << i)) != 0;
        const float lx = 79.0f - 50.0f + 10.0f + (float) i * 16.0f, ly = 312.0f;
        if (on) { g.setColour (kAccent.withAlpha (0.3f)); g.fillEllipse (lx - 6.0f, ly - 6.0f, 12.0f, 12.0f); }
        g.setColour (on ? kAccent : juce::Colour (0x44506890));
        g.fillEllipse (lx - 3.5f, ly - 3.5f, 7.0f, 7.0f);
    }

    // Etiquette de l'anneau (a la place de "DRY / WET")
    g.setColour (kLabel);
    g.setFont (mkFont (10.5f, true, 0.35f));
    g.drawText ("CUTOFF", (int) cx - 80, 523, 160, 18, juce::Justification::centred, false);
    g.setColour (juce::Colour (0xffe6eefc));
    g.setFont (mkFont (12.0f, false));
    g.drawText (valueText (P_cutoff), (int) cx - 80, 541, 160, 18, juce::Justification::centred, false);

    drawRightPanel (g);
}

//==============================================================================
// Presets : Categorie > Style > Preset (1000 presets d'usine) + presets utilisateur
void Earth80sEditor::stepPreset (int dir)
{
    const int cur = audio.getCurrentPresetIndex();
    const int N = kNumFactoryPresets;
    const int next = cur < 0 ? (dir > 0 ? 0 : N - 1) : (cur + dir + N) % N;
    audio.loadFactoryPreset (next);
}

void Earth80sEditor::showPresetMenu()
{
    auto users = audio.getUserPresetFiles();
    const int cur = audio.getCurrentPresetIndex();
    const auto curName = audio.getCurrentPresetName();

    juce::PopupMenu menu;
    for (int c = 0; c < kNumCategories; ++c)
    {
        juce::PopupMenu catMenu;
        for (int s = 0; s < kNumStyles; ++s)
        {
            juce::PopupMenu styleMenu;
            for (int i = 0; i < kPresetsPerStyle; ++i)
            {
                const int idx = c * 100 + s * 20 + i;
                styleMenu.addItem (1 + idx, presetName (idx), true, idx == cur);
            }
            catMenu.addSubMenu (styleName (s), styleMenu);
        }
        menu.addSubMenu (categoryName (c), catMenu);
    }

    juce::PopupMenu userMenu, deleteMenu;
    for (int j = 0; j < users.size(); ++j)
    {
        const auto n = users[j].getFileNameWithoutExtension();
        userMenu.addItem (2000 + j, n, true, cur < 0 && n == curName);
        deleteMenu.addItem (3000 + j, n);
    }
    menu.addSeparator();
    if (users.size() > 0) menu.addSubMenu ("User presets", userMenu);
    menu.addItem (5000, "Save current settings as...");
    if (users.size() > 0) menu.addSubMenu ("Delete user preset", deleteMenu);

    juce::Component::SafePointer<Earth80sEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBar),
                        [safe, users] (int r)
    {
        if (safe == nullptr || r == 0) return;
        auto& a = safe->audio;
        if (r >= 1 && r <= kNumFactoryPresets)   a.loadFactoryPreset (r - 1);
        else if (r >= 2000 && r < 3000)          a.loadUserPreset (users[r - 2000]);
        else if (r >= 3000 && r < 4000)          users[r - 3000].deleteFile();
        else if (r == 5000)                      safe->askSavePreset();
    });
}

void Earth80sEditor::askSavePreset()
{
    auto* w = new juce::AlertWindow ("Save preset", "Give a name to your preset:", juce::MessageBoxIconType::NoIcon);
    const auto cur = audio.getCurrentPresetName();
    w->addTextEditor ("name", (cur == "Init" || audio.getCurrentPresetIndex() >= 0) ? juce::String ("My Preset") : cur, "Name");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<Earth80sEditor> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int result)
    {
        if (result == 1 && safe != nullptr)
        {
            const auto n = w->getTextEditorContents ("name").trim();
            if (n.isNotEmpty()) safe->audio.saveUserPreset (n);
        }
    }), true);
}
