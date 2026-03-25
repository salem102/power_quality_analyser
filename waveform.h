// waveform.h
// Power Quality Waveform Analyser

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>
#include <stddef.h>

// WaveformSample holds all the data from a single CSV row
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

#endif // WAVEFORM_H
