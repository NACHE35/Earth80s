#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

using namespace E80;

Earth80sProcessor::Earth80sProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < kNumParams; ++i)
        raw[(size_t) i] = apvts.getRawParameterValue (kDefs[i].id);
    apvts.state.setProperty ("presetName", "Init", nullptr);
    apvts.state.setProperty ("presetIndex", -1, nullptr);
}

void Earth80sProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    flanger.prepare (sr);
    chorus.prepare (sr);
    delayFx.prepare (sr);
    phaser.reset();
    comp.reset();
    reverb.setSampleRate (sr);
    reverb.reset();
    for (auto& v : voices) v.kill();
    noteStack.clear();
    lfoPhase = 0.0;
    volS.reset (sr, 0.03);   volS.setCurrentAndTargetValue (pv (P_volume));
    powerS.reset (sr, 0.03); powerS.setCurrentAndTargetValue (pv (P_power) > 0.5f ? 1.0f : 0.0f);
    lastVoiceMode = voiceMode = (int) std::lround (pv (P_voice));
}

//==============================================================================
void Earth80sProcessor::updateBlockParams()
{
    const float fsr = (float) sr;

    bp.o1wave = (int) std::lround (pv (P_o1wave));
    bp.o1oct  = (int) std::lround (pv (P_o1oct));
    bp.o1semi = (int) std::lround (pv (P_o1semi));
    bp.o1fine = pv (P_o1fine);
    bp.o1pw = pv (P_o1pw); bp.o1pwm = pv (P_o1pwm); bp.o1lvl = pv (P_o1lvl);
    bp.sub = pv (P_sub); bp.noise = pv (P_noise);

    bp.o2wave = (int) std::lround (pv (P_o2wave));
    bp.o2oct  = (int) std::lround (pv (P_o2oct));
    bp.o2semi = (int) std::lround (pv (P_o2semi));
    bp.o2fine = pv (P_o2fine);
    bp.o2pw = pv (P_o2pw); bp.o2pwm = pv (P_o2pwm); bp.o2lvl = pv (P_o2lvl);
    bp.sync = pv (P_sync) > 0.5f;
    bp.o2env = pv (P_o2env);

    bp.cutoff = pv (P_cutoff);
    const float res = pv (P_reso);
    bp.k = 1.9f * (1.0f - res) * (1.0f - res) + 0.03f;
    bp.hp = pv (P_hp) > 0.5f;
    bp.envAmt = pv (P_envAmt);
    bp.keyTrk = pv (P_keyTrk);
    const float drv = pv (P_drive);
    bp.driveIn = 1.0f + drv * 6.0f;
    bp.driveOut = 1.0f / (1.0f + drv * 2.5f);
    bp.maxCutoff = 0.45f * fsr;
    bp.lfoPitch = pv (P_lfoPitch);
    bp.lfoFilt = pv (P_lfoFilt);
    const float lfoDelay = pv (P_lfoDelay);
    bp.lfoFadeInc = lfoDelay > 0.001f ? 1.0f / (lfoDelay * fsr) : 0.0f;

    auto inc  = [&] (float t) { return 1.0f / (juce::jmax (0.001f, t) * fsr); };
    auto dcoef = [&] (float t) { return std::exp (-4.6f / (juce::jmax (0.001f, t) * fsr)); };
    auto rcoef = [&] (float t) { return std::exp (-6.9f / (juce::jmax (0.001f, t) * fsr)); };
    bp.aAInc = inc (pv (P_aA)); bp.aDCoef = dcoef (pv (P_aD)); bp.aS = pv (P_aS); bp.aRCoef = rcoef (pv (P_aR));
    bp.fAInc = inc (pv (P_fA)); bp.fDCoef = dcoef (pv (P_fD)); bp.fS = pv (P_fS); bp.fRCoef = rcoef (pv (P_fR));

    const float glide = pv (P_glide);
    bp.glideCoef = glide < 0.005f ? 0.0f : 1.0f - std::exp (-4.6f / (glide * fsr));
    bp.tuneSemis = pv (P_tune) * 0.01f;

    bendRange = pv (P_bend);
    lfoShape = (int) std::lround (pv (P_lfoShape));
    lfoInc = (double) pv (P_lfoRate) / sr;

    // Mode de voix
    voiceMode = (int) std::lround (pv (P_voice));
    if (voiceMode != lastVoiceMode) { allNotesOff(); lastVoiceMode = voiceMode; }

    // Detune / panoramique par voix
    const float det = pv (P_uniDet) * 0.4f;   // jusqu'a 40 cents
    const float spr = pv (P_spread);
    for (int i = 0; i < kVoices; ++i)
    {
        const float pos = ((float) i - 2.5f) / 2.5f;
        if (voiceMode == 2)      { voices[i].uniDetune = pos * det; voices[i].pan = pos * spr; }
        else if (voiceMode == 1) { voices[i].uniDetune = 0.0f;      voices[i].pan = 0.0f; }
        else                     { voices[i].uniDetune = 0.0f;      voices[i].pan = pos * spr * 0.5f; }
    }
    voiceGain = voiceMode == 2 ? 0.38f : 0.7f;
}

