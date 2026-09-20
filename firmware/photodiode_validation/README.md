# BPW34 frequency check on XIAO MG24

## Wiring

- BPW34 cathode -> **3V3**, not 5V.
- BPW34 anode -> **D10** (PA05, ADC-capable).
- 47 kohm resistor from the anode/D10 junction -> **GND**.
- USB powers the board. Shield the detector from ambient light and place its sensitive face over one target.

## Arduino IDE

1. In Preferences, add `https://siliconlabs.github.io/arduino/package_arduinosilabs_index.json` to Additional Boards Manager URLs if needed.
2. In Boards Manager, install **Silicon Labs**. Select **Seeed Studio XIAO MG24** (the XIAO_MG24 variant), the matching USB port, and **Tools > Protocol stack > None** if offered. This sketch needs no radio stack or extra libraries.
3. Open `photodiode_validation.ino` in this folder; keep `signal_analysis.h` alongside it. Click Verify, then Upload.
4. Open Serial Monitor at **115200 baud**. Send `h` for help if the startup text was missed.
5. Send `r` to collect approximately 2 seconds at 2 kHz. Keep the sensor stationary. Results appear after acquisition; either no line ending or newline is fine. Repeat captures to check consistency; a short capture can miss rare timing faults.
6. Send `d` to export the last capture as `timestamp_us,adc` CSV. This takes time at 115200 baud and does not acquire new samples. Send `r` again after repositioning.

Start with separate static black and white captures to check that white gives higher ADC readings. A flat signal should NOT produce a valid frequency. Then enable flicker with F1 in the maze and record each target, both stationary and during movement.

At 60 Hz monitor refresh, expected fundamentals are forward **8.57 Hz**, backward **10 Hz**, left **12 Hz**, and right **15 Hz**. For other refresh rates see the repository README. These are full-cycle frequencies, not the count of both rising and falling edges.

## Interpreting the report

- ADC min/max/span: 12-bit raw counts. The default board-core voltage reference is retained; counts are not converted to assumed volts. Near-4095 readings may indicate clipping: try 10 kohm. A weak signal may benefit from 100 kohm, but high source impedance can require longer ADC acquisition time or a buffer.
- Frequency: inverse mean rising-to-rising period, with 35%/65% hysteresis thresholds derived from the capture's min/max. At least three periods and 40 counts of range are required. These are heuristics, not evidence of a clean signal.
- Mean/min/max period and standard deviation: inspect individual intervals, not just average frequency. Large variation can reflect noise, display PWM, missed frames, or threshold artifacts. Min/max thresholds can be distorted by outliers; inspect the raw data when results look wrong.
- Sample rate/gaps: timestamps are estimates from the midpoint of each blocking `analogRead` call using the MCU clock. This is software-scheduled acquisition, not timer-triggered ADC/DMA and not calibrated sub-millisecond metrology. Late samples are flagged and not replaced by synthetic catch-up samples.
- Buffers use 24,000 bytes (4,000 samples plus timestamps), leaving more room for the board core, stack, and heap. Use Protocol stack **None** when available; check the compiler's RAM summary before increasing `SAMPLE_COUNT`. Serial output occurs only after recording. There is no automatic pass/fail claim: compare against the configured sequence and examine the waveform. The 2 kHz capture is not intended to characterize high-frequency backlight PWM.

## Validation status

Firmware has been uploaded and exercised on the MG24. End-to-end optical validation of the maze targets remains pending.

The original 20,000-sample capture left very little RAM in the user's build. The current 4,000-sample capture frees 96,000 bytes while retaining nominal 2 kHz sampling. Check the new compiler memory summary after uploading this version.

Host tests in `tests/photodiode_analysis_test.cpp` (relative to the repository root) validate the analysis logic only. For optical validation, perform dark/steady-light checks, examine raw CSV traces, and compare each maze target against its expected frequency under load.

References: [Seeed pin map and setup](https://wiki.seeedstudio.com/xiao_mg24_getting_started/), [Seeed analog examples](https://wiki.seeedstudio.com/xiao_mg24_pin_multiplexing/), [Silicon Labs Arduino core](https://github.com/SiliconLabsSoftware/arduino).
