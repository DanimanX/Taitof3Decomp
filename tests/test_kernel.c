/* Host tests for decompiled kernel/io leaf functions.
 * - rng: golden vectors from MAME (tests/vectors/rng_seed_frames.txt): each
 *   frame's seed must be reachable from the previous frame's seed by iterating
 *   rng_step (the idle loop calls Rng_Next a variable number of times).
 * - io: exhaustive check of IO_EdgeDetect against per-bit truth table.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../src/kernel/rng.h"
#include "../src/io/input.h"

#define MAX_STEPS 2000000

static int test_rng(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) { printf("rng: SKIP (no %s)\n", path); return 0; }
    unsigned fr, seed, extra, prev = 0;
    int have = 0, pairs = 0, fails = 0, stalls = 0;
    long total = 0, maxs = 0;
    while (fscanf(f, "%u %x %x", &fr, &seed, &extra) == 3) {
        if (have) {
            if (seed == prev) { stalls++; }
            else {
                uint32_t s = prev; long n = 0;
                while (s != seed && n < MAX_STEPS) { s = rng_step(s); n++; }
                if (s != seed) { fails++; printf("rng: FAIL frame %u: %08x not reachable from %08x\n", fr, seed, prev); }
                else { total += n; if (n > maxs) maxs = n; }
            }
            pairs++;
        }
        prev = seed; have = 1;
    }
    fclose(f);
    printf("rng: %d frame pairs, %d unchanged, %d fail, steps/frame avg %.1f max %ld\n",
           pairs, stalls, fails, pairs - stalls ? (double)total / (pairs - stalls) : 0.0, maxs);
    /* Rng_Next return value = swapped seed */
    rng_seed = 0x12345678u;
    uint32_t r = Rng_Next();
    uint32_t want = 0x12345678u * 7u + 5u;
    if (rng_seed != want || r != ((want << 16) | (want >> 16))) { printf("rng: FAIL Rng_Next\n"); fails++; }
    return pairs > 0 && fails == 0 ? 0 : (fails ? 1 : 0);
}

static int test_io(void)
{
    int fails = 0;
    for (int cur = 0; cur < 256; cur++)
        for (int prev = 0; prev < 256; prev++) {
            for (int i = 0; i < IO_NUM_CHANNELS; i++) { io_ch[i].cur = (uint8_t)cur; io_ch[i].prev = (uint8_t)prev; }
            IO_EdgeDetect();
            for (int b = 0; b < 8; b++) {
                int c = (cur >> b) & 1, p = (prev >> b) & 1;
                int press = !(c == 0 && p == 1), rel = !(c == 1 && p == 0); /* active-low */
                for (int i = 0; i < IO_NUM_CHANNELS; i++)
                    if (((io_ch[i].pressed >> b) & 1) != press || ((io_ch[i].released >> b) & 1) != rel) fails++;
            }
        }
    printf("io: edge detect %s (%d mismatches)\n", fails ? "FAIL" : "OK", fails);
    return fails != 0;
}

int main(int argc, char **argv)
{
    int rc = 0;
    rc |= test_rng(argc > 1 ? argv[1] : "tests/vectors/rng_seed_frames.txt");
    rc |= test_io();
    return rc;
}