//==============================================================================
int Earth80sProcessor::findVoiceForNote()
{
    for (int i = 0; i < kVoices; ++i) if (! voices[i].ampE.active()) return i;
    int best = -1; float bestV = 2.0f;
    for (int i = 0; i < kVoices; ++i)
        if (voices[i].ampE.st == Env::Rel && voices[i].ampE.v < bestV) { bestV = voices[i].ampE.v; best = i; }
    if (best >= 0) return best;
    unsigned long long oldest = ~0ull; best = 0;
    for (int i = 0; i < kVoices; ++i) if (voices[i].age < oldest) { oldest = voices[i].age; best = i; }
    return best;
}

void Earth80sProcessor::noteOn (int note, float vel)
{
    ++ageCounter;
    const bool glideOn = bp.glideCoef > 0.0f;

    if (voiceMode == 0)
    {
        const int i = findVoiceForNote();
        voices[i].age = ageCounter;
        voices[i].trigger (note, vel, lastPitch, glideOn, rng);
    }
    else
    {
        noteStack.erase (std::remove (noteStack.begin(), noteStack.end(), note), noteStack.end());
        noteStack.push_back (note);
        bool anyGate = false;
        for (auto& v : voices) anyGate = anyGate || v.gate;
        const int count = voiceMode == 1 ? 1 : kVoices;
        for (int i = 0; i < count; ++i)
        {
            voices[i].age = ageCounter;
            if (anyGate)   // legato : on glisse sans retrigger
            {
                voices[i].note = note; voices[i].tgtPitch = (float) note; voices[i].vel = vel;
                if (! glideOn) voices[i].curPitch = (float) note;
            }
            else
                voices[i].trigger (note, vel, lastPitch, glideOn, rng);
        }
    }
    lastPitch = (float) note;
}

void Earth80sProcessor::noteOff (int note)
{
    if (voiceMode == 0)
    {
        for (auto& v : voices)
            if (v.gate && v.note == note)
            {
                if (sustainPedal) v.sustained = true;
                else v.release();
            }
    }
    else
    {
        noteStack.erase (std::remove (noteStack.begin(), noteStack.end(), note), noteStack.end());
        const int count = voiceMode == 1 ? 1 : kVoices;
        if (! noteStack.empty())
        {
            const int n = noteStack.back();
            for (int i = 0; i < count; ++i) { voices[i].note = n; voices[i].tgtPitch = (float) n; }
            lastPitch = (float) n;
        }
        else
            for (int i = 0; i < count; ++i)
            {
                if (sustainPedal) voices[i].sustained = true;
                else voices[i].release();
            }
    }
}

