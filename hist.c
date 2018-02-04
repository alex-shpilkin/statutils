#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* BSD sysexits.h */
#define EX_USAGE    64
#define EX_DATAERR  65
#define EX_IOERR    74

#define SIMS 32767
#define BINS  1023

#ifndef ARG_MAX
# define QUAS 128
#else
# define QUAS ARG_MAX
#endif

unsigned long hist[BINS+1][SIMS+1]; /* = 0 */

void sort(unsigned long *lo, unsigned long *hi) {
    if (lo >= hi - 1) return;

    unsigned long pivot = *lo;
    unsigned long *p = lo, *q = hi;
    for (;;) {
        while (*p < pivot) p++;
        do q--; while (*q > pivot);
        if (p >= q) break;
        unsigned long t = *p; *p = *q; *q = t;
    }

    sort(lo, q);
    sort(q + 1, hi);

#ifndef NDEBUG
    for (unsigned long *p = lo + 1; p < hi; p++)
        assert(p[-1] <= p[0]);
#endif
}

int main(int argc, char **argv) {
    unsigned long sims, bins;
    unsigned quas;
    double quav[QUAS];

    char *end;
    if (argc < 3)
        goto usage;
    if (sims = strtoul(argv[1], &end, 0), *end != '\0')
        goto usage;
    if (bins = strtoul(argv[2], &end, 0), *end != '\0')
        goto usage;
    if (sims > SIMS || bins > BINS || argc > QUAS + 3) {
        fprintf(stderr, "%s: implementation limit exceeded\n", argv[0]);
        return EX_DATAERR;
    }
    quas = argc - 3;
    for (unsigned n = 0; n < quas; n++) {
        if (quav[n] = strtod(argv[n+3], &end), *end != '\0')
            goto usage;
        if (quav[n] < 0.0 || quav[n] > 1.0) {
            fprintf(stderr, "%s: quantile must be between 0 and 1\n", argv[0]);
            return EX_DATAERR;
        }
    }

    unsigned sim = 0, total, value;
    int read;
    while ((errno = 0, read = scanf("%u %u", &total, &value)) == 2) {
        unsigned bin = (unsigned)floor((double)value / total * bins + 0.5);
        hist[bin][sim++] += value;
        if (sim >= sims) sim = 0;
    }
    if (errno != 0) {
        int status = (errno == EILSEQ ? EX_DATAERR : EX_IOERR);
        perror(argv[0]); return status;
    } else if (read != EOF || sim != 0) {
        return EX_DATAERR;
    }

    for (unsigned bin = 0; bin < bins + 1; bin++) {
        sort(&hist[bin][0], &hist[bin][sims+1]); /* this uses [sims] == 0 */
        for (unsigned n = 0; n < quas; n++) {
            /* The choice of <= over < here is fixed by requiring that the */
            /* quantiles be monotonous                                     */
            if (quav[n] <= 0.5)
                sim = floor(sims * quav[n]) + 1;
            else
                sim = ceil(sims * quav[n]);
            printf("%lu%c", hist[bin][sim], n != quas-1 ? '\t' : '\n');
        }
    }

    return 0;

usage:
    fprintf(stderr, "%s: incorrect arguments\n", argv[0]);
    return EX_USAGE;
}
