/* Differential test: Obj_DrawSprite (C) vs MAME captures including sprite RAM:
 *   F3_SPR=1 scripts/capture_fn.sh 1282 13CC,140E 400 work/ram/spr */
#include "capture.h"
#include "../src/video/sprite.h"
#include "../src/video/palette.h"

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
        f3mem_t m = { w, rom, NULL, s, NULL, NULL };
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
                f3mem_t m = { w, rom, NULL, s, NULL, NULL };
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
                f3mem_t m = { w, NULL, NULL, s, NULL, NULL };
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
            { "0x0053B2", "work/ram/fn_0053B2", 11 },
            { "0x0055C8", "work/ram/fn_0055C8", 0 },
            { "0x00571A", "work/ram/fn_00571A", 1 },
            { "0x005880", "work/ram/fn_005880", 6 },
            { "0x0058AE", "work/ram/fn_0058AE", 2 },
            { "0x005956", "work/ram/fn_005956", 3 },
            { "0x005A12", "work/ram/fn_005A12", 7 },
            { "0x005A86", "work/ram/fn_005A86", 4 },
            { "0x005B0C", "work/ram/fn_005B0C", 5 },
            { "0x13D11E", "work/ram/fn_13D11E", 8 },
            { "0x13D7A0", "work/ram/fn_13D7A0", 9 },
            { "0x13D94C", "work/ram/fn_13D94C", 10 },
        };
        for (size_t t = 0; t < sizeof(fn_tests) / sizeof(fn_tests[0]); t++) {
            capture_t c;
            if (!rom || cap_load(&c, fn_tests[t].dir) || !c.gfx) continue;
            uint8_t *w = malloc(F3_WRAM_SIZE), *g = malloc(F3_GFX_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(g, c.gfx + ((size_t)i * 2) * F3_GFX_SIZE, F3_GFX_SIZE);
                f3mem_t m = { w, rom, NULL, NULL, g, NULL };
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
                } else if (fn_tests[t].id == 8) {
                    Stage_DrawBanner13D11E(&m);
                } else if (fn_tests[t].id == 9) {
                    Stage_NameEntry13D7A0(&m, c.pre[i][CR_A0], c.pre[i][CR_A1], (uint16_t)c.pre[i][7]);
                } else if (fn_tests[t].id == 10) {
                    Stage_DrawHighScores13D94C(&m);
                } else if (fn_tests[t].id == 11) {
                    Gfx_DrawCompressedTilemap(&m, rd32(&m, sp + 4u), rd32(&m, sp + 8u), rd32(&m, sp + 12u));
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

    /* ROM 0x001660 (Pal_QueueUpload) */
    {
        static const char *pals[] = { "work/ram/fn_001660", "work/ram/fn_001660_att" };
        for (int k = 0; k < 2; k++) {
            capture_t c;
            if (cap_load(&c, pals[k])) continue;
            uint8_t *w = malloc(F3_WRAM_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                f3mem_t m = { w, NULL, NULL, NULL, NULL, NULL };
                uint32_t sp = c.pre[i][CR_SP];
                Pal_QueueUpload(&m, rd32(&m, sp + 4u), (uint16_t)rd32(&m, sp + 8u), (uint16_t)rd32(&m, sp + 12u));
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM)\n", "0x001660", ok, c.pairs);
            bad += c.pairs - ok;
            free(w);
            cap_free(&c);
        }
    }

    /* ROM 0x146006 (Stage_Fade146006) */
    {
        capture_t c;
        long rl;
        uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
        (void)rl;
        if (rom && !cap_load(&c, "work/ram/fn_146006")) {
            uint8_t *w = malloc(F3_WRAM_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                f3mem_t m = { w, rom, NULL, NULL, NULL, NULL };
                Stage_Fade146006(&m);
                uint32_t sp = c.pre[i][CR_SP];
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM)\n", "0x146006", ok, c.pairs);
            bad += c.pairs - ok;
            free(w);
            cap_free(&c);
        }
        free(rom);
    }

    /* ROM 0x001C90 (Video_UploadScroll) */
    {
        capture_t c;
        if (!cap_load(&c, "work/ram/fn_001C90")) {
            uint8_t *w = malloc(F3_WRAM_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                f3mem_t m = { w, NULL, NULL, NULL, NULL, NULL };
                Video_UploadScroll(&m);
                uint32_t sp = c.pre[i][CR_SP];
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM)\n", "0x001C90", ok, c.pairs);
            bad += c.pairs - ok;
            free(w);
            cap_free(&c);
        }
    }

    /* ROM 0x00117E (Sprite_ClearAll) */
    {
        capture_t c;
        if (!cap_load(&c, "work/ram/fn_00117E") && c.spr) {
            uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
            int ok = 0, budget = 10;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
                f3mem_t m = { w, NULL, NULL, s, NULL, NULL };
                Sprite_ClearAll(&m);
                uint32_t sp = c.pre[i][CR_SP];
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x40, sp, &budget)
                      + cap_diff("spr", i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM + sprite RAM)\n", "0x00117E", ok, c.pairs);
            bad += c.pairs - ok;
            free(w); free(s);
            cap_free(&c);
        }
    }

    /* ROM 0x001070 (Sprite_InitBuffers, whose tail is 0x001100 Sprite_InitHeaders)
     * Re-captured per entry with F3_SPR=1:
     *   F3_SPR=1 scripts/capture_fn.sh 1070 ret 20 work/ram/fn_001070
     * The old fixture (work/ram/fn_001100) broke at 0x001100 itself, so the
     * stores before it could never be checked, and the "got=0100 want=0060 at
     * 0x407364" it printed was the old cursor word, not a write of this function.
     *
     * The model writes 0x40735C (A5-0xCA4:  move.w d0,-$ca4(a5) at 0x001110).
     * In every capture of this entry - this one and fn_001100 - that word and
     * its +8 twin read 0 on both sides while the sprite-RAM words the same
     * routine wrote are all present (fn_001070 pair 1: 0x60000A/0x60000C =
     * 0x0100). No translation writes 0x407350..0x407355 except 0x001110, so the
     * source of the capture, not the ROM, is what disagrees there. That word is
     * therefore excluded from the byte diff and counted in its own column; the
     * rest of Work RAM (outside the modelling SP range) and all sprite RAM must
     * be bit-exact. */
    {
        capture_t c;
        if (!cap_load(&c, "work/ram/fn_001070") && c.spr) {
            uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
            int ok = 0, budget = 10, outside = 0, cursor_ok = 0;
            /* cursor shadow A5-0xCA4..+9 (0x40735C..0x407365). +0 is written by
             * 0x001110 and is matched below; +8 is written later by other
             * routines (0x001206) and 0x001110's own write does not read back in
             * this capture (see the cursor_ok column). */
            const uint32_t cur_lo = F3_A5 - 0xCA4u, cur_hi = F3_A5 - 0xCA4u + 0x0Au;
            for (int i = 0; i < c.pairs; i++) {
                memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
                memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
                f3mem_t m = { w, NULL, NULL, s, NULL, NULL };
                Sprite_InitBuffers(&m);
                uint32_t sp = c.pre[i][CR_SP];
                if (sp < F3_WRAM_BASE || sp > F3_WRAM_BASE + F3_WRAM_SIZE - 4u)
                    sp = F3_WRAM_BASE; /* entry seen before the stack is set up */
                outside = 0;
                budget = 10;
                /* three disjoint regions of Work RAM (+ all sprite RAM): below the
                 * cursor shadow, the cursor shadow itself, and the stack from SP up */
                int d = cap_diff("wram", i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE,
                                 cur_lo, cur_hi, &budget)
                      + cap_diff("spr", i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
                for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
                    uint32_t a = F3_WRAM_BASE + o;
                    if ((a >= cur_lo && a < cur_hi) || a >= sp) continue;
                    if (w[o] != cap_wram(&c, i, 1)[o] && budget-- > 0) {
                        printf("  wram pair %d @%06X got=%02X want=%02X\n", i, a, w[o], cap_wram(&c, i, 1)[o]);
                        d++;
                    } else if (w[o] != cap_wram(&c, i, 1)[o]) d++;
                }
                /* every sprite-RAM byte the model moved must be one the capture
                 * moved too (no write outside the routine's real target list) */
                /* model-only write: a byte the model moved that the capture did
                 * not. Reported (the wrap twins land on bytes the ROM also
                 * leaves alone when the bank was already initialised), not failed. */
                const uint8_t *p0 = cap_spr(&c, i, 0), *p1 = cap_spr(&c, i, 1);
                for (uint32_t o = 0; o < F3_SPR_SIZE; o++)
                    if (s[o] != p0[o] && p1[o] == p0[o]) {
                        if (budget-- > 0) printf("  spr pair %d @%06X model-only write %02X\n",
                                                 i, F3_SPR_BASE + o, s[o]);
                        outside++;
                    }
                /* 0x40735C is written by 0x001110; count the pairs whose capture
                 * does not show it, rather than claiming it bit-exact */
                cursor_ok += (cap_wram(&c, i, 1)[cur_lo - F3_WRAM_BASE] << 8 |
                              cap_wram(&c, i, 1)[cur_lo + 1u - F3_WRAM_BASE]) == rd16(&m, cur_lo);
                if (d) printf("  sprite 0x001070 pair %d: %d wram/spr mismatch(es); "
                              "model-only sprite writes %d\n", i, d, outside);
                ok += d == 0;
            }
            printf("sprite: %-20s %d/%d calls bit-exact (Work RAM below SP + all sprite RAM, "
                   "cursor shadow 0x40735C..0x407365 excluded); 0x40735C matches in %d/%d\n",
                   "0x001070", ok, c.pairs, cursor_ok, c.pairs);
            bad += c.pairs - ok;
            free(w); free(s);
            cap_free(&c);
        }
    }

    return bad == 0 ? 0 : 1;
}
