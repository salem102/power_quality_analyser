// waveform.h
// Power Quality Waveform Analyser
//
// This header defines the structs we use to hold the data and
// lists all the functions that are written in waveform.c.

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>
#include <stddef.h> // for size_t

// WaveformSample
// Holds all the data from a single line in the CSV file.
typedef struct {
    double timestamp;         // Time in seconds
    double phase_A_voltage;   // Phase A voltage (V)
    double phase_B_voltage;   // Phase B voltage (V)
    double phase_C_voltage;   // Phase C voltage (V)
    double line_current;      // Line current (A)
    double frequency;         // Frequency (Hz)
    double power_factor;      // Power factor (0 to 1)
    double thd_percent;       // Total Harmonic Distortion (%)
} WaveformSample;

// PhaseMetrics
// Holds the results of our calculations for a single phase.
typedef struct {
    double rms_voltage;       // RMS voltage
    double peak_to_peak;      // Difference between min and max voltage
    double dc_offset;         // Average voltage (should be near 0)
    double std_dev;           // Standard deviation
    double variance;          // Variance
    int    clipped_count;     // How many times it hit the 324.9V limit
    int    compliant;         // 1 if it passed the EN 50160 check, else 0
    uint8_t status_flags;     // Bit 0 = clipping, Bit 1 = failed check
} PhaseMetrics;

// Bits for the status flags
#define FLAG_CLIPPING       (1 << 0)   // Bit 0: clipping happened
#define FLAG_OUT_OF_TOL     (1 << 1)   // Bit 1: RMS failed the tolerance check

// Some constants we need for the checks
#define NOMINAL_VOLTAGE     230.0      // Standard UK voltage
#define TOLERANCE_LOWER     207.0      // 230V minus 10%
#define TOLERANCE_UPPER     253.0      // 230V plus 10%
#define CLIP_THRESHOLD      324.9      // The sensor's max reading

// Function prototypes
//
// To avoid writing the same function three times, I'm passing
// the byte offset of the phase voltage we want to look at.
// e.g. compute_rms(samples, n, offsetof(WaveformSample, phase_A_voltage))

// Works out the RMS voltage
double compute_rms(const WaveformSample *samples, int n, size_t phase_offset);

// Finds the peak-to-peak voltage
double compute_peak_to_peak(const WaveformSample *samples, int n, size_t phase_offset);

// Works out the DC offset (average voltage)
double compute_dc_offset(const WaveformSample *samples, int n, size_t phase_offset);

// Calculates standard deviation
double compute_std_dev(const WaveformSample *samples, int n, size_t phase_offset,
                       double mean);

// Calculates variance
double compute_variance(const WaveformSample *samples, int n, size_t phase_offset,
                        double mean);

// Counts how many samples hit the sensor limit
int count_clipped(const WaveformSample *samples, int n, size_t phase_offset);

// Checks if RMS voltage is between 207V and 253V
int check_compliance(double rms);

// Packs the clipping and compliance status into a single byte
uint8_t encode_status_flags(int clipped_count, int compliant);

// Calculates the average frequency
double compute_mean_frequency(const WaveformSample *samples, int n);

// Calculates the average power factor
double compute_mean_power_factor(const WaveformSample *samples, int n);

// Calculates the average THD
double compute_mean_thd(const WaveformSample *samples, int n);

// Runs all the checks on one phase and saves the results
void analyse_phase(const WaveformSample *samples, int n,
                   size_t phase_offset, PhaseMetrics *out);

#endif // WAVEFORM_H
