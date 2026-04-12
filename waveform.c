// waveform.c
// Power Quality Waveform Analyser

#include "waveform.h"
#include <math.h>
#include <stddef.h>

static double get_voltage(const WaveformSample *sample, size_t offset)
{
    const char *base = (const char *)sample;
    const double *field = (const double *)(base + offset);
    return *field;
}

// compute_rms
double compute_rms(const WaveformSample *samples, int n, size_t phase_offset)
{
    double sum_sq = 0.0;
    const WaveformSample *ptr = samples;
    for (int i = 0; i < n; i++, ptr++) {
        double v = get_voltage(ptr, phase_offset);
        sum_sq += v * v;
    }
    return sqrt(sum_sq / (double)n);
}

// compute_peak_to_peak
double compute_peak_to_peak(const WaveformSample *samples, int n, size_t phase_offset)
{
    double vmax = get_voltage(samples, phase_offset);
    double vmin = vmax;
    const WaveformSample *ptr = samples + 1;
    for (int i = 1; i < n; i++, ptr++) {
        double v = get_voltage(ptr, phase_offset);
        if (v > vmax) vmax = v;
        if (v < vmin) vmin = v;
    }
    return vmax - vmin;
}

// compute_dc_offset
double compute_dc_offset(const WaveformSample *samples, int n, size_t phase_offset)
{
    double sum = 0.0;
    const WaveformSample *ptr = samples;
    for (int i = 0; i < n; i++, ptr++) {
        sum += get_voltage(ptr, phase_offset);
    }
    return sum / (double)n;
}

// count_clipped
int count_clipped(const WaveformSample *samples, int n, size_t phase_offset)
{
    int count = 0;
    const WaveformSample *ptr = samples;
    for (int i = 0; i < n; i++, ptr++) {
        double v = get_voltage(ptr, phase_offset);
        if (v < 0.0) v = -v;
        if (v >= CLIP_THRESHOLD) count++;
    }
    return count;
}
