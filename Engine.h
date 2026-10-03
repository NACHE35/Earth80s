#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>

// Moteur de synthe : oscillateurs PolyBLEP, filtre type Korg MS-20, enveloppes, voix.
namespace E80
{
    constexpr int kVoices = 6;   // comme le Juno-106
    constexpr float kPiF = 3.14159265358979f;

    inline float fastTanh (float x) noexcept
    {
        if (x < -3.0f) return -1.0f;
        if (x >  3.0f) return  1.0f;
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Saturation douce (lineaire jusqu'a ~1, plafonne a +/-2) : integrateurs satures du filtre
    inline float satur (float x) noexcept { return 2.0f * fastTanh (0.5f * x); }

    inline float softLimit (float x) noexcept
    {
        const float a = std::abs (x);
        if (a <= 0.8f) return x;
        const float s = 0.8f + 0.2f * fastTanh ((a - 0.8f) / 0.2f);
        return x < 0.0f ? -s : s;
    }

    inline float polyBlep (float t, float dt) noexcept
    {
        if (t < dt)          { t /= dt;               return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt)   { t = (t - 1.0f) / dt;   return t * t + t + t + 1.0f; }
        return 0.0f;
    }
    inline float blepSaw (float t, float dt) noexcept { return 2.0f * t - 1.0f - polyBlep (t, dt); }
    inline float blepPulse (float t, float dt, float pw) noexcept
    {
        float v = t < pw ? 1.0f : -1.0f;
        v += polyBlep (t, dt);
        float t2 = t - pw; if (t2 < 0.0f) t2 += 1.0f;
        v -= polyBlep (t2, dt);
        return v;
    }

    //==========================================================================
    struct Osc
    {
        float ph = 0.0f;

        // wave : 0 Saw, 1 Pulse, 2 Saw+Pulse, 3 Tri, 4 Sine
        float tick (float inc, int wave, float pw, bool& wrapped) noexcept
        {
            ph += inc; wrapped = false;
            if (ph >= 1.0f) { ph -= 1.0f; wrapped = true; }
            switch (wave)
            {
                case 0:  return blepSaw (ph, inc);
                case 1:  return blepPulse (ph, inc, pw);
                case 2:  return 0.7f * (blepSaw (ph, inc) + blepPulse (ph, inc, pw));
                case 3:  return 4.0f * std::abs (ph - 0.5f) - 1.0f;
                default: return std::sin (2.0f * kPiF * ph);
            }
        }
    };

    //==========================================================================
    // Filtre 12 dB/oct a variable d'etat (TPT) avec integrateurs satures : resonance agressive
    // qui s'auto-oscille et sature comme celle du Korg MS-20. Sortie passe-bas ou passe-haut.
    struct MS20Filter
    {
        float s1 = 0.0f, s2 = 0.0f;

        float process (float x, float g, float k, bool hp) noexcept
        {
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float a2 = g * a1;
            const float a3 = g * a2;
            const float v3 = x - s2;
            const float v1 = a1 * s1 + a2 * v3;
            const float v2 = s2 + a2 * s1 + a3 * v3;
            s1 = satur (2.0f * v1 - s1);
            s2 = satur (2.0f * v2 - s2);
            return hp ? (x - k * v1 - v2) : v2;
        }
        void reset() noexcept { s1 = s2 = 0.0f; }
    };

    //==========================================================================
    struct Env
    {
        enum Stage { Idle, Atk, Dec, Sus, Rel };
        Stage st = Idle;
        float v = 0.0f;

        void on() noexcept  { st = Atk; }
        void off() noexcept { if (st != Idle) st = Rel; }
        bool active() const noexcept { return st != Idle; }

        float tick (float aInc, float dCoef, float sus, float rCoef) noexcept
        {
            switch (st)
            {
                case Atk: v += aInc; if (v >= 1.0f) { v = 1.0f; st = Dec; } break;
                case Dec: v = sus + (v - sus) * dCoef; if (std::abs (v - sus) < 1.0e-4f) { v = sus; st = Sus; } break;
                case Sus: v = sus; break;
                case Rel: v *= rCoef; if (v < 1.0e-4f) { v = 0.0f; st = Idle; } break;
                default: break;
            }
            return v;
        }
    };

    //==========================================================================
    // Parametres calcules une fois par bloc
    struct BP
    {
        int o1wave = 0, o1oct = 1, o1semi = 0, o2wave = 0, o2oct = 1, o2semi = 0;
        float o1fine = 0, o1pw = 0.5f, o1pwm = 0, o1lvl = 0.8f, sub = 0, noise = 0;
        float o2fine = 0, o2pw = 0.5f, o2pwm = 0, o2lvl = 0.5f, o2env = 0;
        bool sync = false, hp = false;
        float cutoff = 4000.0f, k = 1.0f, envAmt = 0, keyTrk = 0, driveIn = 1, driveOut = 1, maxCutoff = 18000.0f;
        float lfoPitch = 0, lfoFilt = 0, lfoFadeInc = 0;
        float aAInc = 0.01f, aDCoef = 0.99f, aS = 0.8f, aRCoef = 0.999f;
        float fAInc = 0.01f, fDCoef = 0.99f, fS = 0.3f, fRCoef = 0.999f;
        float glideCoef = 0.0f, tuneSemis = 0.0f;
    };

    //==========================================================================
    struct Voice
    {
        bool gate = false, sustained = false;
        int note = -1;
        float vel = 0.8f;
        unsigned long long age = 0;
        float curPitch = 60.0f, tgtPitch = 60.0f;
        float uniDetune = 0.0f;     // demi-tons
        float pan = 0.0f;           // -1 .. 1
        Osc o1, o2;
        float subPh = 0.0f;
        Env ampE, fltE;
        MS20Filter filt;
        float lfoFade = 0.0f;

        void trigger (int n, float velocity, float startPitch, bool glideFrom, juce::Random& rnd) noexcept
        {
            if (! ampE.active())
            {
                o1.ph = rnd.nextFloat(); o2.ph = rnd.nextFloat(); subPh = 0.0f;
                filt.reset();
            }
            note = n; vel = velocity; gate = true; sustained = false;
            tgtPitch = (float) n;
            curPitch = glideFrom ? startPitch : (float) n;
            ampE.on(); fltE.on();
            lfoFade = 0.0f;
        }

        void release() noexcept { gate = false; sustained = false; ampE.off(); fltE.off(); }

        void kill() noexcept { gate = false; sustained = false; ampE.st = Env::Idle; ampE.v = 0.0f; fltE.st = Env::Idle; fltE.v = 0.0f; filt.reset(); }

        void render (const BP& bp, float lfoVal, double sr, float bendSemis, float modWheel,
                     juce::Random& rnd, float& outL, float& outR) noexcept
        {
            if (bp.glideCoef > 0.0f) curPitch += (tgtPitch - curPitch) * bp.glideCoef;
            else                     curPitch = tgtPitch;

            if (bp.lfoFadeInc > 0.0f) lfoFade = juce::jmin (1.0f, lfoFade + bp.lfoFadeInc);
            else                      lfoFade = 1.0f;
            const float lfo = lfoVal * lfoFade;

            const float fe = fltE.tick (bp.fAInc, bp.fDCoef, bp.fS, bp.fRCoef);
            const float ae = ampE.tick (bp.aAInc, bp.aDCoef, bp.aS, bp.aRCoef);

            const float inv = 1.0f / (float) sr;
            const float vib = lfo * (bp.lfoPitch * 2.0f + modWheel);
            const float base = curPitch + uniDetune + bendSemis + bp.tuneSemis + vib;
            const float p1 = base + (float) (bp.o1oct - 1) * 12.0f + (float) bp.o1semi + bp.o1fine * 0.01f;
            const float p2 = base + (float) (bp.o2oct - 1) * 12.0f + (float) bp.o2semi + bp.o2fine * 0.01f
                                  + fe * bp.o2env * 24.0f;
            const float inc1 = juce::jmin (0.45f, 440.0f * std::exp2 ((p1 - 69.0f) / 12.0f) * inv);
            const float inc2 = juce::jmin (0.45f, 440.0f * std::exp2 ((p2 - 69.0f) / 12.0f) * inv);

            const float pw1 = juce::jlimit (0.05f, 0.95f, bp.o1pw + lfo * 0.4f * bp.o1pwm);
            const float pw2 = juce::jlimit (0.05f, 0.95f, bp.o2pw + lfo * 0.4f * bp.o2pwm);

            bool w1 = false, w2 = false;
            const float s1 = o1.tick (inc1, bp.o1wave, pw1, w1);
            if (bp.sync && w1) o2.ph = 0.0f;
            const float s2 = o2.tick (inc2, bp.o2wave, pw2, w2);

            subPh += inc1 * 0.5f; if (subPh >= 1.0f) subPh -= 1.0f;
            const float sb = blepPulse (subPh, inc1 * 0.5f, 0.5f);
            const float nz = rnd.nextFloat() * 2.0f - 1.0f;

            const float x = 0.5f * (s1 * bp.o1lvl + s2 * bp.o2lvl + sb * bp.sub * 0.8f + nz * bp.noise * 0.7f);

            const float octs = bp.envAmt * fe * 6.0f
                             + bp.keyTrk * (curPitch - 60.0f) / 12.0f
                             + bp.lfoFilt * lfo * 3.0f;
            const float fc = juce::jlimit (20.0f, bp.maxCutoff, bp.cutoff * std::exp2 (octs));
            const float g = std::tan (kPiF * fc * inv);

            const float y = filt.process (x * bp.driveIn, g, bp.k, bp.hp) * bp.driveOut;
            const float a = y * ae * (0.35f + 0.65f * vel) * 1.4142f;

            const float ang = (pan + 1.0f) * 0.25f * kPiF;
            outL = a * std::cos (ang);
            outR = a * std::sin (ang);
        }
    };
}
