// io.c
// Power Quality Waveform Analyser
//
// Handles reading the CSV file and writing the results report.

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// max characters we expect on a single CSV line
#define MAX_LINE_LEN  512

// start by allocating space for 1100 rows - the file has around 1000
#define INITIAL_CAPACITY 1100

// load_csv
// Opens the file, skips the header, then reads each row into
// a WaveformSample struct. Returns a pointer to the array.
WaveformSample *load_csv(const char *filename, int *out_count)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open '%s'. Check the filename.\n", filename);
        *out_count = 0;
        return NULL;
    }

    // allocate the initial array
    WaveformSample *samples = (WaveformSample *)malloc(
        INITIAL_CAPACITY * sizeof(WaveformSample));
    if (samples == NULL) {
        fprintf(stderr, "Error: malloc failed.\n");
        fclose(fp);
        *out_count = 0;
        return NULL;
    }

    char line[MAX_LINE_LEN];

    // skip the header row
    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: '%s' appears to be empty.\n", filename);
        free(samples);
        fclose(fp);
        *out_count = 0;
        return NULL;
    }

    int count    = 0;
    int capacity = INITIAL_CAPACITY;

    // read each data row one by one
    while (fgets(line, sizeof(line), fp) != NULL) {

        // ignore blank lines
        if (line[0] == '\n' || line[0] == '\r') continue;

        // if we somehow run out of space, grow the array
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

        // point to the current slot using pointer arithmetic
        WaveformSample *row = samples + count;

        // parse the 8 comma-separated values from this line
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
            // row is incomplete - skip it and warn
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

// flag_description - helper to turn a status flag byte into text
static const char *flag_description(uint8_t flags)
{
    if (flags == 0x00) return "0x00 - No anomalies";
    if (flags == 0x01) return "0x01 - Clipping detected";
    if (flags == 0x02) return "0x02 - Out of tolerance";
    if (flags == 0x03) return "0x03 - Clipping + Out of tolerance";
    return "UNKNOWN";
}

// write_results
// Writes the full analysis report to results.txt.
int write_results(const char *filename,
                  const WaveformSample *samples,
                  int n,
                  const PhaseMetrics *phase_a,
                  const PhaseMetrics *phase_b,
                  const PhaseMetrics *phase_c)
{
    FILE *fp = fopen("results.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not create results.txt.\n");
        return -1;
    }

    // work out system-wide averages and min/max values
    double mean_freq = 0.0, mean_pf = 0.0, mean_thd = 0.0;
    double freq_min  = samples->frequency;
    double freq_max  = freq_min;
    double pf_min    = samples->power_factor;
    double pf_max    = pf_min;
    double thd_min   = samples->thd_percent;
    double thd_max   = thd_min;

    const WaveformSample *ptr = samples;
    for (int i = 0; i < n; i++, ptr++) {
        mean_freq += ptr->frequency;
        mean_pf   += ptr->power_factor;
        mean_thd  += ptr->thd_percent;
        if (ptr->frequency    < freq_min) freq_min = ptr->frequency;
        if (ptr->frequency    > freq_max) freq_max = ptr->frequency;
        if (ptr->power_factor < pf_min)   pf_min   = ptr->power_factor;
        if (ptr->power_factor > pf_max)   pf_max   = ptr->power_factor;
        if (ptr->thd_percent  < thd_min)  thd_min  = ptr->thd_percent;
        if (ptr->thd_percent  > thd_max)  thd_max  = ptr->thd_percent;
    }
    mean_freq /= n;
    mean_pf   /= n;
    mean_thd  /= n;

    int total_clipped = phase_a->clipped_count +
                        phase_b->clipped_count +
                        phase_c->clipped_count;

    // write report header section
    fprintf(fp, "Power Quality Waveform Analysis Report\n");
    fprintf(fp, "\n");
    fprintf(fp, "Input file  : %s\n", filename);
    fprintf(fp, "Total rows  : %d\n", n);
    fprintf(fp, "Sample rate : 5000 Hz  (0.2 ms spacing)\n");
    fprintf(fp, "Window      : %.4f s  (~10 cycles at 50 Hz)\n",
            (samples + n - 1)->timestamp);
    fprintf(fp, "\n");

    // per-phase metrics
    const char  *phase_names[3]   = { "Phase A", "Phase B", "Phase C" };
    const PhaseMetrics *phases[3] = { phase_a,   phase_b,   phase_c   };

    for (int p = 0; p < 3; p++) {
        const PhaseMetrics *pm = phases[p];
        fprintf(fp, "%s\n", phase_names[p]);
        fprintf(fp, "  RMS Voltage        : %.4f V\n",  pm->rms_voltage);
        fprintf(fp, "  Peak-to-Peak       : %.4f V\n",  pm->peak_to_peak);
        fprintf(fp, "  DC Offset          : %+.6f V\n",  pm->dc_offset);
        fprintf(fp, "  Standard Deviation : %.4f V\n",  pm->std_dev);
        fprintf(fp, "  Variance           : %.4f V^2\n",pm->variance);
        fprintf(fp, "  Clipped Samples    : %d\n",       pm->clipped_count);
        fprintf(fp, "  Status Flags       : %s\n",       flag_description(pm->status_flags));
        fprintf(fp, "  EN 50160 Compliant : %s",
                pm->compliant ? "YES (207-253 V)\n" : "NO  *** OUT OF TOLERANCE ***\n");
        fprintf(fp, "\n");
    }

    // system-wide measurements
    fprintf(fp, "System-Wide Measurements\n");
    fprintf(fp, "  Frequency    :  Mean = %.4f Hz,  Min = %.4f Hz,  Max = %.4f Hz\n",
            mean_freq, freq_min, freq_max);
    fprintf(fp, "  Power Factor :  Mean = %.4f,     Min = %.4f,     Max = %.4f\n",
            mean_pf, pf_min, pf_max);
    fprintf(fp, "  THD          :  Mean = %.4f %%,  Min = %.4f %%,  Max = %.4f %%\n",
            mean_thd, thd_min, thd_max);
    fprintf(fp, "\n");

    // clipping summary
    fprintf(fp, "Clipping Summary (threshold |V| >= %.1f V)\n", CLIP_THRESHOLD);
    fprintf(fp, "  Phase A clipped samples : %d\n", phase_a->clipped_count);
    fprintf(fp, "  Phase B clipped samples : %d\n", phase_b->clipped_count);
    fprintf(fp, "  Phase C clipped samples : %d\n", phase_c->clipped_count);
    fprintf(fp, "  Total clipped samples   : %d\n", total_clipped);
    fprintf(fp, "\n");

    // compliance summary table
    fprintf(fp, "Compliance Summary\n");
    fprintf(fp, "  %-10s  %-10s  %-12s  %-10s  %-8s\n",
            "Phase", "RMS (V)", "P-P (V)", "DC Off (V)", "Status");
    for (int p = 0; p < 3; p++) {
        const PhaseMetrics *pm = phases[p];
        fprintf(fp, "  %-10s  %-10.4f  %-12.4f  %-+10.6f  %-8s\n",
                phase_names[p],
                pm->rms_voltage,
                pm->peak_to_peak,
                pm->dc_offset,
                pm->compliant ? "PASS" : "FAIL");
    }
    fprintf(fp, "\n");
    fprintf(fp, "End of Report\n");

    fclose(fp);
    return 0;
}
