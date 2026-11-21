/* Differential test: Obj_DrawSprite (C) vs MAME captures including sprite RAM:
 *   F3_SPR=1 scripts/capture_fn.sh 1282 13CC,140E 400 work/ram/spr */
#include "capture.h"
#include "../src/video/sprite.h"

static int run(const char *pfx, int *total)
{
    capture_t c;
    long rl;
    uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
    (void)rl;
    if (!rom || cap_load(&c, pfx) || !c.spr) { printf("sprite: SKIP (need %s_* with F3_SPR=1)\n", pfx); free(rom); return 0; }
    uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
    int ok = 0, budget = 10, single = 0, multi = 0, flips[4] = {0}, entries = 0;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        f3mem_t m = { w, rom, NULL, s, NULL };
        uint32_t obj = c.pre[i][CR_A0], cur = rd32(&m, SPR_CURSOR);
        if (rd32(&m, rd32(&m, obj + 0x10)) == 0x00010001u) single++; else { multi++; flips[rd8(&m, obj + 0x1C) & 3]++; }
        Obj_DrawSprite(&m, obj);
        entries += (int)((rd32(&m, SPR_CURSOR) - cur) / 16);
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget)
              + cap_diff("spr", i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
        ok += d == 0;
    }
    printf("sprite: %-20s %d/%d calls bit-exact (Work RAM + sprite RAM); %d single, %d multi (flip 0/1/2/3: %d/%d/%d/%d), %d entries\n",
           pfx, ok, c.pairs, single, multi, flips[0], flips[1], flips[2], flips[3], entries);
    free(w); free(s); free(rom);
    *total += c.pairs;
    int bad = c.pairs - ok;
    cap_free(&c);
    return bad;
}

int main(void)
{
    /* natural; flip modes forced with F3_POKEA0B=1C=01/02/03 */
    static const char *caps[] = { "work/ram/spr", "work/ram/spr_flip1", "work/ram/spr_flip2", "work/ram/spr_flip3" };
    int bad = 0, total = 0;
    for (int k = 0; k < 4; k++) bad += run(caps[k], &total);
    return bad == 0 ? 0 : 1;
}
