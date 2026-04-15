// waveform.c
// Power Quality Waveform Analyser
//
// This file contains all the math functions for analysing the waveforms.

#include "waveform.h"
#include <math.h>
#include <stddef.h>

// get_voltage - helper function to get the voltage from a sample
// I cast the struct to a char pointer so I can just add the byte
// offset to find the right phase voltage, then cast it back to a
// double pointer to read the value.
static double get_voltage(const WaveformSample *sample, size_t offset)
{
    const char *base = (const char *)sample;
    const double *field = (const double *)(base + offset);
    return *field;
}

// compute_rms
// Calculates the Root Mean Square voltage.
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
// Finds the difference between the highest and lowest voltage.
double compute_peak_to_peak(const WaveformSample *samples, int n, size_t phase_offset)
{
    // start by assuming the first sample is both min and max
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
// Finds the average voltage over the whole waveform.
// For AC, it should be close to 0V.
double compute_dc_offset(const WaveformSample *samples, int n, size_t phase_offset)
{
    double sum = 0.0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        sum += get_voltage(ptr, phase_offset);
    }
    return sum / (double)n;
}

// compute_variance (done for Merit/Distinction marks)
// Works out how spread out the voltages are from the mean.
double compute_variance(const WaveformSample *samples, int n,
                        size_t phase_offset, double mean)
{
    double sum_sq_dev = 0.0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        double dev = get_voltage(ptr, phase_offset) - mean;
        sum_sq_dev += dev * dev;
    }
    return sum_sq_dev / (double)n;
}

// compute_std_dev (done for Merit/Distinction marks)
double compute_std_dev(const WaveformSample *samples, int n,
                       size_t phase_offset, double mean)
{
    return sqrt(compute_variance(samples, n, phase_offset, mean));
}

// count_clipped
// Counts how many samples hit the sensor's maximum limit.
int count_clipped(const WaveformSample *samples, int n, size_t phase_offset)
{
    int count = 0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        double v = get_voltage(ptr, phase_offset);
        // flip negative voltages to positive for an easier check
        if (v < 0.0) v = -v;
        if (v >= CLIP_THRESHOLD) count++;
    }
    return count;
}

// check_compliance
// Checks if the RMS voltage is within the EN 50160 limits
int check_compliance(double rms)
{
    return (rms >= TOLERANCE_LOWER && rms <= TOLERANCE_UPPER) ? 1 : 0;
}

// encode_status_flags (done for Distinction marks)
// Packs the clipping and compliance status into a single byte
uint8_t encode_status_flags(int clipped_count, int compliant)
{
    uint8_t flags = 0x00; // start with all bits set to 0

    if (clipped_count > 0) {
        flags |= FLAG_CLIPPING;   // flip bit 0 to 1
    }
    if (!compliant) {
        flags |= FLAG_OUT_OF_TOL; // flip bit 1 to 1
    }
    return flags;
}

// compute_mean_frequency
double compute_mean_frequency(const WaveformSample *samples, int n)
{
    double sum = 0.0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        sum += ptr->frequency;
    }
    return sum / (double)n;
}

// compute_mean_power_factor
double compute_mean_power_factor(const WaveformSample *samples, int n)
{
    double sum = 0.0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        sum += ptr->power_factor;
    }
    return sum / (double)n;
}

// compute_mean_thd
double compute_mean_thd(const WaveformSample *samples, int n)
{
    double sum = 0.0;
    const WaveformSample *ptr = samples;

    for (int i = 0; i < n; i++, ptr++) {
        sum += ptr->thd_percent;
    }
    return sum / (double)n;
}

// analyse_phase
// A handy function that calls all the individual calculation
// functions and saves the results into a PhaseMetrics struct.
void analyse_phase(const WaveformSample *samples, int n,
                   size_t phase_offset, PhaseMetrics *out)
{
    out->rms_voltage   = compute_rms(samples, n, phase_offset);
    out->peak_to_peak  = compute_peak_to_peak(samples, n, phase_offset);
    out->dc_offset     = compute_dc_offset(samples, n, phase_offset);
    out->variance      = compute_variance(samples, n, phase_offset, out->dc_offset);
    out->std_dev       = compute_std_dev(samples, n, phase_offset, out->dc_offset);
    out->clipped_count = count_clipped(samples, n, phase_offset);
    out->compliant     = check_compliance(out->rms_voltage);
    out->status_flags  = encode_status_flags(out->clipped_count, out->compliant);
}
