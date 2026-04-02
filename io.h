// io.h
// Power Quality Waveform Analyser

#ifndef IO_H
#define IO_H
#include "waveform.h"

// load_csv reads the CSV and returns a malloc'd array of WaveformSample
WaveformSample *load_csv(const char *filename, int *out_count);

#endif // IO_H
