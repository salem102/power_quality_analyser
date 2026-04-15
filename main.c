// main.c
// Power Quality Waveform Analyser
//
// This is the main entry point. It checks the command line argument,
// loads the CSV file, runs the analysis on each phase, prints the
// results to the screen, then writes everything to a results file.
//
// Run it like this:
//   ./power_analyser power_quality_log.csv

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "waveform.h"
#include "io.h"

int main(int argc, char *argv[])
{
    // need exactly one argument - the csv filename
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <csv_filename>\n", argv[0]);
        fprintf(stderr, "Example: %s power_quality_log.csv\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    // load the data from the csv into an array of structs
    int n = 0;
    WaveformSample *samples = load_csv(filename, &n);

    if (samples == NULL || n == 0) {
        // load_csv already printed what went wrong
        return EXIT_FAILURE;
    }

    // run the analysis for each of the three phases
    PhaseMetrics phase_a, phase_b, phase_c;

    printf("Analysing Phase A...\n");
    analyse_phase(samples, n,
                  offsetof(WaveformSample, phase_A_voltage), &phase_a);

    printf("Analysing Phase B...\n");
    analyse_phase(samples, n,
                  offsetof(WaveformSample, phase_B_voltage), &phase_b);

    printf("Analysing Phase C...\n");
    analyse_phase(samples, n,
                  offsetof(WaveformSample, phase_C_voltage), &phase_c);

    // print a summary of the results to the console
    printf("\nPower Quality Analysis - Results\n\n");

    const char  *names[3]   = { "Phase A", "Phase B", "Phase C" };
    PhaseMetrics *phases[3] = { &phase_a,  &phase_b,  &phase_c  };

    for (int p = 0; p < 3; p++) {
        PhaseMetrics *pm = phases[p];
        printf("%s:\n", names[p]);
        printf("  RMS Voltage  : %.4f V  [%s]\n",
               pm->rms_voltage,
               pm->compliant ? "COMPLIANT" : "NON-COMPLIANT");
        printf("  Peak-to-Peak : %.4f V\n",   pm->peak_to_peak);
        printf("  DC Offset    : %+.6f V\n",  pm->dc_offset);
        printf("  Std Dev      : %.4f V\n",   pm->std_dev);
        printf("  Clipped      : %d samples\n", pm->clipped_count);
        printf("  Status Flags : 0x%02X\n\n",   pm->status_flags);
    }

    int total_clipped = phase_a.clipped_count +
                        phase_b.clipped_count +
                        phase_c.clipped_count;
    printf("Total clipped samples across all phases: %d\n\n", total_clipped);

    // write the full report to results.txt
    printf("Writing results to results.txt...\n");
    int ret = write_results(filename, samples, n,
                            &phase_a, &phase_b, &phase_c);
    if (ret == 0) {
        printf("Results file written ok.\n");
    } else {
        fprintf(stderr, "Error: could not write results file.\n");
    }

    // free the memory we allocated for the samples array
    free(samples);
    samples = NULL;

    printf("Done.\n");
    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
