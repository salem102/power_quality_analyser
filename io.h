// io.h
// Power Quality Waveform Analyser
//
// Header file for the input/output functions.

#ifndef IO_H
#define IO_H

#include "waveform.h"

// load_csv
// Reads the CSV file and puts the data into a dynamically allocated
// array of WaveformSample structs.
// Returns a pointer to the array (remember to free it later!)
// or NULL if it couldn't read the file.
WaveformSample *load_csv(const char *filename, int *out_count);

// write_results
// Writes all our calculation results into a text file called 'results.txt'.
// Returns 0 if it worked, -1 if it failed to create the file.
int write_results(const char *filename,
                  const WaveformSample *samples,
                  int n,
                  const PhaseMetrics *phase_a,
                  const PhaseMetrics *phase_b,
                  const PhaseMetrics *phase_c);

#endif // IO_H
