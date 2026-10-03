#include "Presets.h"
#include <cmath>

using namespace E80;

namespace
{
    const char* kCats[kNumCategories] = { "Bass", "Lead", "Pad", "Keys", "Brass", "Strings", "Pluck", "Bell & Mallet", "Atmosphere", "FX & Sweeps" };
    const char* kStyles[kNumStyles]   = { "Classic 80's", "Synthwave", "Italo & Disco", "Dark Cinema", "Modern Retro" };

    const char* kAdj[kNumStyles][kPresetsPerStyle] = {
        { "Chrome", "Neon", "Velvet", "Crystal", "Glass", "Golden", "Silver", "Ivory", "Pastel", "Retro",
          "Vintage", "Analog", "Warm", "Sunset", "Miami", "Cruise", "Boulevard", "Arcade", "Mixtape", "Polaroid" },
        { "Outrun", "Laser", "Grid", "Turbo", "Cyber", "Electric", "Horizon", "Rewind", "Hyper", "Vapor",
          "Highway", "Starlight", "Digital", "Specter", "Overdrive", "Radar", "Nitro", "Magenta", "Aero", "Teal" },
        { "Disco", "Funky", "Glitter", "Boogie", "Velour", "Euro", "Ballroom", "Mirrorball", "Starlit", "Sparkle",
          "Platinum", "Satin", "Stardust", "Riviera", "Champagne", "Moonlight", "Glam", "Slick", "Groovy", "Lounge" },
        { "Shadow", "Noir", "Abyss", "Haunted", "Frozen", "Obsidian", "Eclipse", "Dread", "Grim", "Void",
          "Ashen", "Midnight", "Crimson", "Ghost", "Requiem", "Tomb", "Nightfall", "Gloom", "Carbon", "Sinister" },
        { "Fresh", "Hybrid", "Future", "Lo-Fi", "Prism", "Silk", "Halo", "Aurora", "Drift", "Bloom",
          "Orbit", "Echo", "Fuse", "Lumen", "Haze", "Nova", "Flux", "Mirage", "Tidal", "Cobalt" }
    };

    const char* kNoun[kNumCategories][kPresetsPerStyle] = {
        { "Bass", "Sub", "Thump", "Growl", "Wobble", "Slap", "Stab Bass", "Rumble", "Punch", "Roller",
          "Octaver", "Walker", "Throb", "Groove", "Drive Bass", "Mono Bass", "Pump", "Boom", "Thunder", "Anchor" },
        { "Lead", "Solo", "Scream", "Whistle", "Sync Lead", "Cry", "Soar", "Blade", "Hook", "Flight",
          "Shout", "Singer", "Siren", "Glider", "Spark", "Razor", "Beam", "Rider", "Flame", "Voice Lead" },
        { "Pad", "Wash", "Cloud", "Choir Pad", "Lagoon", "Sweep Pad", "Glow", "Fog Pad", "Veil", "Tide",
          "Canopy", "Aura", "Meadow", "Swell", "Mist", "Skyline Pad", "Blanket", "Shimmer", "Dome", "Float" },
        { "Keys", "Piano", "E-Piano", "Organ", "Clav", "Tines", "Comp Keys", "Stack", "Chord", "Mallet Keys",
          "Tone", "Funk Keys", "Pluck Keys", "Digi Keys", "Icy Keys", "Soul Keys", "Jazz Keys", "Bar Keys", "Stage Keys", "Mellow Keys" },
        { "Brass", "Horns", "Stabs", "Trumpet", "Fanfare", "Section", "Swell Brass", "Bone", "Blast", "Punch Brass",
          "Hit", "Riff", "Power Brass", "Big Band", "Shot", "Sax", "Bugle", "Parade", "Heavy Brass", "Royal" },
        { "Strings", "Ensemble", "Violins", "Cellos", "Orchestra", "Bows", "Legato", "Sweep Strings", "Pizzicato", "Section Strings",
          "Quartet", "Tremolo", "Staccato", "Swell Strings", "Cinema Strings", "Chamber", "Divisi", "Lush Strings", "Smooth Strings", "Sustain" },
        { "Pluck", "Arp", "Harp", "Pick", "Stab Pluck", "Bounce", "Blip", "Chime Pluck", "Staccato Pluck", "Sequence",
          "Pizz", "Kalimba", "Tick", "Hop", "Spring", "String Pluck", "Zap", "Twang", "Drip", "Gate Pluck" },
        { "Bell", "Chimes", "Glock", "Mallet", "Vibes", "Marimba", "Gong", "Tubular", "Music Box", "Celesta",
          "Ice Bell", "Bowl Bell", "Ping", "Tinkle", "Handbell", "Steel Drum", "Sleigh", "Church Bell", "FM Bell", "Dream Bell" },
        { "Atmosphere", "Drone", "Ambience", "Space", "Ether", "Dream", "Fog", "Soundscape", "Nebula", "Reverie",
          "Steam", "Stillness", "Cavern", "Voyage", "Planet", "Lullaby", "Whisper", "Skyline", "Tundra", "Expanse" },
        { "FX", "Sweep FX", "Riser", "Beam FX", "Impact", "Noise Wash", "Sci-Fi", "Alarm", "Engine", "Static",
          "Wind", "Rain FX", "Thunder FX", "Portal", "Glitch", "Transform", "Hyperspace", "Sonar", "Transmission", "Takeoff" }
    };
}

