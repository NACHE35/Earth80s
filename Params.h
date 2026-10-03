#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

// Table unique des parametres du synthe (utilisee par le processeur, l'editeur et les presets).
namespace E80
{
    enum class Kind { Float, Choice, Bool };
    enum class Unit { Percent, Hz, Sec, Ms, Cents, Semi, Db, Ratio, Plain, Choice, Bool };

    struct ParamDef
    {
        const char* id; const char* name; Kind kind;
        float mn, mx, step, skew, def;
        Unit unit; const char* choices;
        bool inPreset;   // false : non modifie par les presets (volume, tune, power)
        bool bipolar;    // affichage centre sur zero
    };

    // X(id, nom, type, min, max, pas, skew, defaut, unite, choix, dansPreset, bipolaire)
    #define E80_PARAM_LIST(X) \
    /* ---- gauche : filtre MS-20 + performance ---- */ \
    X(volume,   "Volume",         Float,  0.f,    1.f,     0.001f, 1.f,   0.8f,    Percent, "", false, false) \
    X(cutoff,   "Cutoff",         Float,  20.f,   20000.f, 0.01f,  0.23f, 4000.f,  Hz,      "", true,  false) \
    X(reso,     "Resonance",      Float,  0.f,    1.f,     0.001f, 1.f,   0.2f,    Percent, "", true,  false) \
    X(hp,       "Highpass",       Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(glide,    "Portamento",     Float,  0.f,    3.f,     0.001f, 0.4f,  0.f,     Sec,     "", true,  false) \
    X(tune,     "Tune",           Float,  -100.f, 100.f,   0.1f,   1.f,   0.f,     Cents,   "", false, true)  \
    X(power,    "Power",          Bool,   0.f,    1.f,     1.f,    1.f,   1.f,     Bool,    "", false, false) \
    /* ---- onglet MASTER ---- */ \
    X(aA,       "Amp Attack",     Float,  0.001f, 8.f,     0.001f, 0.35f, 0.01f,   Sec,     "", true,  false) \
    X(aD,       "Amp Decay",      Float,  0.001f, 8.f,     0.001f, 0.35f, 0.4f,    Sec,     "", true,  false) \
    X(aS,       "Amp Sustain",    Float,  0.f,    1.f,     0.001f, 1.f,   0.8f,    Percent, "", true,  false) \
    X(aR,       "Amp Release",    Float,  0.001f, 10.f,    0.001f, 0.35f, 0.3f,    Sec,     "", true,  false) \
    X(fA,       "Filter Attack",  Float,  0.001f, 8.f,     0.001f, 0.35f, 0.01f,   Sec,     "", true,  false) \
    X(fD,       "Filter Decay",   Float,  0.001f, 8.f,     0.001f, 0.35f, 0.4f,    Sec,     "", true,  false) \
    X(fS,       "Filter Sustain", Float,  0.f,    1.f,     0.001f, 1.f,   0.3f,    Percent, "", true,  false) \
    X(fR,       "Filter Release", Float,  0.001f, 10.f,    0.001f, 0.35f, 0.3f,    Sec,     "", true,  false) \
    X(envAmt,   "Env Amount",     Float,  -1.f,   1.f,     0.001f, 1.f,   0.3f,    Percent, "", true,  true)  \
    X(keyTrk,   "Key Track",      Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(drive,    "Drive",          Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    X(voice,    "Voice Mode",     Choice, 0.f,    0.f,     1.f,    1.f,   0.f,     Choice,  "Poly|Mono|Unison", true, false) \
    X(uniDet,   "Unison Detune",  Float,  0.f,    1.f,     0.001f, 1.f,   0.3f,    Percent, "", true,  false) \
    X(spread,   "Stereo Spread",  Float,  0.f,    1.f,     0.001f, 1.f,   0.4f,    Percent, "", true,  false) \
    X(bend,     "Bend Range",     Float,  0.f,    12.f,    1.f,    1.f,   2.f,     Semi,    "", true,  false) \
    X(lfoShape, "LFO Shape",      Choice, 0.f,    0.f,     1.f,    1.f,   1.f,     Choice,  "Sine|Tri|Square|S&H", true, false) \
    X(lfoRate,  "LFO Rate",       Float,  0.05f,  30.f,    0.001f, 0.4f,  5.f,     Hz,      "", true,  false) \
    X(lfoDelay, "LFO Delay",      Float,  0.f,    3.f,     0.001f, 0.5f,  0.f,     Sec,     "", true,  false) \
    X(lfoPitch, "LFO Pitch",      Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    X(lfoFilt,  "LFO Filter",     Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    /* ---- onglet OSC 1 ---- */ \
    X(o1wave,   "Osc1 Wave",      Choice, 0.f,    0.f,     1.f,    1.f,   0.f,     Choice,  "Saw|Pulse|Saw+Pulse|Tri|Sine", true, false) \
    X(o1oct,    "Osc1 Octave",    Choice, 0.f,    0.f,     1.f,    1.f,   1.f,     Choice,  "16'|8'|4'|2'", true, false) \
    X(o1semi,   "Osc1 Semi",      Float,  -12.f,  12.f,    1.f,    1.f,   0.f,     Semi,    "", true,  true)  \
    X(o1fine,   "Osc1 Fine",      Float,  -100.f, 100.f,   0.1f,   1.f,   0.f,     Cents,   "", true,  true)  \
    X(o1pw,     "Osc1 Pulse Width", Float, 0.05f, 0.95f,   0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(o1pwm,    "Osc1 PWM",       Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    X(o1lvl,    "Osc1 Level",     Float,  0.f,    1.f,     0.001f, 1.f,   0.8f,    Percent, "", true,  false) \
    X(sub,      "Sub Osc",        Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    X(noise,    "Noise",          Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    /* ---- onglet OSC 2 ---- */ \
    X(o2wave,   "Osc2 Wave",      Choice, 0.f,    0.f,     1.f,    1.f,   0.f,     Choice,  "Saw|Pulse|Saw+Pulse|Tri|Sine", true, false) \
    X(o2oct,    "Osc2 Octave",    Choice, 0.f,    0.f,     1.f,    1.f,   1.f,     Choice,  "16'|8'|4'|2'", true, false) \
    X(o2semi,   "Osc2 Semi",      Float,  -12.f,  12.f,    1.f,    1.f,   0.f,     Semi,    "", true,  true)  \
    X(o2fine,   "Osc2 Fine",      Float,  -100.f, 100.f,   0.1f,   1.f,   7.f,     Cents,   "", true,  true)  \
    X(o2pw,     "Osc2 Pulse Width", Float, 0.05f, 0.95f,   0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(o2pwm,    "Osc2 PWM",       Float,  0.f,    1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  false) \
    X(o2lvl,    "Osc2 Level",     Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(sync,     "Osc Sync",       Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(o2env,    "Env > Pitch",    Float,  -1.f,   1.f,     0.001f, 1.f,   0.f,     Percent, "", true,  true)  \
    /* ---- onglet FX ---- */ \
    X(fxComp,   "Comp On",        Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(cThr,     "Comp Threshold", Float,  -40.f,  0.f,     0.1f,   1.f,   -12.f,   Db,      "", true,  false) \
    X(cRatio,   "Comp Ratio",     Float,  1.f,    20.f,    0.1f,   0.5f,  3.f,     Ratio,   "", true,  false) \
    X(cAtk,     "Comp Attack",    Float,  0.5f,   100.f,   0.1f,   0.4f,  10.f,    Ms,      "", true,  false) \
    X(cRel,     "Comp Release",   Float,  20.f,   1000.f,  1.f,    0.4f,  200.f,   Ms,      "", true,  false) \
    X(cGain,    "Comp Makeup",    Float,  0.f,    18.f,    0.1f,   1.f,   3.f,     Db,      "", true,  false) \
    X(fxPhase,  "Phaser On",      Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(phRate,   "Phaser Rate",    Float,  0.05f,  8.f,     0.001f, 0.4f,  0.5f,    Hz,      "", true,  false) \
    X(phDepth,  "Phaser Depth",   Float,  0.f,    1.f,     0.001f, 1.f,   0.6f,    Percent, "", true,  false) \
    X(phFb,     "Phaser Feedback", Float, -0.9f,  0.9f,    0.001f, 1.f,   0.4f,    Percent, "", true,  true)  \
    X(phMix,    "Phaser Mix",     Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(fxFlange, "Flanger On",     Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(flRate,   "Flanger Rate",   Float,  0.05f,  5.f,     0.001f, 0.4f,  0.25f,   Hz,      "", true,  false) \
    X(flDepth,  "Flanger Depth",  Float,  0.f,    1.f,     0.001f, 1.f,   0.7f,    Percent, "", true,  false) \
    X(flFb,     "Flanger Feedback", Float, -0.9f, 0.9f,    0.001f, 1.f,   0.5f,    Percent, "", true,  true)  \
    X(flMix,    "Flanger Mix",    Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(fxChorus, "Chorus On",      Bool,   0.f,    1.f,     1.f,    1.f,   1.f,     Bool,    "", true,  false) \
    X(chMode,   "Chorus Mode",    Choice, 0.f,    0.f,     1.f,    1.f,   0.f,     Choice,  "I|II|I+II", true, false) \
    X(chDepth,  "Chorus Depth",   Float,  0.f,    1.f,     0.001f, 1.f,   0.8f,    Percent, "", true,  false) \
    X(chMix,    "Chorus Mix",     Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(fxDelay,  "Delay On",       Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(dlTime,   "Delay Time",     Float,  10.f,   1500.f,  1.f,    0.5f,  375.f,   Ms,      "", true,  false) \
    X(dlFb,     "Delay Feedback", Float,  0.f,    0.95f,   0.001f, 1.f,   0.35f,   Percent, "", true,  false) \
    X(dlTone,   "Delay Tone",     Float,  0.f,    1.f,     0.001f, 1.f,   0.6f,    Percent, "", true,  false) \
    X(dlMix,    "Delay Mix",      Float,  0.f,    1.f,     0.001f, 1.f,   0.3f,    Percent, "", true,  false) \
    X(fxRev,    "Reverb On",      Bool,   0.f,    1.f,     1.f,    1.f,   0.f,     Bool,    "", true,  false) \
    X(rvSize,   "Reverb Size",    Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(rvDamp,   "Reverb Damping", Float,  0.f,    1.f,     0.001f, 1.f,   0.5f,    Percent, "", true,  false) \
    X(rvMix,    "Reverb Mix",     Float,  0.f,    1.f,     0.001f, 1.f,   0.25f,   Percent, "", true,  false)

    #define E80_X_ENUM(id,name,kind,mn,mx,step,skew,def,unit,choices,inp,bip) P_##id,
    enum ParamIndex { E80_PARAM_LIST(E80_X_ENUM) kNumParams };
    #undef E80_X_ENUM

    #define E80_X_TABLE(id,name,kind,mn,mx,step,skew,def,unit,choices,inp,bip) \
        { #id, name, Kind::kind, mn, mx, step, skew, def, Unit::unit, choices, inp, bip },
    inline const ParamDef kDefs[kNumParams] = { E80_PARAM_LIST(E80_X_TABLE) };
    #undef E80_X_TABLE

    inline int numChoices (const char* s)
    {
        int n = 1;
        for (; *s; ++s) if (*s == '|') ++n;
        return n;
    }

#ifndef E80_TEST_STUB
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        AudioProcessorValueTreeState::ParameterLayout layout;
        for (const auto& d : kDefs)
        {
            switch (d.kind)
            {
                case Kind::Float:
                    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { d.id, 1 }, d.name,
                                NormalisableRange<float> (d.mn, d.mx, d.step, d.skew), d.def));
                    break;
                case Kind::Choice:
                    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { d.id, 1 }, d.name,
                                StringArray::fromTokens (d.choices, "|", ""), (int) d.def));
                    break;
                case Kind::Bool:
                    layout.add (std::make_unique<AudioParameterBool> (ParameterID { d.id, 1 }, d.name, d.def > 0.5f));
                    break;
            }
        }
        return layout;
    }
#endif
}
