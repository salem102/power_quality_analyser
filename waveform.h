// waveform.h
// Power Quality Waveform Analyser

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double timestamp;
    double phase_A_voltage;
    double phase_B_voltage;
    double phase_C_voltage;
    double line_current;
    double frequency;
    double power_factor;
    double thd_percent;
} WaveformSample;

typedef struct {
    double rms_voltage;
    double peak_to_peak;
    double dc_offset;
    double std_dev;
    double variance;
    int    clipped_count;
    int    compliant;
    uint8_t status_flags;
} PhaseMetrics;

#define FLAG_CLIPPING   (1 << 0)
#define FLAG_OUT_OF_TOL (1 << 1)
#define NOMINAL_VOLTAGE 230.0
#define TOLERANCE_LOWER 207.0
#define TOLERANCE_UPPER 253.0
#define CLIP_THRESHOLD  324.9

double compute_rms(const WaveformSample *samples, int n, size_t phase_offset);
double compute_peak_to_peak(const WaveformSample *samples, int n, size_t phase_offset);

#endif // WAVEFORM_H
