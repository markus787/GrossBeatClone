#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

// Kurven-Presets. Eingabe x = Position im Zyklus (0..1).
// timeCurve  -> von wo im Zyklus wird gelesen (0..1, darf auch <0 sein = vorheriger Zyklus)
//               Werte <= x lesen Vergangenes (Stutter, Reverse, Tape-Stop ...).
// volumeCurve -> Lautstärke (0..1)
namespace gb
{
    inline float frac (float x) { return x - std::floor (x); }

    inline juce::StringArray timeNames()
    {
        return { "Normal", "Stutter 1/8", "Stutter 1/16", "Tape Stop",
                 "Reverse 1/4", "Scratch", "Halbe Geschwindigkeit" };
    }

    inline juce::StringArray volumeNames()
    {
        return { "Aus", "Gate 1/8", "Gate 1/16", "Fade Out", "Pump 1/4" };
    }

    inline float timeCurve (int preset, float x)
    {
        switch (preset)
        {
            case 1: { float s = std::floor (x * 8.0f) / 8.0f;
                      return s + std::fmod (x - s, 1.0f / 16.0f); }
            case 2: { float s = std::floor (x * 16.0f) / 16.0f;
                      return s + std::fmod (x - s, 1.0f / 32.0f); }
            case 3: { if (x < 0.5f) return x;
                      float u = (x - 0.5f) * 2.0f;
                      return 0.5f + 0.5f * (u - 0.5f * u * u); }
            case 4: { float s = std::floor (x * 4.0f);
                      float f = x * 4.0f - s;
                      return (s - f) / 4.0f; }
            case 5: return x - 0.15f * (1.0f + std::sin (juce::MathConstants<float>::twoPi * 2.0f * x));
            case 6: return x * 0.5f;
            default: return x;
        }
    }

    inline float volumeCurve (int preset, float x)
    {
        switch (preset)
        {
            case 1: return frac (x * 8.0f)  < 0.5f ? 1.0f : 0.0f;
            case 2: return frac (x * 16.0f) < 0.5f ? 1.0f : 0.0f;
            case 3: return 1.0f - x;
            case 4: return 0.1f + 0.9f * frac (x * 4.0f);
            default: return 1.0f;
        }
    }
}
