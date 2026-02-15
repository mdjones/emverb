### Initial creation

* Used Claude Opus 4.6 in GitHub Copilot

Oops. Misspelling


User: In emverb create a reverb/delay for the Electrosmith patch.init() using libDaisy and DaisySP. Make it so that delay and input signal can have varing degress of reverb applied to them sepertaly 

GitHub Copilot: 

  

Read [](file:///Users/jonesmic/Lab/emverb)

Created 4 todos

Starting: *Create project structure and Makefile* (2/4)

Created [](file:///Users/jonesmic/Lab/emverb/Makefile)

Starting: *Implement main DSP application* (3/4)

Created [](file:///Users/jonesmic/Lab/emverb/emverb.cpp)

Starting: *Create README documentation* (4/4)

Created [](file:///Users/jonesmic/Lab/emverb/README.md)

Completed: *Create README documentation* (4/4)

Done. Three files created in emverb:

### Architecture

**Signal flow** — the input is split into a dry path and a stereo delay line (with feedback). Both paths feed a shared `ReverbSc` reverb via **independent send knobs**, then everything is summed and soft-clipped at the output.

### Files

- Makefile — libDaisy/DaisySP build system for the Patch Submodule target
- emverb.cpp — all DSP: stereo delay, stereo reverb, parameter smoothing, soft-clip output protection
- README.md — signal flow diagram, control mapping, build instructions

### Knob mapping

| Knob | Function | Notes |
|------|----------|-------|
| **CV_1** | Delay Time | 0 – ~2 seconds (96k samples at 48 kHz, buffer in SDRAM) |
| **CV_2** | Delay Feedback | 0 – 95% (clamped to prevent runaway) |
| **CV_3** | Dry → Reverb Send | How much clean input goes to reverb |
| **CV_4** | Delay → Reverb Send | How much delay output goes to reverb |

This gives you full independent control — you can reverb-drench only the echoes while keeping the dry signal clean, or vice versa, or any mix in between. Reverb character (decay, damping) and output mix levels are exposed as compile-time constants at the top of the source file.

Made changes.

