#include "../firmware/photodiode_validation/signal_analysis.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>

int main() {
    const size_t n = 4000;
    static uint16_t values[n];
    static uint32_t times[n];
    for (double hz : {8.571428571, 10.0, 12.0, 15.0, 30.0}) {
        for (size_t i = 0; i < n; ++i) {
            times[i] = uint32_t(i * 500);
            const double phase = fmod(times[i] * hz / 1e6, 1.0);
            values[i] = (phase < 0.4 ? 1800 : 300) + int(i % 7);
        }
        const auto r = analyzeSignal(values, times, n);
        assert(fabs(r.frequencyHz - hz) < 0.02);
        assert(r.periods >= 15 && r.maxSampleGapUs == 500);
        assert(fabs(r.sampleRateHz - 2000) < 0.01);
    }
    for (size_t i = 0; i < n; ++i) values[i] = 300 + i % 5;
    assert(analyzeSignal(values, times, n).frequencyHz == 0);
    for (size_t i = 0; i < n; ++i) values[i] = 4095;
    assert(analyzeSignal(values, times, n).clippedHigh == n);
    for (size_t i = n/2; i < n; ++i) times[i] += 5000;
    assert(analyzeSignal(values, times, n).maxSampleGapUs == 5500);
    assert(analyzeSignal(values, times, 0).frequencyHz == 0);
    puts("Photodiode analysis tests passed.");
}
