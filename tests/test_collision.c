/* Differential test: Task_Collision body (C) vs exact MAME pre/post RAM pairs
 * captured with the debugger at 0xC63F2 / 0xC63F0
 * (scripts/capture_fn.sh C63F2 C63F0 300 work/ram/coll). Whole-RAM compare. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/collision/collision.h"

static uint8_t *slurp(const char *path, long *len)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); *len = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc((size_t)*len);
    if (fread(b, 1, (size_t)*len, f) != (size_t)*len) { free(b); b = NULL; }
    fclose(f);
    return b;
}

int main(int argc, char **argv)
{
    const char *cap_path = argc > 1 ? argv[1] : "work/ram/coll_pairs.bin";
    long cl;
    uint8_t *cap = slurp(cap_path, &cl);
    if (!cap) { printf("collision: SKIP (need %s)\n", cap_path); return 0; }
    int pairs = (int)(cl / (2 * F3_WRAM_SIZE)), ok = 0, shown = 0;
    uint8_t *work = malloc(F3_WRAM_SIZE);
    for (int i = 0; i < pairs; i++) {
        const uint8_t *pre = cap + (size_t)i * 2 * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
        memcpy(work, pre, F3_WRAM_SIZE);
        f3mem_t m = { work, NULL, NULL, NULL, NULL };
        Task_Collision_Body(&m);
        /* ignore the task's own stack (registers saved by the trap) */
        int diffs = 0;
        for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
            uint32_t a = F3_WRAM_BASE + o;
            if (a >= 0x4022B4 && a < 0x4062B4) continue; /* task stacks */
            if (work[o] != post[o]) {
                if (shown < 10) { printf("  pair %d @%06X got=%02X want=%02X\n", i, a, work[o], post[o]); shown++; }
                diffs++;
            }
        }
        ok += diffs == 0;
    }
    printf("collision: %d/%d pairs bit-exact over Work RAM (hits exercised: %u shot-vs-enemy, %u player-vs-hazard, %u bomb)\n",
           ok, pairs, coll_stats.shot_hits, coll_stats.hazard_hits, coll_stats.bomb_hits);
    free(work); free(cap);
    /* require that the capture actually exercised the hit paths */
    return ok == pairs && pairs > 0 && coll_stats.shot_hits && coll_stats.hazard_hits ? 0 : 1;
}
