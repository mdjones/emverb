# emverb

Stereo **delay + reverb** effect for the **Electrosmith Patch.Init()** (Daisy Patch Submodule).

The dry input signal and the delay output can each be sent to a shared stereo reverb (ReverbSc) with **independent send levels**, giving you full control over how much reverb is applied to the clean sound versus the echoes.

---

## Signal Flow

```
Audio In ──┬──▶ Delay Line (+ feedback) ──▶ delay_out
           └──▶ dry_out

Reverb Send  =  (dry_out × Dry Verb Send)
              + (delay_out × Delay Verb Send)

Reverb Send ──▶ ReverbSc ──▶ verb_out

Output  =  dry_out × 0.6
         + delay_out × 0.5
         + verb_out  × 0.6          ──▶ soft-clip ──▶ Audio Out
```

## Controls

| Knob / CV | Parameter            | Range                  |
|-----------|----------------------|------------------------|
| CV_1      | Delay Time           | 0 … ~2 s              |
| CV_2      | Delay Feedback       | 0 … 95 %              |
| CV_3      | Dry → Reverb Send    | 0 (none) … 1 (full)   |
| CV_4      | Delay → Reverb Send  | 0 (none) … 1 (full)   |

- **CV_3 = 0, CV_4 = 0** → pure delay, no reverb.
- **CV_3 = 1, CV_4 = 0** → reverb on dry signal only; delay stays dry.
- **CV_3 = 0, CV_4 = 1** → reverb on delay taps only; dry signal stays clean.
- **CV_3 = 1, CV_4 = 1** → everything drenched in reverb.

## Compile-Time Tuning

Edit the constants near the top of `emverb.cpp`:

| Constant        | Default | Description                              |
|-----------------|---------|------------------------------------------|
| `VERB_FEEDBACK` | 0.85    | Reverb decay length (0 – 1)              |
| `VERB_LP_FREQ`  | 6000 Hz | Reverb damping low-pass cutoff           |
| `DRY_LEVEL`     | 0.6     | Dry signal in output mix                 |
| `DELAY_LEVEL`   | 0.5     | Delay signal in output mix               |
| `VERB_LEVEL`    | 0.6     | Reverb signal in output mix              |
| `MAX_FEEDBACK`  | 0.95    | Hard cap on delay feedback               |
| `SMOOTH_COEFF`  | 0.002   | Knob smoothing speed (higher = faster)   |

## Building

### Prerequisites

- [libDaisy](https://github.com/electro-smith/libDaisy) built (`make` in libDaisy root)
- [DaisySP](https://github.com/electro-smith/DaisySP) built (`make` in DaisySP root)

### Set library paths

Either edit the top of the `Makefile` or export environment variables:

```bash
export LIBDAISY_DIR=/path/to/libDaisy
export DAISYSP_DIR=/path/to/DaisySP
```

### Compile

```bash
make
```

### Flash (USB DFU)

Put the Daisy into DFU mode (hold BOOT, press RESET, release BOOT), then:

```bash
make flash-dfu
```

Or using the Daisy web programmer, upload `build/emverb.bin`.

## License

MIT — do whatever you want with it.

