#include "waveform.h"
#include <math.h>
#include <stddef.h>

static double get_voltage(const WaveformSample *s, int phase_offset) {
    const char *base = (const char *)s;
    const double *field = (const double *)(base + phase_offset);
    return *field;
}


double compute_rms(const WaveformSample *samples, int n, int phase_offset) {
    double sum_sq = 0.0;
    for (int i = 0; i < n; i++) {
        double v = get_voltage(&samples[i], phase_offset);
        sum_sq += v * v;
    }
    return sqrt(sum_sq / (double)n);
}
double compute_std_dev(const WaveformSample *samples, int n, int phase_offset, double dc) {
    double sum_sq_diff = 0.0;
    for (int i = 0; i < n; i++) {
        double v = get_voltage(&samples[i], phase_offset);
        double diff = v - dc;
        sum_sq_diff += diff * diff;
    }
    // This closes the for loop
    return sqrt(sum_sq_diff / (double) n); // This closes the function
// Fixed to match the prototypes in header
}
int count_clipped(const WaveformSample *samples, int n, int phase_offset){
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(get_voltage(&samples[i], phase_offset)) >= CLIP_THRESHOLD) {
            count++;

        }
    }
    return count;
}

PhaseResult analyse_phase(const WaveformSample *samples, int n, char label, int phase_offset) {
    PhaseResult res = {0};
    res.phase_label = label;
    res.rms_voltage = compute_rms(samples, n, phase_offset);

    double v_max = get_voltage(&samples[0], phase_offset);
    double v_min = v_max;
    double sum = 0;

    for(int i = 0; i < n; i++) {
        double v = get_voltage(&samples[i], phase_offset);
        sum += v;
        if(v > v_max) v_max = v;
        if(v < v_min) v_min = v;
    }

    res.max_voltage = v_max;
    res.min_voltage = v_min;
    res.peak_to_peak = v_max - v_min; // Added this line
    res.dc_offset = sum / n;
    res.clipped_count = count_clipped(samples, n, phase_offset);

    if (res.clipped_count > 0) res.status_flags |= FLAG_CLIPPING;
    if (res.rms_voltage < TOLERANCE_LOW || res.rms_voltage > TOLERANCE_HIGH)
        res.status_flags |= FLAG_OUT_OF_TOL;

    return res;
}

// Keep your mean_frequency, mean_power_factor, and mean_thd as they were.
double mean_frequency(const WaveformSample *samples, int n) {
    double sum = 0;
    for(int i=0; i<n; i++) sum += samples[i].frequency;
    return sum / (double)n;
}

double mean_power_factor(const WaveformSample *samples, int n) {
    double sum = 0;
    for(int i=0; i<n; i++) sum += samples[i].power_factor;
    return sum / (double)n;
}

double mean_thd(const WaveformSample *samples, int n) {
    double sum = 0;
    for(int i=0; i<n; i++) sum += samples[i].thd_percent;
    return sum / (double)n;
}