const char* E80::categoryName (int c) { return kCats[juce::jlimit (0, kNumCategories - 1, c)]; }
const char* E80::styleName (int s)    { return kStyles[juce::jlimit (0, kNumStyles - 1, s)]; }

juce::String E80::presetName (int index)
{
    index = juce::jlimit (0, kNumFactoryPresets - 1, index);
    const int cat = index / 100, style = (index % 100) / 20, i = index % 20;
    return juce::String (kAdj[style][i]) + " " + kNoun[cat][(i * 7 + style * 3) % 20];
}

//==============================================================================
PresetValues E80::makePreset (int index)
{
    index = juce::jlimit (0, kNumFactoryPresets - 1, index);

    PresetValues v {};
    for (int i = 0; i < kNumParams; ++i) v[(size_t) i] = kDefs[i].def;

    const int cat = index / 100, style = (index % 100) / 20, n = index % 20, flavor = n % 4;
    juce::Random r (91237 + index * 7919);

    auto rf     = [&] (float a, float b) { return a + (b - a) * r.nextFloat(); };
    auto chance = [&] (float p) { return r.nextFloat() < p; };
    auto pick   = [&] (std::initializer_list<float> l) { return *(l.begin() + r.nextInt ((int) l.size())); };
    auto S      = [&] (int id, float x) { v[(size_t) id] = x; };
    auto V      = [&] (int id) -> float& { return v[(size_t) id]; };

    // Base propre : effets coupes
    for (int id : { (int) P_fxComp, (int) P_fxPhase, (int) P_fxFlange, (int) P_fxChorus, (int) P_fxDelay, (int) P_fxRev }) S (id, 0.0f);
    S (P_o1lvl, 0.9f); S (P_o2lvl, 0.0f); S (P_o2fine, 0.0f);

    auto delayOn = [&] (float t, float fb, float tone, float mix)
    { S (P_fxDelay, 1); S (P_dlTime, t); S (P_dlFb, fb); S (P_dlTone, tone); S (P_dlMix, mix); };
    auto reverbOn = [&] (float size, float damp, float mix)
    { S (P_fxRev, 1); S (P_rvSize, size); S (P_rvDamp, damp); S (P_rvMix, mix); };
    auto chorusOn = [&] (int mode, float depth, float mix)
    { S (P_fxChorus, 1); S (P_chMode, (float) mode); S (P_chDepth, depth); S (P_chMix, mix); };

    //==================== Recettes par categorie ====================
    switch (cat)
    {
        case 0: // BASS
            S (P_o1wave, pick ({ 0, 1, 2 })); S (P_o1oct, chance (0.7f) ? 0.0f : 1.0f);
            S (P_o1pw, rf (0.3f, 0.7f)); S (P_o1pwm, rf (0.0f, 0.3f));
            S (P_o1lvl, rf (0.8f, 1.0f)); S (P_sub, rf (0.3f, 0.9f)); S (P_noise, rf (0.0f, 0.04f));
            if (chance (0.55f))
            {
                S (P_o2lvl, rf (0.35f, 0.7f)); S (P_o2wave, pick ({ 0, 1 })); S (P_o2oct, V (P_o1oct));
                S (P_o2fine, rf (-12.0f, 12.0f)); S (P_o2semi, pick ({ 0, 0, 0, 7, 12 }));
            }
            S (P_cutoff, rf (160.0f, 900.0f)); S (P_reso, rf (0.1f, 0.55f));
            S (P_envAmt, rf (0.25f, 0.7f)); S (P_keyTrk, rf (0.3f, 0.8f));
            S (P_fA, rf (0.001f, 0.006f)); S (P_fD, rf (0.12f, 0.5f)); S (P_fS, rf (0.0f, 0.25f)); S (P_fR, rf (0.05f, 0.3f));
            S (P_aA, rf (0.001f, 0.005f)); S (P_aD, rf (0.3f, 1.0f)); S (P_aS, rf (0.7f, 1.0f)); S (P_aR, rf (0.05f, 0.2f));
            S (P_voice, chance (0.65f) ? 1.0f : 0.0f);
            if (V (P_voice) > 0.5f && chance (0.5f)) S (P_glide, rf (0.01f, 0.12f));
            S (P_drive, rf (0.0f, 0.3f)); S (P_uniDet, 0.2f); S (P_spread, 0.2f);
            S (P_fxComp, 1); S (P_cThr, -18.0f); S (P_cRatio, 4.0f); S (P_cGain, 3.0f);
            break;

        case 1: // LEAD
            S (P_o1wave, pick ({ 0, 0, 2, 1 })); S (P_o1oct, 1);
            S (P_o1pw, rf (0.35f, 0.65f)); S (P_o1pwm, rf (0.0f, 0.5f)); S (P_o1lvl, rf (0.8f, 1.0f)); S (P_sub, rf (0.0f, 0.3f));
            S (P_o2lvl, rf (0.55f, 0.9f)); S (P_o2wave, pick ({ 0, 0, 1, 2 })); S (P_o2oct, 1);
            S (P_o2fine, rf (4.0f, 14.0f)); S (P_o2semi, pick ({ 0, 0, 0, 7, 12 }));
            if (chance (0.2f)) { S (P_sync, 1); S (P_o2env, rf (0.15f, 0.5f)); S (P_o2semi, pick ({ 5, 7, 12 })); }
            S (P_cutoff, rf (1800.0f, 7000.0f)); S (P_reso, rf (0.1f, 0.5f));
            S (P_envAmt, rf (0.1f, 0.5f)); S (P_keyTrk, rf (0.4f, 0.9f));
            S (P_fA, rf (0.001f, 0.02f)); S (P_fD, rf (0.3f, 0.9f)); S (P_fS, rf (0.4f, 0.8f)); S (P_fR, rf (0.15f, 0.5f));
            S (P_aA, rf (0.002f, 0.02f)); S (P_aD, rf (0.2f, 0.6f)); S (P_aS, rf (0.85f, 1.0f)); S (P_aR, rf (0.15f, 0.5f));
            S (P_voice, chance (0.6f) ? 1.0f : 0.0f);
            if (V (P_voice) > 0.5f && chance (0.7f)) S (P_glide, rf (0.02f, 0.2f));
            S (P_lfoShape, 0); S (P_lfoRate, rf (4.5f, 6.5f)); S (P_lfoDelay, rf (0.2f, 0.8f)); S (P_lfoPitch, rf (0.05f, 0.25f));
            break;

        case 2: // PAD
            S (P_o1wave, pick ({ 0, 2, 2, 1 })); S (P_o1oct, 1);
            S (P_o1pw, 0.5f); S (P_o1pwm, rf (0.2f, 0.7f)); S (P_o1lvl, rf (0.7f, 0.95f)); S (P_sub, rf (0.0f, 0.3f)); S (P_noise, rf (0.0f, 0.03f));
            S (P_o2lvl, rf (0.6f, 0.9f)); S (P_o2wave, pick ({ 0, 0, 2 })); S (P_o2fine, rf (6.0f, 15.0f)); S (P_o2pwm, rf (0.2f, 0.6f));
            S (P_cutoff, rf (900.0f, 3500.0f)); S (P_reso, rf (0.05f, 0.3f));
            S (P_envAmt, rf (0.1f, 0.35f)); S (P_keyTrk, rf (0.4f, 0.7f));
            S (P_fA, rf (0.5f, 2.5f)); S (P_fD, rf (1.0f, 3.0f)); S (P_fS, rf (0.4f, 0.7f)); S (P_fR, rf (1.0f, 3.0f));
            S (P_aA, rf (0.3f, 1.8f)); S (P_aD, rf (1.0f, 2.0f)); S (P_aS, rf (0.85f, 1.0f)); S (P_aR, rf (1.2f, 4.0f));
            S (P_voice, chance (0.3f) ? 2.0f : 0.0f); S (P_uniDet, rf (0.2f, 0.5f)); S (P_spread, rf (0.5f, 0.9f));
            S (P_lfoShape, 1); S (P_lfoRate, rf (0.1f, 0.8f)); S (P_lfoFilt, rf (0.0f, 0.25f));
            break;

        case 3: // KEYS
            S (P_o1wave, pick ({ 1, 2, 3, 3, 4 })); S (P_o1oct, 1);
            S (P_o1pw, rf (0.3f, 0.6f)); S (P_o1pwm, rf (0.0f, 0.3f)); S (P_o1lvl, rf (0.8f, 1.0f)); S (P_sub, rf (0.0f, 0.3f));
            S (P_o2lvl, rf (0.25f, 0.55f)); S (P_o2wave, pick ({ 3, 4, 1 })); S (P_o2oct, pick ({ 1, 2 }));
            S (P_o2fine, rf (-8.0f, 8.0f)); S (P_o2semi, pick ({ 0, 0, 7, 12 }));
            S (P_cutoff, rf (1500.0f, 5000.0f)); S (P_reso, rf (0.0f, 0.25f));
            S (P_envAmt, rf (0.3f, 0.6f)); S (P_keyTrk, rf (0.5f, 0.9f));
            S (P_fA, 0.002f); S (P_fD, rf (0.2f, 0.7f)); S (P_fS, rf (0.1f, 0.4f)); S (P_fR, rf (0.15f, 0.5f));
            S (P_aA, rf (0.002f, 0.01f)); S (P_aD, rf (0.6f, 1.8f)); S (P_aS, rf (0.2f, 0.6f)); S (P_aR, rf (0.2f, 0.6f));
            S (P_lfoShape, 0); S (P_lfoRate, rf (4.5f, 6.0f)); S (P_lfoPitch, rf (0.0f, 0.05f));
            break;

        case 4: // BRASS
            S (P_o1wave, pick ({ 0, 0, 2 })); S (P_o1oct, 1); S (P_o1lvl, rf (0.85f, 1.0f));
            S (P_o2lvl, rf (0.6f, 0.9f)); S (P_o2wave, 0); S (P_o2fine, rf (4.0f, 11.0f));
            S (P_cutoff, rf (600.0f, 1500.0f)); S (P_reso, rf (0.05f, 0.25f));
            S (P_envAmt, rf (0.4f, 0.7f)); S (P_keyTrk, rf (0.5f, 0.8f));
            S (P_fA, rf (0.03f, 0.12f)); S (P_fD, rf (0.25f, 0.5f)); S (P_fS, rf (0.45f, 0.7f)); S (P_fR, rf (0.1f, 0.3f));
            S (P_aA, rf (0.02f, 0.08f)); S (P_aD, rf (0.2f, 0.5f)); S (P_aS, rf (0.85f, 1.0f)); S (P_aR, rf (0.1f, 0.25f));
            S (P_lfoShape, 0); S (P_lfoRate, 5.5f); S (P_lfoDelay, rf (0.3f, 0.8f)); S (P_lfoPitch, rf (0.0f, 0.1f));
            break;

        case 5: // STRINGS
            S (P_o1wave, 0); S (P_o1oct, 1); S (P_o1lvl, rf (0.8f, 1.0f));
            S (P_o2lvl, rf (0.7f, 0.95f)); S (P_o2wave, 0); S (P_o2fine, rf (8.0f, 14.0f));
            S (P_cutoff, rf (2000.0f, 6000.0f)); S (P_reso, rf (0.0f, 0.15f));
            S (P_envAmt, rf (0.05f, 0.25f)); S (P_keyTrk, rf (0.5f, 0.9f));
            S (P_fA, rf (0.1f, 0.6f)); S (P_fD, rf (0.5f, 1.5f)); S (P_fS, rf (0.5f, 0.8f)); S (P_fR, rf (0.5f, 1.5f));
            S (P_aA, rf (0.25f, 0.9f)); S (P_aD, 1.0f); S (P_aS, rf (0.85f, 1.0f)); S (P_aR, rf (0.7f, 1.8f));
            S (P_voice, chance (0.25f) ? 2.0f : 0.0f); S (P_uniDet, rf (0.15f, 0.4f)); S (P_spread, rf (0.5f, 0.9f));
            S (P_lfoShape, 0); S (P_lfoRate, rf (4.5f, 6.0f)); S (P_lfoDelay, rf (0.5f, 1.2f)); S (P_lfoPitch, rf (0.02f, 0.08f));
            break;

        case 6: // PLUCK / ARP
            S (P_o1wave, pick ({ 0, 1, 2 })); S (P_o1oct, 1); S (P_o1pw, rf (0.25f, 0.6f)); S (P_o1lvl, 0.9f);
            if (chance (0.5f)) { S (P_o2lvl, rf (0.25f, 0.5f)); S (P_o2wave, pick ({ 0, 1 })); S (P_o2oct, 2); S (P_o2fine, rf (-6.0f, 6.0f)); }
            S (P_cutoff, rf (700.0f, 2500.0f)); S (P_reso, rf (0.2f, 0.5f));
            S (P_envAmt, rf (0.5f, 0.8f)); S (P_keyTrk, rf (0.5f, 0.9f));
            S (P_fA, 0.001f); S (P_fD, rf (0.08f, 0.3f)); S (P_fS, 0.0f); S (P_fR, rf (0.08f, 0.25f));
            S (P_aA, 0.001f); S (P_aD, rf (0.15f, 0.5f)); S (P_aS, rf (0.0f, 0.1f)); S (P_aR, rf (0.1f, 0.3f));
            delayOn (pick ({ 188, 250, 281, 375 }), rf (0.25f, 0.45f), 0.6f, rf (0.2f, 0.35f));
            break;

        case 7: // BELL / MALLET
            S (P_o1wave, pick ({ 4, 4, 3 })); S (P_o1oct, 1); S (P_o1lvl, rf (0.8f, 1.0f));
            S (P_o2lvl, rf (0.5f, 0.8f)); S (P_o2wave, pick ({ 4, 3 })); S (P_o2oct, pick ({ 1, 2 }));
            S (P_o2semi, pick ({ 7, 12, 5, 9, -5 })); S (P_o2fine, rf (-30.0f, 30.0f));
            S (P_cutoff, rf (3000.0f, 12000.0f)); S (P_reso, rf (0.0f, 0.2f));
            S (P_envAmt, rf (0.1f, 0.4f)); S (P_keyTrk, rf (0.6f, 1.0f));
            S (P_fA, 0.001f); S (P_fD, rf (0.5f, 1.5f)); S (P_fS, rf (0.1f, 0.4f)); S (P_fR, rf (0.4f, 1.2f));
            S (P_aA, 0.001f); S (P_aD, rf (1.0f, 3.0f)); S (P_aS, 0.0f); S (P_aR, rf (1.0f, 3.0f));
            reverbOn (rf (0.5f, 0.8f), rf (0.3f, 0.6f), rf (0.3f, 0.5f));
            break;

        case 8: // ATMOSPHERE
            S (P_o1wave, pick ({ 0, 2, 3 })); S (P_o1oct, 1); S (P_o1pwm, rf (0.2f, 0.6f)); S (P_o1lvl, rf (0.6f, 0.9f));
            S (P_sub, rf (0.0f, 0.3f)); S (P_noise, rf (0.05f, 0.3f));
            S (P_o2lvl, rf (0.4f, 0.8f)); S (P_o2wave, pick ({ 0, 3, 4 })); S (P_o2fine, rf (5.0f, 20.0f)); S (P_o2semi, pick ({ 0, 7, 12, -12 }));
            S (P_cutoff, rf (400.0f, 2500.0f)); S (P_reso, rf (0.3f, 0.6f));
            S (P_envAmt, rf (-0.3f, 0.3f)); S (P_keyTrk, rf (0.3f, 0.7f));
            S (P_fA, rf (2.0f, 5.0f)); S (P_fD, rf (2.0f, 5.0f)); S (P_fS, rf (0.3f, 0.7f)); S (P_fR, rf (3.0f, 7.0f));
            S (P_aA, rf (2.0f, 6.0f)); S (P_aD, 2.0f); S (P_aS, rf (0.85f, 1.0f)); S (P_aR, rf (3.0f, 8.0f));
            S (P_voice, chance (0.5f) ? 2.0f : 0.0f); S (P_uniDet, rf (0.3f, 0.7f)); S (P_spread, rf (0.8f, 1.0f));
            S (P_lfoShape, pick ({ 0, 1, 3 })); S (P_lfoRate, rf (0.05f, 0.4f)); S (P_lfoFilt, rf (0.2f, 0.5f)); S (P_lfoPitch, rf (0.0f, 0.05f));
            reverbOn (rf (0.8f, 0.95f), rf (0.3f, 0.6f), rf (0.5f, 0.7f));
            delayOn (rf (300.0f, 700.0f), rf (0.4f, 0.6f), rf (0.3f, 0.5f), rf (0.25f, 0.4f));
            if (chance (0.5f)) { S (P_fxPhase, 1); S (P_phRate, rf (0.05f, 0.3f)); S (P_phDepth, rf (0.5f, 1.0f)); S (P_phFb, rf (0.2f, 0.6f)); S (P_phMix, rf (0.3f, 0.5f)); }
            else               { S (P_fxFlange, 1); S (P_flRate, rf (0.05f, 0.25f)); S (P_flDepth, rf (0.5f, 1.0f)); S (P_flFb, rf (0.3f, 0.7f)); S (P_flMix, rf (0.3f, 0.5f)); }
            break;

        default: // FX & SWEEPS
            S (P_o1wave, pick ({ 0, 1, 2 })); S (P_o1oct, 1); S (P_o1lvl, 0.8f); S (P_noise, rf (0.3f, 0.9f));
            S (P_o2lvl, rf (0.4f, 0.9f)); S (P_o2wave, pick ({ 0, 1 })); S (P_o2semi, std::round (rf (-12.0f, 12.0f))); S (P_o2fine, rf (-50.0f, 50.0f));
            if (chance (0.5f)) S (P_sync, 1);
            S (P_o2env, rf (-0.6f, 0.9f));
            S (P_cutoff, rf (200.0f, 6000.0f)); S (P_reso, rf (0.4f, 0.9f)); S (P_envAmt, rf (-0.9f, 0.9f)); S (P_hp, chance (0.35f) ? 1.0f : 0.0f);
            S (P_fA, rf (0.5f, 3.0f)); S (P_fD, rf (1.0f, 4.0f)); S (P_fS, rf (0.0f, 0.5f)); S (P_fR, rf (1.0f, 4.0f));
            S (P_aA, rf (0.2f, 2.0f)); S (P_aD, 2.0f); S (P_aS, rf (0.5f, 1.0f)); S (P_aR, rf (1.0f, 4.0f));
            S (P_lfoShape, pick ({ 0, 1, 2, 3 })); S (P_lfoRate, rf (0.2f, 12.0f)); S (P_lfoFilt, rf (0.2f, 0.8f)); S (P_lfoPitch, rf (0.0f, 0.5f));
            S (P_voice, chance (0.4f) ? 1.0f : 0.0f); if (V (P_voice) > 0.5f) S (P_glide, rf (0.1f, 1.2f));
            S (P_fxFlange, 1); S (P_flRate, rf (0.05f, 1.5f)); S (P_flDepth, rf (0.5f, 1.0f)); S (P_flFb, rf (0.3f, 0.85f)); S (P_flMix, rf (0.4f, 0.8f));
            delayOn (rf (150.0f, 800.0f), rf (0.3f, 0.6f), rf (0.3f, 0.7f), rf (0.2f, 0.4f));
            reverbOn (rf (0.5f, 0.9f), rf (0.3f, 0.6f), rf (0.3f, 0.5f));
            break;
    }

    //==================== Style ====================
    const bool tonal = (cat != 7 && cat != 9);
    switch (style)
    {
        case 0: // Classic 80's : chaleureux, chorus Juno
            chorusOn ((int) pick ({ 0, 1 }), rf (0.6f, 1.0f), rf (0.4f, 0.8f));
            if (chance (0.5f)) reverbOn (rf (0.3f, 0.5f), 0.5f, rf (0.1f, 0.2f));
            break;

        case 1: // Synthwave : large, delays, drive
            chorusOn ((int) pick ({ 0, 2 }), rf (0.6f, 1.0f), rf (0.5f, 0.8f));
            if (chance (0.8f)) delayOn (pick ({ 357, 375, 429, 468 }), rf (0.25f, 0.45f), rf (0.4f, 0.7f), rf (0.2f, 0.35f));
            if (chance (0.9f)) reverbOn (rf (0.4f, 0.65f), rf (0.4f, 0.6f), rf (0.2f, 0.35f));
            V (P_drive) += rf (0.05f, 0.25f);
            if (tonal && V (P_o2lvl) > 0.0f) S (P_o2fine, rf (8.0f, 16.0f));
            V (P_cutoff) *= 1.2f;
            break;

        case 2: // Italo & Disco : brillant, pulse, compression
            S (P_fxComp, 1); S (P_cThr, -14.0f); S (P_cRatio, rf (2.5f, 4.0f)); S (P_cGain, 3.0f); S (P_cAtk, 8.0f); S (P_cRel, 180.0f);
            chorusOn (1, rf (0.6f, 1.0f), rf (0.4f, 0.7f));
            if (tonal && (cat == 0 || cat == 1 || cat == 3 || cat == 4) && chance (0.5f))
            { S (P_o1wave, pick ({ 1, 2 })); S (P_o1pwm, rf (0.25f, 0.55f)); }
            if (chance (0.4f)) { S (P_fxPhase, 1); S (P_phRate, rf (0.2f, 0.7f)); S (P_phDepth, rf (0.4f, 0.8f)); S (P_phFb, rf (0.2f, 0.5f)); S (P_phMix, rf (0.25f, 0.4f)); }
            if (chance (0.3f)) delayOn (pick ({ 250, 300, 333 }), rf (0.2f, 0.35f), 0.6f, rf (0.15f, 0.25f));
            if (chance (0.5f)) reverbOn (rf (0.3f, 0.5f), 0.5f, rf (0.15f, 0.25f));
            V (P_cutoff) *= 1.5f; V (P_aR) *= 0.7f; V (P_fR) *= 0.7f;
            break;

        case 3: // Dark Cinema : sombre, long, reverb enorme
            V (P_cutoff) *= 0.55f; V (P_reso) += 0.1f; V (P_aR) *= 1.6f; V (P_fR) *= 1.4f;
            V (P_sub) += 0.15f; V (P_noise) += 0.03f; V (P_lfoFilt) += rf (0.1f, 0.3f);
            chorusOn ((int) pick ({ 1, 2 }), rf (0.5f, 1.0f), rf (0.3f, 0.6f));
            reverbOn (rf (0.7f, 0.95f), rf (0.4f, 0.7f), rf (0.35f, 0.55f));
            if (chance (0.6f)) delayOn (rf (400.0f, 800.0f), rf (0.4f, 0.6f), rf (0.25f, 0.5f), rf (0.25f, 0.4f));
            if (chance (0.3f)) { S (P_fxFlange, 1); S (P_flRate, rf (0.05f, 0.2f)); S (P_flDepth, rf (0.5f, 1.0f)); S (P_flFb, rf (0.3f, 0.7f)); S (P_flMix, 0.3f); }
            break;

        default: // Modern Retro : propre, subtil, compresse
            S (P_fxComp, 1); S (P_cThr, -10.0f); S (P_cRatio, 2.5f); S (P_cGain, 2.0f);
            chorusOn ((int) pick ({ 0, 1 }), rf (0.4f, 0.8f), rf (0.2f, 0.4f));
            if (chance (0.3f)) { S (P_fxPhase, 1); S (P_phRate, rf (0.1f, 0.6f)); S (P_phDepth, rf (0.4f, 0.8f)); S (P_phFb, rf (0.1f, 0.4f)); S (P_phMix, rf (0.2f, 0.4f)); }
            else if (chance (0.25f)) { S (P_fxFlange, 1); S (P_flRate, rf (0.1f, 0.4f)); S (P_flDepth, rf (0.4f, 0.8f)); S (P_flFb, rf (0.2f, 0.5f)); S (P_flMix, rf (0.2f, 0.4f)); }
            reverbOn (rf (0.5f, 0.7f), rf (0.4f, 0.6f), rf (0.15f, 0.3f));
            if (tonal) { S (P_lfoShape, pick ({ 1, 3 })); V (P_spread) = juce::jmax (V (P_spread), 0.8f); }
            V (P_drive) += 0.1f;
            break;
    }

    // Les cordes gardent leur ensemble "I+II" typique
    if (cat == 5) chorusOn (2, rf (0.6f, 1.0f), rf (0.6f, 0.9f));

    //==================== Variation par "caractere" (n % 4) ====================
    switch (flavor)
    {
        case 1: V (P_cutoff) *= 1.4f; V (P_spread) = juce::jmax (V (P_spread), 0.7f); V (P_uniDet) += 0.2f; V (P_reso) *= 0.8f; break;
        case 2: V (P_cutoff) *= 0.7f; V (P_aR) *= 1.5f; V (P_fR) *= 1.5f; break;
        case 3:
            V (P_lfoFilt) += rf (0.15f, 0.35f); V (P_o1pwm) += 0.3f;
            if (chance (0.5f) && V (P_fxPhase) < 0.5f) { S (P_fxPhase, 1); S (P_phRate, rf (0.2f, 1.2f)); S (P_phDepth, rf (0.5f, 0.9f)); S (P_phFb, rf (0.2f, 0.6f)); S (P_phMix, rf (0.25f, 0.45f)); }
            else if (V (P_fxDelay) < 0.5f) delayOn (pick ({ 250, 375, 500 }), rf (0.25f, 0.4f), 0.5f, rf (0.15f, 0.3f));
            break;
        default: break;
    }

    // Securite : sons jamais muets
    V (P_o1lvl) = juce::jmax (V (P_o1lvl), 0.6f);
    V (P_cutoff) = juce::jlimit (60.0f, 18000.0f, V (P_cutoff));

    // Contraintes de plage / quantification
    for (int i = 0; i < kNumParams; ++i)
    {
        const auto& d = kDefs[i];
        float& x = v[(size_t) i];
        switch (d.kind)
        {
            case Kind::Float:  x = juce::jlimit (d.mn, d.mx, x); if (d.step >= 1.0f) x = std::round (x); break;
            case Kind::Choice: x = (float) juce::jlimit (0, numChoices (d.choices) - 1, (int) std::lround (x)); break;
            case Kind::Bool:   x = x > 0.5f ? 1.0f : 0.0f; break;
        }
    }
    return v;
}
