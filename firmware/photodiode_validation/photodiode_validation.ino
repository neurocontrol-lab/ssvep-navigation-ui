#include <Arduino.h>
#include "signal_analysis.h"

// XIAO MG24: BPW34 cathode -> 3V3; anode -> D10;
// resistor (start with 47 kohm) from the anode/D10 junction to GND.
constexpr unsigned SIGNAL_PIN = D10;
constexpr uint32_t SAMPLE_INTERVAL_US = 500; // nominal 2 kHz
constexpr size_t SAMPLE_COUNT = 4000;        // nominal 2 seconds, 24 kB buffers
constexpr unsigned MINIMUM_SIGNAL_SPAN = 40; // ADC counts; diagnostic heuristic
uint16_t samples[SAMPLE_COUNT];
uint32_t timestamps[SAMPLE_COUNT];
bool haveCapture = false;
uint32_t lateSamples = 0;

void printHelp() {
    Serial.println("BPW34 / XIAO MG24 / D10 / 12-bit ADC");
    Serial.println("r: record ~2 seconds and report; d: dump last capture as CSV; h: help");
    Serial.println("Keep the sensor over ONE target, shielded from ambient light.");
}

void printReport() {
    const SignalReport r = analyzeSignal(samples, timestamps, SAMPLE_COUNT, MINIMUM_SIGNAL_SPAN);
    Serial.print("ADC min/max/span: "); Serial.print(r.minimum); Serial.print('/');
    Serial.print(r.maximum); Serial.print('/'); Serial.println(r.maximum-r.minimum);
    Serial.print("Observed sample rate Hz: "); Serial.println(r.sampleRateHz, 2);
    Serial.print("Largest sample gap us: "); Serial.println(r.maxSampleGapUs);
    Serial.print("Samples late by >= one interval: "); Serial.println(lateSamples);
    if (r.maximum-r.minimum < MINIMUM_SIGNAL_SPAN) {
        Serial.println("WEAK/FLAT SIGNAL: check wiring, alignment, shielding, or resistor value.");
    } else if (r.periods < 3) {
        Serial.println("INSUFFICIENT EDGES: enable flicker and inspect the CSV waveform.");
    } else {
        Serial.print("Estimated optical frequency Hz: "); Serial.println(r.frequencyHz, 3);
        Serial.print("Complete rising-to-rising periods: "); Serial.println(r.periods);
        Serial.print("Period mean/min/max ms: "); Serial.print(r.meanPeriodUs/1000, 3);
        Serial.print('/'); Serial.print(r.minPeriodUs/1000.0, 3);
        Serial.print('/'); Serial.println(r.maxPeriodUs/1000.0, 3);
        Serial.print("Period standard deviation ms: "); Serial.println(r.periodStdUs/1000, 3);
        if (r.periodStdUs > r.meanPeriodUs * 0.05)
            Serial.println("IRREGULAR EDGES: inspect waveform; noise/PWM/frame timing may contribute.");
    }
    if (r.clippedHigh) Serial.println("ADC NEAR FULL SCALE: check clipping; try a smaller resistor.");
    if (lateSamples || r.maxSampleGapUs > SAMPLE_INTERVAL_US * 2)
        Serial.println("SAMPLING GAPS: inspect timestamps; do not treat this as a timing pass.");
    Serial.println("Estimate only: no automatic validation pass. d exports timestamp_us,adc.");
}

void captureSignal() {
    Serial.println("Recording now; no serial output during acquisition...");
    Serial.flush();
    lateSamples = 0;
    const uint32_t start = micros();
    uint32_t deadline = start;
    for (size_t i = 0; i < SAMPLE_COUNT; ++i) {
        while (int32_t(micros() - deadline) < 0) {}
        const uint32_t before = micros();
        if (before - deadline >= SAMPLE_INTERVAL_US) {
            ++lateSamples;
            deadline = before; // don't invent a burst of catch-up samples
        }
        samples[i] = analogRead(SIGNAL_PIN);
        const uint32_t after = micros();
        // Conversion-call midpoint approximation, not a hardware ADC timestamp.
        timestamps[i] = (before - start) + (after - before) / 2;
        deadline += SAMPLE_INTERVAL_US;
    }
    haveCapture = true;
    printReport();
}

void setup() {
    Serial.begin(115200);
    pinMode(SIGNAL_PIN, INPUT); // no pull-up
    analogReadResolution(12);
    // Keep the board core's default reference. Report counts, not assumed volts.
    for (unsigned i = 0; i < 16; ++i) analogRead(SIGNAL_PIN);
    const uint32_t started = millis();
    while (!Serial && millis() - started < 5000) {}
    printHelp();
}

void loop() {
    if (!Serial.available()) return;
    const char command = Serial.read();
    if (command == 'r' || command == 'R') captureSignal();
    else if (command == 'h' || command == 'H') printHelp();
    else if (command == 'd' || command == 'D') {
        if (!haveCapture) { Serial.println("Send r first."); return; }
        Serial.println("timestamp_us,adc");
        for (size_t i = 0; i < SAMPLE_COUNT; ++i) {
            Serial.print(timestamps[i]); Serial.print(','); Serial.println(samples[i]);
        }
    }
}
