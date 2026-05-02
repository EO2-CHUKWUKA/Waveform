// Created by EO2-CHUKWUKA on 28/04/2026.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Waveform.h"    // Needed so it understands PhaseResult
#include "InputOutput.h"
/* Maximum characters in one CSV line (8 doubles, commas, newline) */
#define MAX_LINE_LEN 512

/* ---------------------------------------------------------------
 * load_csv
 * 1. Open the file — exit early with NULL if not found.
 * 2. Skip the header row.
 * 3. Count data rows with a first pass, so we can malloc exactly
 *    the right amount of memory.
 * 4. Rewind, skip header again, then parse each row into a struct.
 * --------------------------------------------------------------- */
WaveformSample *load_csv(const char *filepath, int *n_out)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("ERROR: Cannot open file '%s'\n", filepath);
        return NULL;
    }

    char line[MAX_LINE_LEN];

    /* --- Pass 1: count data rows --- */
    /* Skip header */
    if (!fgets(line, sizeof(line), fp)) {
        printf("ERROR: File '%s' appears to be empty.\n", filepath);
        fclose(fp);
        return NULL;
    }

    int row_count = 0;
    while (fgets(line, sizeof(line), fp)) {
        /* Skip blank lines */
        if (line[0] != '\n' && line[0] != '\r' && line[0] != '\0')
            row_count++;
    }

    if (row_count == 0) {
        printf("ERROR: No data rows found in '%s'.\n", filepath);
        fclose(fp);
        return NULL;
    }

    /* --- Allocate exactly row_count structs --- */
    WaveformSample *samples = (WaveformSample *)malloc(
            (size_t)row_count * sizeof(WaveformSample));

    if (!samples) {
        printf("ERROR: malloc failed — out of memory.\n");
        fclose(fp);
        return NULL;
    }

    /* --- Pass 2: parse each row --- */
    rewind(fp);
    fgets(line, sizeof(line), fp);   /* skip header again */

    WaveformSample *ptr = samples;   /* use pointer to walk the array */
    int parsed = 0;

    while (parsed < row_count && fgets(line, sizeof(line), fp)) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
            continue;

        /* scanf parses all 8 comma-separated doubles in one call */
        int fields = sscanf(line,
                            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                            &ptr->timestamp,
                            &ptr->phase_A_voltage,
                            &ptr->phase_B_voltage,
                            &ptr->phase_C_voltage,
                            &ptr->line_current,
                            &ptr->frequency,
                            &ptr->power_factor,
                            &ptr->thd_percent);

        if (fields != 8) {
            printf("WARNING: Row %d only parsed %d fields — skipping.\n",
                    parsed + 1, fields);
            continue;
        }

        ptr++;     /* advance pointer to next struct slot */
        parsed++;
    }

    fclose(fp);
    *n_out = parsed;
    return samples;
}

/* ---------------------------------------------------------------
 * format_report_to_file
 * Shared formatting logic — writes to any FILE* (file or stdout).
 * --------------------------------------------------------------- */
static void format_report(FILE *out,
                          const PhaseResult results[3],
                          double mean_freq,
                          double mean_pf,
                          double mean_thd_val,
                          int    total_clipped)
{
    fprintf(out, "============================================================\n");
    fprintf(out, "  POWER QUALITY WAVEFORM ANALYSER — RESULTS REPORT\n");
    fprintf(out, "  UGMFGT-15-1 Programming for Engineers\n");
    fprintf(out,"  Student ID: 25049425\n ");
    fprintf(out, "  Standard: EN 50160  |  Nominal: 230 V  |  50 Hz\n");
    fprintf(out, "============================================================\n\n");

    /* --- Per-phase results --- */
    for (int i = 0; i < 3; i++) {
        const PhaseResult *r = &results[i];
        int compliant = !(r->status_flags & FLAG_OUT_OF_TOL);
        int clipping  =  (r->status_flags & FLAG_CLIPPING);

        fprintf(out, "------------------------------------------------------------\n");
        fprintf(out, "  PHASE %c\n", r->phase_label);
        fprintf(out, "------------------------------------------------------------\n");
        fprintf(out, "  RMS Voltage        : %8.4f V   [%s]\n",
                r->rms_voltage,
                compliant ? "COMPLIANT (207-253 V)" : "*** OUT OF TOLERANCE ***");
        fprintf(out, "  Peak-to-Peak       : %8.4f V\n", r->peak_to_peak);
        fprintf(out, "  DC Offset          : %+.6f V\n",  r->dc_offset);
        fprintf(out, "  Std Deviation      : %8.4f V\n", r->std_dev);
        fprintf(out, "  Clipped Samples    : %d%s\n",
                r->clipped_count,
                clipping ? "  [CLIPPING DETECTED]" : "");
        if (clipping && r-> clipped_count > 0) {
            fprintf(out, ">> clipping Row log (Timestamps):\n");
            for (int j=0; j < r-> clipped_count; j++) {
                //This prints the row/timestamp we saved earlier in Waveform.c
                fprintf(out, "     - Row Timestamp: %.6f s | Column: Phase %c\n",
                        r->clip_timestamps[j], r->phase_label);
            }
        }
        fprintf(out, "  Status Flags (hex) : 0x%02X\n", r->status_flags);
        fprintf(out, "    bit 0 (clipping)     : %d\n",
                (r->status_flags & FLAG_CLIPPING)   ? 1 : 0);
        fprintf(out, "    bit 1 (out-of-tol)   : %d\n",
                (r->status_flags & FLAG_OUT_OF_TOL) ? 1 : 0);
        fprintf(out, "\n");
    }

    /* --- System-wide results --- */
    fprintf(out, "------------------------------------------------------------\n");
    fprintf(out, "  SYSTEM-WIDE METRICS\n");
    fprintf(out, "------------------------------------------------------------\n");
    fprintf(out, "  Total Clipped Samples : %d  (all phases combined)\n",
            total_clipped);
    fprintf(out, "  Mean Frequency        : %.4f Hz  (nominal 50.0 Hz)\n",
            mean_freq);
    fprintf(out, "  Mean Power Factor     : %.4f      (ideal 1.0)\n",
            mean_pf);
    fprintf(out, "  Mean THD              : %.4f %%   (<5%% = clean, EN 50160 limit 8%%)\n",
            mean_thd_val);
    fprintf(out, "\n");
    fprintf(out, "============================================================\n");
    fprintf(out, "THIS IS MY END OF REPORT\n");
    fprintf(out, "============================================================\n");
}

/* ---------------------------------------------------------------
 * write_report — writes to results.txt
 * --------------------------------------------------------------- */
int write_report(const char *output_path,
                 const PhaseResult results[3],
                 double mean_freq,
                 double mean_pf,
                 double mean_thd_val,
                 int    total_clipped)
{
    FILE *fp = fopen(output_path, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot write to '%s'\n", output_path);
        return -1;
    }

    format_report(fp, results, mean_freq, mean_pf, mean_thd_val, total_clipped);
    fclose(fp);
    return 0;
}

/* ---------------------------------------------------------------
 * print_report — prints to stdout
 * --------------------------------------------------------------- */
void print_report(const PhaseResult results[3],
                  double mean_freq,
                  double mean_pf,
                  double mean_thd_val,
                  int    total_clipped)
{
    format_report(stdout, results, mean_freq, mean_pf, mean_thd_val, total_clipped);
}