void Earth80sProcessor::allNotesOff()
{
    noteStack.clear();
    for (auto& v : voices) v.release();
}

void Earth80sProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())             noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())       noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())    bendNorm = (float) (m.getPitchWheelValue() - 8192) / 8192.0f;
    else if (m.isController())
    {
        const int cc = m.getControllerNumber(), val = m.getControllerValue();
        if (cc == 1)       modWheel = (float) val / 127.0f;
        else if (cc == 64)
        {
            const bool down = val >= 64;
            if (sustainPedal && ! down)
                for (auto& v : voices) if (v.sustained) v.release();
            sustainPedal = down;
        }
        else if (cc == 123 || cc == 120) allNotesOff();
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
}

//==============================================================================
void Earth80sProcessor::renderVoices (float* L, float* R, int from, int to)
{
    const float bendSemis = bendNorm * bendRange;
    for (int i = from; i < to; ++i)
    {
        lfoPhase += lfoInc;
        if (lfoPhase >= 1.0) { lfoPhase -= 1.0; shValue = rng.nextFloat() * 2.0f - 1.0f; }
        const float p = (float) lfoPhase;
        float lfo;
        switch (lfoShape)
        {
            case 0:  lfo = std::sin (2.0f * kPiF * p); break;
            case 1:  lfo = 4.0f * std::abs (p - 0.5f) - 1.0f; break;
            case 2:  lfo = p < 0.5f ? 1.0f : -1.0f; break;
            default: lfo = shValue; break;
        }

        float sl = 0.0f, sr_ = 0.0f;
        for (auto& v : voices)
        {
            if (! v.ampE.active()) continue;
            float l, r;
            v.render (bp, lfo, sr, bendSemis, modWheel, rng, l, r);
            sl += l; sr_ += r;
        }
        L[i] = sl * voiceGain;
        R[i] = sr_ * voiceGain;
    }
}

void Earth80sProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2) return;

    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    updateBlockParams();

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (pos, n, meta.samplePosition);
        renderVoices (L, R, pos, t);
        handleMidi (meta.getMessage());
        pos = t;
    }
    renderVoices (L, R, pos, n);

    int mask = 0;
    for (int i = 0; i < kVoices; ++i) if (voices[i].ampE.active()) mask |= (1 << i);
    voiceMask.store (mask);

    // ---------------- Effets : Comp -> Phaser -> Flanger -> Chorus -> Delay -> Reverb ----------------
    if (pv (P_fxComp) > 0.5f)
    {
        comp.set (sr, pv (P_cThr), pv (P_cRatio), pv (P_cAtk), pv (P_cRel), pv (P_cGain));
        for (int i = 0; i < n; ++i) comp.process (L[i], R[i]);
    }
    if (pv (P_fxPhase) > 0.5f)
    {
        const float rate = pv (P_phRate), dep = pv (P_phDepth), fb = pv (P_phFb) * 0.95f, mix = pv (P_phMix);
        for (int i = 0; i < n; ++i) phaser.process (L[i], R[i], sr, rate, dep, fb, mix);
    }
    if (pv (P_fxFlange) > 0.5f)
    {
        const float rate = pv (P_flRate), dep = pv (P_flDepth), fb = pv (P_flFb), mix = pv (P_flMix);
        for (int i = 0; i < n; ++i) flanger.process (L[i], R[i], sr, rate, dep, fb, mix);
    }
    if (pv (P_fxChorus) > 0.5f)
    {
        const int mode = juce::jlimit (0, 2, (int) std::lround (pv (P_chMode)));
        const float dep = pv (P_chDepth), mix = pv (P_chMix);
        for (int i = 0; i < n; ++i) chorus.process (L[i], R[i], sr, mode, dep, mix);
    }
    if (pv (P_fxDelay) > 0.5f)
    {
        delayFx.setTone (sr, pv (P_dlTone));
        const float t = pv (P_dlTime), fb = pv (P_dlFb) * 0.95f, mix = pv (P_dlMix);
        for (int i = 0; i < n; ++i) delayFx.process (L[i], R[i], sr, t, fb, mix);
    }
    if (pv (P_fxRev) > 0.5f)
    {
        const float mix = pv (P_rvMix);
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.2f + 0.78f * pv (P_rvSize);
        rp.damping = pv (P_rvDamp);
        rp.wetLevel = 0.33f * mix;
        rp.dryLevel = 0.5f * (1.0f - 0.3f * mix);
        rp.width = 1.0f; rp.freezeMode = 0.0f;
        reverb.setParameters (rp);
        reverb.processStereo (L, R, n);
    }

    // ---------------- Master ----------------
    volS.setTargetValue (pv (P_volume));
    powerS.setTargetValue (pv (P_power) > 0.5f ? 1.0f : 0.0f);
    float pkL = 0.0f, pkR = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        const float g = volS.getNextValue() * powerS.getNextValue();
        L[i] = softLimit (L[i] * g);
        R[i] = softLimit (R[i] * g);
        pkL = juce::jmax (pkL, std::abs (L[i]));
        pkR = juce::jmax (pkR, std::abs (R[i]));
    }
    levelL.store (pkL);
    levelR.store (pkR);
}

