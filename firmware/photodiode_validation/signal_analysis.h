#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>

struct SignalReport {
    uint16_t minimum = 4095, maximum = 0;
    unsigned periods = 0, clippedHigh = 0;
    uint32_t minPeriodUs = UINT32_MAX, maxPeriodUs = 0, maxSampleGapUs = 0;
    double frequencyHz = 0, meanPeriodUs = 0, periodStdUs = 0, sampleRateHz = 0;
};

// Offline Schmitt-trigger analysis: thresholds at 35%/65% of measured range.
// This estimates optical edges, not EEG responses or calibrated absolute timing.
inline SignalReport analyzeSignal(const uint16_t* values, const uint32_t* times,
    size_t count, unsigned minimumSpan = 40) {
    SignalReport r;
    if (!count) return r;
    for (size_t i = 0; i < count; ++i) {
        if (values[i] < r.minimum) r.minimum = values[i];
        if (values[i] > r.maximum) r.maximum = values[i];
        if (values[i] >= 4090) ++r.clippedHigh;
        if (i && times[i] - times[i-1] > r.maxSampleGapUs)
            r.maxSampleGapUs = times[i] - times[i-1];
    }
    if (count > 1 && times[count-1] != times[0])
        r.sampleRateHz = 1e6 * (count-1) / (times[count-1] - times[0]);
    const unsigned span = r.maximum - r.minimum;
    if (span < minimumSpan) return r;
    const double low = r.minimum + span * 0.35;
    const double high = r.minimum + span * 0.65;
    bool armed = false, haveRise = false;
    uint32_t lastRise = 0;
    double sum = 0, sumSquares = 0;
    for (size_t i = 0; i < count; ++i) {
        if (values[i] <= low) armed = true;
        if (armed && values[i] >= high) {
            armed = false;
            if (haveRise) {
                const uint32_t period = times[i] - lastRise;
                if (period < r.minPeriodUs) r.minPeriodUs = period;
                if (period > r.maxPeriodUs) r.maxPeriodUs = period;
                sum += period;
                sumSquares += double(period) * period;
                ++r.periods;
            }
            lastRise = times[i];
            haveRise = true;
        }
    }
    if (r.periods && sum > 0) {
        r.meanPeriodUs = sum / r.periods;
        r.frequencyHz = 1e6 / r.meanPeriodUs;
        const double variance = sumSquares / r.periods - r.meanPeriodUs * r.meanPeriodUs;
        r.periodStdUs = sqrt(variance > 0 ? variance : 0);
    }
    return r;
}
