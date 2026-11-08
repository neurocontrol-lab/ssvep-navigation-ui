# Visual stimulus and timing

Four opaque black and white squares sit at the top, bottom, left, and right of the viewport. Their centers use the 12% and 88% positions on the respective axes, and each square scales from 140 pixels at 1920 × 1080. Static labels sit outside the flickering area, and their spacing stays fixed in pixels. The top-right compass and timer remain visible. [Target constants](../src/Constants.cpp)

| Target | Bright / dark frames | 60 Hz monitor | 75 Hz monitor | 90 Hz monitor | 100 Hz monitor | 120 Hz monitor |
|---|---|---|---|---|---|---|
| Forward | 3 / 4 | 8.57 | 10.71 | 12.86 | 14.29 | 17.14 |
| Backward | 3 / 3 | 10.00 | 12.50 | 15.00 | 16.67 | 20.00 |
| Turn left | 2 / 3 | 12.00 | 15.00 | 18.00 | 20.00 | 24.00 |
| Turn right | 2 / 2 | 15.00 | 18.75 | 22.50 | 25.00 | 30.00 |

The table gives nominal fundamentals in Hz. The maze advances the sequence once per buffer swap. At one swap per display refresh, divide the refresh rate by the total bright and dark frames. The on-screen labels use the refresh rate reported by the OS. Measure the light output to check the timing under load.

The listed fundamentals range from 8.57 to 30 Hz. The 120 Hz right target reaches the lower edge of the 30–80 Hz range used for narrowband gamma in one human visual cortex study. A 30 Hz stimulus also has second and third harmonics at 60 and 90 Hz. Choose acquisition and filter bandwidth for the harmonics used in CCA. Treat the gamma range as a description of neural activity. [Hermes et al., *Stimulus Dependence of Gamma Oscillations in Human Visual Cortex*](https://pubmed.ncbi.nlm.nih.gov/24855114/)

## Display behavior

- **F1** pauses or resumes flicker. Paused targets stay gray. Resuming restarts the sequence.
- The maze pauses flicker on victory and game-over screens.
- The maze requests VSync at startup. If VSync is unavailable, the targets start paused. F1 can preview them, while EEG steering remains disabled.

The launcher previews the frequencies for the selected stimulus profile. The maze checks those references against its active display before arming EEG steering. Variable refresh, compositor behavior, and missed refreshes can affect optical timing even when the nominal frequencies agree.

## Optical validation

Use a fixed refresh rate and measure each target with the BPW34 photodiode during navigation. The MG24 validation sketch reads the photodiode on **D10**, buffers a capture, and reports frequency and timing statistics. The capture tools can export raw samples to CSV. [BPW34 wiring and capture guide](https://github.com/neurocontrol-lab/ssvep-acquisition/blob/main/firmware/photodiode_validation/README.md)

The current stimulus uses solid monochrome patches to simplify timing measurements. Validate contrast, optical timing, and decoding separately if testing colored or patterned targets.
