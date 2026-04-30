#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>

/* ---------------------------------------------------------------
 * WaveformSample
 * Holds all 8 fields from one row of power_quality_log.csv
 * --------------------------------------------------------------- */
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

/*
 * PhaseResult
 * Computed metrics for one phase, plus a bitwise status byte.
 *
 *  Bit layout (uint8_t status_flags):
 *    bit 0  — clipping detected   (1 = yes)
 *    bit 1  — out of tolerance     (1 = yes)
 *    bits 2-7 reserved (0)
 *  */
#define FLAG_CLIPPING     (1u << 0)
#define FLAG_OUT_OF_TOL   (1u << 1)

typedef struct {
        char   phase_label;    // 'A', 'B', or 'C'
        double rms_voltage;    // Calculated RMS
        double max_voltage;    // The highest peak found in the data
        double min_voltage;    // The lowest peak found in the data
        double peak_to_peak;   // The difference (max - min)
        double dc_offset;      // The average voltage
        double std_dev;
        int    clipped_count;  // Number of samples >= 324.9V
        uint8_t status_flags;  // Bitmask for errors
} PhaseResult;

#define NOMINAL_VOLTAGE   230.0
#define TOLERANCE_LOW     207.0   /* 230 * 0.90 */
#define TOLERANCE_HIGH    253.0   /* 230 * 1.10 */
#define CLIP_THRESHOLD    324.9

/* ---------------------------------------------------------------
 * Analysis function prototypes
 * Each operates on a pointer to the sample array.
 * --------------------------------------------------------------- */

/* Compute RMS voltage for one phase.
 * offset_bytes: byte offset into WaveformSample to the phase field */
double compute_rms(const WaveformSample *samples, int n, int phase_offset);

/* Compute peak-to-peak amplitude for one phase. */
double compute_peak_to_peak(const WaveformSample *samples, int n, int phase_offset);

/* Compute DC offset (arithmetic mean) for one phase. */
double compute_dc_offset(const WaveformSample *samples, int n, int phase_offset);

/* Compute population standard deviation for one phase. */
double compute_std_dev(const WaveformSample *samples, int n, int phase_offset, double dc);

/* Count clipped samples (|v| >= CLIP_THRESHOLD) for one phase. */
int count_clipped(const WaveformSample *samples, int n, int phase_offset);

/* Check EN 50160 compliance and return 1 if compliant, 0 if not. */
int check_compliance(double rms);

/* Run full analysis for a single phase. Fills out a PhaseResult. */
PhaseResult analyse_phase(const WaveformSample *samples, int n,
                          char label, int phase_offset);

/* Compute mean frequency across all samples. */
double mean_frequency(const WaveformSample *samples, int n);

/* Compute mean power factor across all samples. */
double mean_power_factor(const WaveformSample *samples, int n);

/* Compute mean THD across all samples. */
double mean_thd(const WaveformSample *samples, int n);

#endif /* WAVEFORM_H */












