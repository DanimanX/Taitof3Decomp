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

    /* ROM 0x001410 (Obj_DrawSpriteSub) */
    {
        capture_t c;
        long rl;
        uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
        (void)rl;
        if (rom && !cap_load(&c, "work/ram/fn_001410") && c.spr) {
            uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
                f3mem_t m = { w, rom, NULL, s, NULL };
                Obj_DrawSpriteSub(&m, c.pre[i][CR_A0], (uint16_t)c.pre[i][CR_D0],
                                  (uint16_t)c.pre[i][CR_D0 + 1], (uint16_t)c.pre[i][CR_D0 + 2]);
                uint32_t sp = c.pre[i][CR_SP];
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget)
                      + cap_diff("spr", i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM + sprite RAM)\n", "0x001410", ok, c.pairs);
            bad += c.pairs - ok;
            free(w); free(s);
            cap_free(&c);
        }
        free(rom);
    }

    /* ROM 0x001232 (Video_SetSpriteScroll) */
    {
        capture_t c;
        if (!cap_load(&c, "work/ram/fn_001232") && c.spr) {
            uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
                f3mem_t m = { w, NULL, NULL, s, NULL };
                Video_SetSpriteScroll(&m);
                uint32_t sp = c.pre[i][CR_SP];
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget)
                      + cap_diff("spr", i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM + sprite RAM)\n", "0x001232", ok, c.pairs);
            bad += c.pairs - ok;
            free(w); free(s);
            cap_free(&c);
        }
    }

    /* ROM 0x0055C8, 0x00571A, 0x0058AE, 0x005956, 0x005A86, 0x005B0C */
    {
        long rl;
        uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
        (void)rl;
        static const struct { const char *name, *dir; int id; } fn_tests[] = {
            { "0x0055C8", "work/ram/fn_0055C8", 0 },
            { "0x00571A", "work/ram/fn_00571A", 1 },
            { "0x005880", "work/ram/fn_005880", 6 },
            { "0x0058AE", "work/ram/fn_0058AE", 2 },
            { "0x005956", "work/ram/fn_005956", 3 },
            { "0x005A12", "work/ram/fn_005A12", 7 },
            { "0x005A86", "work/ram/fn_005A86", 4 },
            { "0x005B0C", "work/ram/fn_005B0C", 5 },
        };
        for (size_t t = 0; t < sizeof(fn_tests) / sizeof(fn_tests[0]); t++) {
            capture_t c;
            if (!rom || cap_load(&c, fn_tests[t].dir) || !c.gfx) continue;
            uint8_t *w = malloc(F3_WRAM_SIZE), *g = malloc(F3_GFX_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(g, c.gfx + ((size_t)i * 2) * F3_GFX_SIZE, F3_GFX_SIZE);
                f3mem_t m = { w, rom, NULL, NULL, g };
                uint32_t sp = c.pre[i][CR_SP];
                if (fn_tests[t].id == 0) {
                    Gfx_DrawTilemapBlock(&m, rd32(&m, sp + 4u), rd32(&m, sp + 8u), rd16(&m, sp + 12u));
                } else if (fn_tests[t].id == 1) {
                    Gfx_DrawDecimal(&m, rd32(&m, sp + 4u), rd16(&m, sp + 8u), rd32(&m, sp + 10u), rd16(&m, sp + 14u));
                } else if (fn_tests[t].id == 2) {
                    Gfx_ClearPf3(&m);
                } else if (fn_tests[t].id == 3) {
                    Gfx_ClearPf1AndLineRam(&m);
                } else if (fn_tests[t].id == 4) {
                    Gfx_UploadTextTiles(&m, c.pre[i][CR_A0], (uint16_t)c.pre[i][CR_D0]);
                } else if (fn_tests[t].id == 6) {
                    Gfx_ClearTextLayer(&m);
                } else if (fn_tests[t].id == 7) {
                    Gfx_ClearAllLayers(&m);
                } else {
                    Gfx_InitLineRam(&m);
                }
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x60, sp, &budget)
                      + cap_diff("gfx", i, F3_GFX_BASE, g, c.gfx + ((size_t)i * 2 + 1) * F3_GFX_SIZE, F3_GFX_SIZE, 0, 0, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM + gfx RAM)\n", fn_tests[t].name, ok, c.pairs);
            bad += c.pairs - ok;
            free(w); free(g);
            cap_free(&c);
        }
        free(rom);
    }

    return bad == 0 ? 0 : 1;
}
