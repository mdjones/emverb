/**
 * emverb – Stereo Delay + Reverb for Electrosmith Patch.Init()
 *
 * Signal-flow
 * ───────────
 *   Audio In ──┬──▶ Delay Line (w/ feedback) ──▶ delay_out
 *              └──▶ dry_out
 *
 *   Reverb Send Bus  =  (dry_out × dry_verb_send)
 *                      + (delay_out × dly_verb_send)
 *
 *   Reverb Bus ──▶ ReverbSc ──▶ verb_out
 *
 *   Output  =  dry_out  +  delay_out  +  verb_out
 *
 * Controls  (Patch.Init() knobs / CV jacks)
 * ─────────
 *   CV_1  –  Delay Time        (0 … ~2 s)
 *   CV_2  –  Delay Feedback    (0 … 95 %)
 *   CV_3  –  Dry → Reverb Send
 *   CV_4  –  Delay → Reverb Send
 *
 * Compile-time constants (below) let you tune reverb character,
 * output levels, and smoothing without extra hardware controls.
 */

#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

// ── Hardware ────────────────────────────────────────────────
DaisyPatchSM hw;

// ── Delay ───────────────────────────────────────────────────
// ~2 seconds at 48 kHz, placed in SDRAM so we don't eat
// the small internal SRAM.
static constexpr size_t MAX_DELAY_SAMPLES = 96000;
static DelayLine<float, MAX_DELAY_SAMPLES> DSY_SDRAM_BSS delay_l;
static DelayLine<float, MAX_DELAY_SAMPLES> DSY_SDRAM_BSS delay_r;

// ── Reverb ──────────────────────────────────────────────────
ReverbSc verb;

// ── Tuning constants ────────────────────────────────────────
// Reverb character
static constexpr float VERB_FEEDBACK = 0.85f;  // decay length  (0–1)
static constexpr float VERB_LP_FREQ  = 6000.f; // damping LP cutoff (Hz)

// Output mix levels (keep sum ≤ ~1.8 to avoid hard clipping)
static constexpr float DRY_LEVEL   = 0.6f;
static constexpr float DELAY_LEVEL = 0.5f;
static constexpr float VERB_LEVEL  = 0.6f;

// Max feedback ratio — clamped to avoid runaway oscillation
static constexpr float MAX_FEEDBACK = 0.95f;

// One-pole smoothing coefficient (per sample).
// ~500 samples → ≈10 ms settling at 48 kHz.
static constexpr float SMOOTH_COEFF = 0.002f;

// ── Smoothed parameter state ────────────────────────────────
static float s_delay_time  = 0.f;
static float s_delay_fback = 0.f;
static float s_dry_verb    = 0.f;
static float s_dly_verb    = 0.f;

// ── Helpers ─────────────────────────────────────────────────
/** Simple one-pole low-pass for parameter smoothing. */
inline float smooth(float current, float target)
{
    return current + SMOOTH_COEFF * (target - current);
}

/** Soft-clip via fast tanh approximation (Pade 3/2). */
inline float soft_clip(float x)
{
    if (x > 3.f)  return 1.f;
    if (x < -3.f) return -1.f;
    float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}

// ── Audio callback ──────────────────────────────────────────
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();

    // Raw knob / CV readings (0 – 1)
    const float k_time  = hw.GetAdcValue(CV_1);
    const float k_fback = hw.GetAdcValue(CV_2);
    const float k_dverb = hw.GetAdcValue(CV_3);
    const float k_yverb = hw.GetAdcValue(CV_4);

    for (size_t i = 0; i < size; i++)
    {
        // ── Smooth parameters ───────────────────────────────
        s_delay_time  = smooth(s_delay_time,  k_time);
        s_delay_fback = smooth(s_delay_fback, k_fback * MAX_FEEDBACK);
        s_dry_verb    = smooth(s_dry_verb,    k_dverb);
        s_dly_verb    = smooth(s_dly_verb,    k_yverb);

        const float delay_samps =
            s_delay_time * static_cast<float>(MAX_DELAY_SAMPLES - 1);

        delay_l.SetDelay(delay_samps);
        delay_r.SetDelay(delay_samps);

        // ── Read inputs ─────────────────────────────────────
        const float dry_l = IN_L[i];
        const float dry_r = IN_R[i];

        // ── Delay read ──────────────────────────────────────
        const float dly_l = delay_l.Read();
        const float dly_r = delay_r.Read();

        // ── Delay write (input + feedback) ──────────────────
        delay_l.Write(dry_l + dly_l * s_delay_fback);
        delay_r.Write(dry_r + dly_r * s_delay_fback);

        // ── Reverb send bus ─────────────────────────────────
        // Dry and delay signals are sent independently.
        const float verb_in_l =
            (dry_l * s_dry_verb) + (dly_l * s_dly_verb);
        const float verb_in_r =
            (dry_r * s_dry_verb) + (dly_r * s_dly_verb);

        float verb_out_l, verb_out_r;
        verb.Process(verb_in_l, verb_in_r, &verb_out_l, &verb_out_r);

        // ── Output mix ──────────────────────────────────────
        const float mix_l = (dry_l   * DRY_LEVEL)
                          + (dly_l   * DELAY_LEVEL)
                          + (verb_out_l * VERB_LEVEL);

        const float mix_r = (dry_r   * DRY_LEVEL)
                          + (dly_r   * DELAY_LEVEL)
                          + (verb_out_r * VERB_LEVEL);

        // Soft-clip to protect the output
        OUT_L[i] = soft_clip(mix_l);
        OUT_R[i] = soft_clip(mix_r);
    }
}

// ── Main ────────────────────────────────────────────────────
int main()
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    const float sr = hw.AudioSampleRate();

    // Delay init
    delay_l.Init();
    delay_r.Init();

    // Reverb init
    verb.Init(sr);
    verb.SetFeedback(VERB_FEEDBACK);
    verb.SetLpFreq(VERB_LP_FREQ);

    // Go
    hw.StartAudio(AudioCallback);

    for (;;) { }
}
