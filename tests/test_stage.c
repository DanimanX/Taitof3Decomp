/* Differential test: StageTimeline_Step (C) vs MAME pre/post pairs captured
 * at 0x13F2D4 -> 0x13F2CA / 0x13F342:
 *   scripts/capture_fn.sh 13F2D4 13F2CA,13F342 600 work/ram/stagetl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capture.h"
#include "../src/stage/stage_timeline.h"

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

static int total_spawned, total_ends, total_fx;

/* returns mismatching frames, -1 if capture missing */
static int run_one(const char *pfx)
{
    char p1[256], p2[256];
    snprintf(p1, sizeof p1, "%s_pairs.bin", pfx);
    snprintf(p2, sizeof p2, "%s_regs.tsv", pfx);
    long cl = 0, rl = 0;
    uint8_t *cap = slurp(p1, &cl), *rom = slurp("work/maincpu.bin", &rl);
    FILE *rf = fopen(p2, "r");
    if (!cap || !rom || !rf) { printf("stage: SKIP (need %s_* and work/maincpu.bin)\n", pfx); if (rf) fclose(rf); free(cap); free(rom); return -1; }
    int pairs = (int)(cl / (2 * F3_WRAM_SIZE));
    uint32_t *sp = calloc((size_t)pairs, sizeof *sp), *exitpc = calloc((size_t)pairs, sizeof *exitpc);
    char line[512];
    (void)rl;
    if (fgets(line, sizeof line, rf))
        while (fgets(line, sizeof line, rf)) {
            int i; char which[8];
            if (sscanf(line, "%d\t%7s", &i, which) != 2 || i >= pairs) continue;
            char *c = line;
            for (int t = 0; t < 18 && c; t++) c = strchr(c + 1, '\t');
            if (c && !strcmp(which, "pre")) sp[i] = (uint32_t)strtoul(c + 1, NULL, 16);
        }
    fclose(rf);
    uint8_t *work = malloc(F3_WRAM_SIZE);
    int ok = 0, shown = 0, spawned = 0, moved = 0, ends = 0, fx = 0;
    for (int i = 0; i < pairs; i++) {
        const uint8_t *pre = cap + (size_t)i * 2 * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
        memcpy(work, pre, F3_WRAM_SIZE);
        f3mem_t m = { work, rom, NULL, NULL, NULL };
        uint16_t before = rd16(&m, TL_LAST_POS);
        uint8_t fx_before = rd8(&m, TL_FX_STARTED);
        ends += StageTimeline_Step(&m, &spawned);
        fx += !fx_before && rd8(&m, TL_FX_STARTED);
        moved += rd16(&m, TL_LAST_POS) != before;
        int d = 0;
        for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
            uint32_t a = F3_WRAM_BASE + o;
            if (a >= sp[i] - 0x40 && a < sp[i]) continue;   /* callee save areas */
            if (work[o] != post[o]) {
                if (shown++ < 10) printf("  pair %d @%06X got=%02X want=%02X\n", i, a, work[o], post[o]);
                d++;
            }
        }
        ok += d == 0;
    }
    printf("stage: StageTimeline_Step %-24s %3d/%3d frames bit-exact (%d pos changes, %d waves, %d end markers, %d fx starts)\n",
           pfx, ok, pairs, moved, spawned, ends, fx);
    total_spawned += spawned; total_ends += ends; total_fx += fx;
    free(work); free(cap); free(rom); free(sp); free(exitpc);
    return pairs - ok;
}

static int test_stage_scroll(void)
{
    capture_t c;
    long rl = 0;
    uint8_t *rom = slurp("work/maincpu.bin", &rl);
    (void)rl;
    if (!rom || cap_load(&c, "work/ram/fn_16A3E0") || !c.gfx) {
        printf("stage: StageScroll_Step SKIP (need work/ram/fn_16A3E0_* and work/maincpu.bin)\n");
        free(rom);
        return 0;
    }
    uint8_t *work = malloc(F3_WRAM_SIZE), *gfx = malloc(F3_GFX_SIZE);
    int ok = 0, budget = 10;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(work, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        memcpy(gfx, cap_gfx(&c, i, 0), F3_GFX_SIZE);
        f3mem_t m = { work, rom, NULL, NULL, gfx };
        StageScroll_Step(&m);
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff("wram", i, F3_WRAM_BASE, work, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x100u, sp, &budget)
              + cap_diff("gfx", i, F3_GFX_BASE, gfx, cap_gfx(&c, i, 1), F3_GFX_SIZE, 0, 0, &budget);
        ok += d == 0;
    }
    printf("stage: StageScroll_Step %-24s %3d/%3d calls bit-exact (Work RAM + gfx RAM)\n",
           "work/ram/fn_16A3E0", ok, c.pairs);
    free(work); free(gfx); free(rom); cap_free(&c);
    return c.pairs - ok;
}

int main(void)
{
    /* natural play; forced end marker (F3_POKE32=40B308=0013FC34,40B30C=173F0000);
       forced fx start (F3_POKE32=40B318=00000000) */
    static const char *caps[] = { "work/ram/stagetl", "work/ram/stagetl_end", "work/ram/stagetl_fx" };
    int bad = 0, any = 0;
    for (int k = 0; k < 3; k++) {
        int r = run_one(caps[k]);
        if (r < 0) continue;
        any = 1; bad += r;
    }
    bad += test_stage_scroll();
    if (!any) return 0;
    return bad == 0 && total_spawned > 0 ? 0 : 1;
}
