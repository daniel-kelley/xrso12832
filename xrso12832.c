/*
   xrso12832.c

   Copyright (c) 2021 by Daniel Kelley <dkelley@gmp.san-jose.ca.us>

   xoshiro128plus 32 bit PRNG testbench

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "xoshiro128plus.h"

struct info {
    uint32_t count;
    uint32_t sidx;
    uint32_t seed[NUM_SEED];
    uint32_t short_jump;
    uint32_t long_jump;
    uint32_t mask;
    uint32_t shift;
    int numbit;
    int raw;
    int verbose;
    int skip;
    int header;
    FILE *output;
    const char *output_file;
};


static inline uint32_t shift_and_mask(struct info *info, uint32_t val)
{
    return (val >> info->shift) & info->mask;
}

static void raw(struct info *info)
{
    /* raw */
    for (;;) {
        uint32_t val = next();
        if (info->mask) {
            val = shift_and_mask(info, val);
        }
        fwrite(&val, sizeof(uint32_t), 1, info->output);
    }
}

static void set_numbit(struct info *info)
{
    uint32_t val = (uint32_t)-1;
    int i;
    val = shift_and_mask(info, val);
    info->numbit = 0;
    /* count bits */
    for (i=0; i<32; i++) {
        if (val&1) {
            info->numbit++;
        }
        val >>= 1;
    }
}

static void header(struct info *info)
{
    uint32_t s0 = get_seed(0);
    uint32_t s1 = get_seed(1);
    uint32_t s2 = get_seed(2);
    uint32_t s3 = get_seed(3);

    fprintf(
        info->output,
        "#==================================================================\n"
        );
    fprintf(
        info->output,
        "# generator xorshiro128 %x %x %x %x\n", s0, s1, s2, s3);
    fprintf(
        info->output,
        "#==================================================================\n");
    fprintf(
        info->output,
        "type: d");
    fprintf(
        info->output,
        "count: %u\n", info->count);
    fprintf(
        info->output,
        "numbit: %d\n", info->numbit);
}

static void cooked(struct info *info)
{
    if (info->mask) {
        set_numbit(info);
    }

    if (info->header) {
        header(info);
    }

    while (info->count--) {
        uint32_t val = next();
        if (info->mask) {
            val = shift_and_mask(info, val);
        }
        fprintf(info->output, "%u\n", val);
    }
}

int set_seed(struct info *info, uint32_t val)
{
    int err = 1;

    if (info->sidx < NUM_SEED) {
        info->seed[info->sidx++] = val;
        err = 0;
    } else {
        fprintf(stderr, "Too many seeds\n");
    }

    return err;
}

static int run(struct info *info)
{
    uint32_t i;

    if (info->sidx == 0) {
        /* Start with something... */
        set_seed(info, 1);
    }
    seed(info->seed);

    for (i=0; i<info->long_jump; i++) {
        long_jump();
    }

    for (i=0; i<info->short_jump; i++) {
        jump();
    }

    if (info->raw) {
        raw(info);
    } else {
        cooked(info);
    }

    return 0;
}

static void usage(const char *prog)
{
    fprintf(stderr,"%s \n", prog);
    fprintf(stderr,"  -h        Print this message\n");
    fprintf(stderr,"  -r        Raw mode\n");
    fprintf(stderr,"  -H        Print dieharder header in cooked mode\n");
    fprintf(stderr,"  -o file   output file\n");
    fprintf(stderr,"  -n count  #numbers\n");
    fprintf(stderr,"  -s val    Initial seed(s)\n");
    fprintf(stderr,"  -j n      #short jumps\n");
    fprintf(stderr,"  -l n      #long jumps\n");
    fprintf(stderr,"  -M n      integer mask\n");
    fprintf(stderr,"  -S n      integer shift\n");
}

int main(int argc, char *argv[])
{
    int err = 0;
    int c;
    struct info info;

    memset(&info, 0, sizeof(info));
    info.numbit = 32;
    info.output = stdout;

    while ((c = getopt(argc, argv, "n:s:j:l:o:M:S:rvHh")) != EOF) {
        switch (c) {
        case 'n':
            info.count = strtoul(optarg,0,0);
            break;
        case 's':
            if (!err) {
                err = set_seed(&info, strtoul(optarg,0,0));
            }
            break;
        case 'j':
            info.short_jump = strtol(optarg,0,0);
            break;
        case 'l':
            info.long_jump = strtol(optarg,0,0);
            break;
        case 'o':
            info.output_file = optarg;
            break;
        case 'M':
            info.mask = strtol(optarg,0,0);
            break;
        case 'S':
            info.shift = strtol(optarg,0,0);
            break;
        case 'r':
            info.raw = 1;
            break;
        case 'v':
            info.verbose = 1;
            break;
        case 'H':
            info.header = 1;
            break;
        case 'h':
            usage(argv[0]);
            err = EXIT_SUCCESS;
            info.skip = 1;
            break;
        default:
            break;
        }
    }

    if (info.output_file) {
        info.output = fopen(info.output_file, "w");
        if (!info.output) {
            fprintf(
                stderr,
                "%s: %s\n",
                info.output_file,
                strerror(errno));
            err = 2;
        }
    }

    if (!err && !info.skip) {
        err = run(&info);
    }

    if (info.output) {
        fclose(info.output);
    }


    return err;
}
