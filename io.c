// io.c
// Power Quality Waveform Analyser

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN  512
#define INITIAL_CAPACITY 1100

WaveformSample *load_csv(const char *filename, int *out_count)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open '%s'.\n", filename);
        *out_count = 0;
        return NULL;
    }

    WaveformSample *samples = (WaveformSample *)malloc(
        INITIAL_CAPACITY * sizeof(WaveformSample));
    if (samples == NULL) {
        fprintf(stderr, "Error: malloc failed.\n");
        fclose(fp);
        *out_count = 0;
        return NULL;
    }

    char line[MAX_LINE_LEN];

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: '%s' appears to be empty.\n", filename);
        free(samples);
        fclose(fp);
        *out_count = 0;
        return NULL;
    }

    int count = 0;
    int capacity = INITIAL_CAPACITY;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (count >= capacity) {
            capacity += 500;
            WaveformSample *tmp = (WaveformSample *)realloc(
                samples, capacity * sizeof(WaveformSample));
            if (tmp == NULL) {
                fprintf(stderr, "Error: realloc failed at row %d.\n", count);
                free(samples);
                fclose(fp);
                *out_count = 0;
                return NULL;
            }
            samples = tmp;
        }

        WaveformSample *row = samples + count;

        int fields = sscanf(line,
            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &row->timestamp,
            &row->phase_A_voltage,
            &row->phase_B_voltage,
            &row->phase_C_voltage,
            &row->line_current,
            &row->frequency,
            &row->power_factor,
            &row->thd_percent);

        if (fields != 8) {
            fprintf(stderr, "Warning: skipping row %d, only got %d/8 fields.\n",
                    count + 1, fields);
            continue;
        }

        count++;
    }

    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "Error: no valid data found in '%s'.\n", filename);
        free(samples);
        *out_count = 0;
        return NULL;
    }

    *out_count = count;
    printf("Loaded %d samples from '%s'.\n", count, filename);
    return samples;
}
