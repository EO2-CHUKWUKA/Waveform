
// Created by EO2-CHUKWUKA on 28/04/2026.


#ifndef C_PROGRAM_INPUTOUTPUT_H
#define C_PROGRAM_INPUTOUTPUT_H

#include "Waveform.h"

/* ---------------------------------------------------------------
 * load_csv
 * Opens the CSV at 'filepath', allocates a WaveformSample array
 * on the heap (malloc), and fills it with parsed data.
 *
 * Returns: pointer to the allocated array on success, NULL on error.
 * Sets *n_out to the number of rows loaded.
 *
 * Caller is responsible for free()-ing the returned pointer.
 * --------------------------------------------------------------- */
WaveformSample *load_csv(const char *filepath, int *n_out);

/* ---------------------------------------------------------------
 * write_report
 * Writes a plain-text results report to 'output_path'.
 * Includes all three PhaseResults plus system-wide metrics.
 *
 * Returns 0 on success, -1 on error.
 * --------------------------------------------------------------- */
int write_report(const char *output_path,
                 const PhaseResult results[3],
                 double mean_freq,
                 double mean_pf,
                 double mean_thd_val,
                 int    total_clipped);

/* ---------------------------------------------------------------
 * print_report
 * Prints the same report to stdout (for live demo / viva).
 * --------------------------------------------------------------- */
void print_report(const PhaseResult results[3],
                  double mean_freq,
                  double mean_pf,
                  double mean_thd_val,
                  int    total_clipped);

#endif //C_PROGRAM_INPUTOUTPUT_H//
