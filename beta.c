#include <assert.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* BSD sysexits.h */
#define EX_USAGE    64
#define EX_DATAERR  65
#define EX_IOERR    74

#if defined(__GNUC__) || defined(__icc__) || defined(__clang__)
# define unlikely(C) (__builtin_expect(!!(C), 0))
#else
# define unlikely(C) (C)
#endif

static uint64_t randu = UINT64_C(0x70736575646F7261), /* pseudora */
                randv = UINT64_C(0x6E646F6D73656564); /* ndomseed */

static inline uint64_t xor128p(void) {
    /* xorshift128+ by Sebastiano Vigna, arXiv:1404.0390v3 [cs.DS]    */
    /* Period 2^128 - 1, 1-dimensional equidistribution, 8-byte state */
    assert(randu != 0 || randv != 0);

    uint64_t u = randv, v = randu, r = u + v;
    randu = u;
    v ^= v << 23;
    randv = v ^ u ^ (v >> 18) ^ (u >> 5);
    return r;
}

static inline double uniform(void) {
    return (xor128p() >> 1) * 0x1p-63;
}

#define SIZE 32767
static unsigned size;         /* 0 <= sample value <= size < SIZE       */
static double   cdf[SIZE+1];  /* cdf[n] = sum(0 <= k < n) Pr(value = k) */
                              /* cdf[size+1] = 1.0 (not stored)         */

#define EXPSTEP (DBL_MAX_EXP / 2)

static void prepare(unsigned m) {
    assert(m <= size && size < SIZE);

    static int eov[SIZE+1];
    const unsigned l = size - m;
    double p = 1.0, a = 0.0; int e = 0;

    for (unsigned n = 0; n < size + 1; n++) {
        assert(isfinite(a) && isfinite(p));
        cdf[n] = a; eov[n] = e;
        
        double f = ((double)(m + n + 1) / (n + 1)) *
                   ((double)(size - n) / (l + size - n));
        if unlikely (isinf(a + p) || isinf(p * f)) {
            e += EXPSTEP;
            p  = scalbn(p, -EXPSTEP);
            a  = scalbn(a, -EXPSTEP);
        }
        a += p;
        p *= f;
    }

    if unlikely (e != 0) {
        for (unsigned n = 0; n < size + 1; n++)
            cdf[n] = scalbn(cdf[n], eov[n] - e) / a;
    } else {
        for (unsigned n = 0; n < size + 1; n++)
            cdf[n] = cdf[n] / a;
    }

#ifndef NDEBUG
    for (unsigned n = 0; n < size; n++)
        assert(isfinite(cdf[n]));
    for (unsigned n = 0; n < size; n++)
        assert(cdf[n] <= cdf[n+1]);
    assert(cdf[size+1] <= 1.0);
    double maxp = m < size ? cdf[m+1] - cdf[m] : 1.0 - cdf[m];
    for (unsigned n = 0; n < size; n++)
        assert(cdf[n+1] - cdf[n] <= maxp);
#endif
}

static inline unsigned sample(void) {
    double u = uniform();
    unsigned lo = 0, hi = size + 1;
    /* Loop invariant: cdf[lo] <= u < cdf[hi] */
    while (lo < hi - 1) {
        unsigned mi = (hi + lo) / 2;
        if (u < cdf[mi])
            hi = mi;
        else
            lo = mi;
    }
    return lo;
}

int main(int argc, char **argv) {
    unsigned long count;
    char *end;
    if (argc != 2 || (count = strtoul(argv[1], &end, 0), *end != '\0')) {
        fprintf(stderr, "%s: incorrect arguments\n", argv[0]);
        return EX_USAGE;
    }

    unsigned m;
    int read;
    while ((errno = 0, read = scanf("%u %u", &size, &m)) == 2) {
        if unlikely (size >= SIZE) {
            fprintf(stderr, "%s: size too large [maximum=%u]", argv[0], SIZE);
            return EX_DATAERR;
        }
        if unlikely (m >= SIZE) {
            fprintf(stderr, "%s: sample value larger than size", argv[0]);
            return EX_DATAERR;
        }

        prepare(m);
        for (unsigned long n = 0; n < count; n++)
            printf("%u\t%u\n", size, sample());
        fflush(stdout);
    }
    if (errno != 0) {
        int status = (errno == EILSEQ ? EX_DATAERR : EX_IOERR);
        perror(argv[0]); return status;
    } else if (read != EOF) {
        return EX_DATAERR;
    }

    return 0;
}
