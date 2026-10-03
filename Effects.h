#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <vector>
#include "Engine.h"

// Effets : compresseur, phaser, flanger, chorus (style Juno), delay ping-pong. (La reverb utilise juce::Reverb.)
namespace E80
{
    struct DelayLine
    {
        std::vector<float> buf;
        int w = 0, n = 0;
        void prepare (int size) { n = size; buf.assign ((size_t) n, 0.0f); w = 0; }
        void push (float x) noexcept { buf[(size_t) w] = x; if (++w >= n) w = 0; }
        float read (float d) const noexcept
        {
            d = juce::jlimit (1.0f, (float) (n - 2), d);
            float rp = (float) w - d;
            while (rp < 0.0f) rp += (float) n;
            int i0 = (int) rp;
            const float f = rp - (float) i0;
            if (i0 >= n) i0 -= n;
            int i1 = i0 + 1; if (i1 >= n) i1 = 0;
            return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
        }
    };

    struct AP1
    {
        float s = 0.0f;
        inline float process (float x, float a) noexcept { const float y = a * x + s; s = x - a * y; return y; }
    };

    //==========================================================================
    struct Compressor
    {
        float env = 0.0f, aC = 0.0f, rC = 0.0f, thr = -12.0f, ratio = 3.0f, makeup = 1.0f;

        void set (double sr, float thrDb, float ratio_, float atkMs, float relMs, float makeupDb)
        {
            aC = std::exp (-1.0f / (atkMs * 0.001f * (float) sr));
            rC = std::exp (-1.0f / (relMs * 0.001f * (float) sr));
            thr = thrDb; ratio = ratio_;
            makeup = std::pow (10.0f, makeupDb / 20.0f);
        }

        void process (float& l, float& r) noexcept
        {
            const float lvl = juce::jmax (std::abs (l), std::abs (r));
            env = lvl > env ? aC * env + (1.0f - aC) * lvl : rC * env + (1.0f - rC) * lvl;
            const float db = 20.0f * std::log10 (env + 1.0e-6f);
            const float over = db - thr;
            const float knee = 6.0f;
            float gr = 0.0f;
            if (2.0f * over > knee)                   gr = over * (1.0f - 1.0f / ratio);
            else if (2.0f * std::abs (over) <= knee)  { const float x = over + knee * 0.5f; gr = (1.0f - 1.0f / ratio) * x * x / (2.0f * knee); }
            const float g = std::pow (10.0f, -gr / 20.0f) * makeup;
            l *= g; r *= g;
        }
        void reset() { env = 0.0f; }
    };

    //==========================================================================
    struct PhaserFx
    {
        AP1 st[2][6];
        float fb[2] { 0.0f, 0.0f };
        double ph = 0.0;

        void reset() { for (auto& c : st) for (auto& s : c) s.s = 0.0f; fb[0] = fb[1] = 0.0f; }

        void process (float& l, float& r, double sr, float rate, float depth, float fbk, float mix) noexcept
        {
            ph += (double) rate / sr; ph -= std::floor (ph);
            const float in[2] = { l, r };
            float out[2];
            for (int c = 0; c < 2; ++c)
            {
                const float lfo = (float) std::sin (6.283185307179586 * (ph + 0.25 * c));
                const float fc = 250.0f * std::exp2 (depth * 4.0f * (0.5f + 0.5f * lfo));
                const float t = std::tan (kPiF * fc / (float) sr);
                const float a = (t - 1.0f) / (t + 1.0f);
                const float x = in[c] + fastTanh (fbk * fb[c]);
                float y = x;
                for (int k = 0; k < 6; ++k) y = st[c][k].process (y, a);
                fb[c] = y;
                out[c] = in[c] * (1.0f - mix) + mix * 0.5f * (x + y);
            }
            l = out[0]; r = out[1];
        }
    };

    //==========================================================================
    struct FlangerFx
    {
        DelayLine dl[2];
        double ph = 0.0;

        void prepare (double sr) { for (auto& d : dl) d.prepare ((int) (0.03 * sr) + 8); ph = 0.0; }

        void process (float& l, float& r, double sr, float rate, float depth, float fbk, float mix) noexcept
        {
            ph += (double) rate / sr; ph -= std::floor (ph);
            float in[2] = { l, r }, out[2];
            for (int c = 0; c < 2; ++c)
            {
                double p = ph + 0.25 * c; p -= std::floor (p);
                const float tri = (float) (2.0 * std::abs (p - 0.5));
                const float ms = 0.5f + depth * 5.5f * tri;
                const float d = dl[c].read (ms * (float) sr / 1000.0f);
                dl[c].push (in[c] + fbk * d);
                out[c] = (in[c] + mix * d) / (1.0f + 0.5f * mix);
            }
            l = out[0]; r = out[1];
        }
    };

    //==========================================================================
    // Chorus a la Juno-106 : modes I, II, I+II (lignes a retard modulees en opposition L/R)
    struct ChorusFx
    {
        DelayLine dl[2];
        double ph = 0.0;
        float lp[2] { 0.0f, 0.0f };
        float lpA = 0.5f;

        void prepare (double sr)
        {
            for (auto& d : dl) d.prepare ((int) (0.04 * sr) + 8);
            lpA = 1.0f - std::exp (-6.283185307f * 9000.0f / (float) sr);
            ph = 0.0; lp[0] = lp[1] = 0.0f;
        }

        void process (float& l, float& r, double sr, int mode, float depth, float mix) noexcept
        {
            static const float rate[3] = { 0.83f, 0.32f, 9.75f };
            static const float dep[3]  = { 1.8f, 2.0f, 0.35f };
            ph += (double) rate[mode] / sr; ph -= std::floor (ph);
            const float in[2] = { l, r };
            float out[2];
            for (int c = 0; c < 2; ++c)
            {
                dl[c].push (in[c]);
                double p = ph + (c == 0 ? 0.0 : 0.5); p -= std::floor (p);
                const float tri = (float) (4.0 * std::abs (p - 0.5) - 1.0);
                const float ms = 3.35f + dep[mode] * (0.3f + 1.1f * depth) * tri;
                float w = dl[c].read (ms * (float) sr / 1000.0f);
                lp[c] += lpA * (w - lp[c]);
                out[c] = in[c] + (lp[c] - in[c]) * 0.6f * mix;
            }
            l = out[0]; r = out[1];
        }
    };

    //==========================================================================
    struct DelayFx
    {
        DelayLine dl[2];
        float lp[2] { 0.0f, 0.0f };
        float curD = -1.0f, lpA = 0.5f;

        void prepare (double sr) { for (auto& d : dl) d.prepare ((int) (1.6 * sr) + 8); curD = -1.0f; lp[0] = lp[1] = 0.0f; }

        void setTone (double sr, float tone)
        {
            const float fc = 500.0f * std::pow (24.0f, tone);
            lpA = 1.0f - std::exp (-6.283185307f * fc / (float) sr);
        }

        // delay ping-pong : l'entree alimente la ligne gauche, qui alimente la droite, etc.
        void process (float& l, float& r, double sr, float timeMs, float fb, float mix) noexcept
        {
            const float target = timeMs * (float) sr / 1000.0f;
            if (curD < 0.0f) curD = target;
            curD += (target - curD) * 0.0008f;
            const float oL = dl[0].read (curD), oR = dl[1].read (curD);
            lp[0] += lpA * (oL - lp[0]);
            lp[1] += lpA * (oR - lp[1]);
            dl[0].push (0.5f * (l + r) + fb * lp[1]);
            dl[1].push (fb * lp[0]);
            l += oL * mix;
            r += oR * mix;
        }
    };
}