juce::AudioProcessorEditor* Earth80sProcessor::createEditor() { return new Earth80sEditor (*this); }

//==============================================================================
void Earth80sProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void Earth80sProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// Presets
juce::String Earth80sProcessor::getCurrentPresetName() const
{
    return apvts.state.getProperty ("presetName", "Init").toString();
}

int Earth80sProcessor::getCurrentPresetIndex() const
{
    return (int) apvts.state.getProperty ("presetIndex", -1);
}

void Earth80sProcessor::setParamReal (int index, float v)
{
    if (auto* p = apvts.getParameter (kDefs[index].id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void Earth80sProcessor::loadFactoryPreset (int index)
{
    if (index < 0 || index >= kNumFactoryPresets) return;
    const auto v = makePreset (index);
    for (int i = 0; i < kNumParams; ++i)
        if (kDefs[i].inPreset) setParamReal (i, v[(size_t) i]);
    apvts.state.setProperty ("presetName", presetName (index), nullptr);
    apvts.state.setProperty ("presetIndex", index, nullptr);
}

juce::File Earth80sProcessor::getUserPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #if JUCE_MAC
    d = d.getChildFile ("Application Support");
   #endif
    return d.getChildFile ("NACHE").getChildFile ("Earth 80s").getChildFile ("Presets");
}

juce::Array<juce::File> Earth80sProcessor::getUserPresetFiles() const
{
    auto files = getUserPresetDir().findChildFiles (juce::File::findFiles, false, "*.xml");
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
               { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
    return files;
}

bool Earth80sProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = getUserPresetDir();
    if (! dir.createDirectory()) return false;

    juce::XmlElement x ("Earth80sPreset");
    x.setAttribute ("name", clean);
    for (int i = 0; i < kNumParams; ++i)
        if (kDefs[i].inPreset) x.setAttribute (kDefs[i].id, (double) pv (i));

    const bool ok = x.writeTo (dir.getChildFile (clean + ".xml"));
    if (ok)
    {
        apvts.state.setProperty ("presetName", clean, nullptr);
        apvts.state.setProperty ("presetIndex", -1, nullptr);
    }
    return ok;
}

void Earth80sProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr || ! xml->hasTagName ("Earth80sPreset")) return;
    for (int i = 0; i < kNumParams; ++i)
        if (kDefs[i].inPreset && xml->hasAttribute (kDefs[i].id))
            setParamReal (i, (float) xml->getDoubleAttribute (kDefs[i].id));
    apvts.state.setProperty ("presetName", file.getFileNameWithoutExtension(), nullptr);
    apvts.state.setProperty ("presetIndex", -1, nullptr);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Earth80sProcessor(); }
