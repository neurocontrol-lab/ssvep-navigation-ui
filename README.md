# SSVEP–CCA-Based EEG Navigation

A research project investigating EEG-based control of a 3D maze using steady-state visual evoked potentials (SSVEP) and canonical correlation analysis (CCA). The existing C++/OpenGL/FreeGLUT environment provides a testbed for evaluating neural command decoding and closed-loop navigation.

**Current status:** the keyboard/mouse-controlled navigation environment is implemented. Visual stimulation, EEG acquisition integration, CCA decoding, and neural control are planned; this repository does not yet provide a working EEG-controlled system.

## Research direction

The planned interface associates four flickering visual targets with **forward, backward, turn left, and turn right** commands. CCA will compare windows of multichannel EEG with reference signals at the stimulus frequencies and their harmonics. Confidence thresholds and temporal evidence accumulation will determine when to execute a command.

Planned pipeline:

```text
EEG acquisition → preprocessing → SSVEP/CCA decoding
    → confidence and temporal decision logic → navigation command
```

Initial work will focus on:

- Four fixed screen-space targets, with photodiode validation of their optical frequency and timing.
- Integration of the planned four-channel EEG acquisition system.
- Calibration and evaluation of stimulus frequencies, EEG window lengths, and decision thresholds.
- Evaluation of decoding accuracy, false activations, command latency, maze completion time, and path efficiency.
- Comparison of direct decoding, confidence-gated control, and environment-aware command filtering.

## Branches

- **`main`**: ongoing SSVEP–CCA research and integration, built on the existing navigation environment.
- **`codex/opengl-game`**: preserved OpenGL game baseline before the research reorganization.

## Existing navigation environment

Originally developed for **UCSB CS280, Spring 2022**, the environment includes collision-aware movement, mouse look, strafing and jumping, a compass and timer HUD, and maze layouts loaded from text files.

<p align="center">
  <a href="https://youtu.be/9cJ7eTtbbqo">
    <img src="resources/3.webp" width="70%" alt="Existing OpenGL maze navigation environment">
  </a>
</p>
<p align="center">
  <em>The existing navigation environment used as the testbed for the planned BCI extension. <a href="https://youtu.be/9cJ7eTtbbqo">Watch the environment demo</a>.</em>
</p>

### Manual controls

- **W/S or up/down arrows:** move forward/backward.
- **A/D:** strafe left/right.
- **Left/right arrows:** turn the viewpoint.
- **Mouse:** change viewing direction.
- **Spacebar:** jump.
- **Escape:** exit.

These are the current manual controls; the planned BCI left/right commands will turn the viewpoint.

## Background and demo

- **Portable Windows x86 baseline:** [Play_Game.zip](../../raw/refs/heads/codex/opengl-game/resources/Play_Game.zip) (launch using `Play_Game.bat`).
- **Environment demo:** [Video](https://www.youtube.com/watch?v=9cJ7eTtbbqo).
- **Original course assignment explanation:** [Video](https://www.youtube.com/watch?v=O8dUZq9Oty0).
- **Original course assignment:** [HW1.pdf](resources/HW1.pdf).
