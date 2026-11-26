/* Generic runner for function captures (scripts/capture_fn.sh <addr> ret ...
 * -> work/ram/fn_<ADDR>_{pairs,regs,gfx,spr}): one entry->return call per
 * pair. For each pair the pre Work RAM / screen memory (0x610000-0x62FFFF) /
 * sprite RAM images are loaded, the C translation runs from the captured
 * pre registers through an adapter, and the post images are compared byte
 * for byte, skipping the stack window [SP-0x100, SP) below the entry SP
 * (locals, saved registers and the callee's frames live there). Selected
 * output registers are compared under a mask when the table entry asks.
 *
 * Adapter contract: int fn(const f3mem_t *m, f3regs_t *r). r holds the pre
 * registers (D0-D7, A0-A6, A7 = SP at entry, so stack arguments are at
 * SP+4...); the adapter writes the registers the routine returns. A return
 * < 0 means the call reached a path that is not decompiled yet: that pair is
 * counted as "undecompiled", not as a mismatch, and fails the routine only if
 * no pair was observable (as test_objects does with REJECT).
 *
 * Env: F3_ONLY=<substring of the entry name> runs only the matching entries.
 * A missing capture is a SKIP. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capture.h"
#include "../src/enemy/obj_1194C2.h"      /* Text_DrawString (0x005666) */
#include "../src/player/player_task.h"    /* Player_CoinCheckA (0x0C77BE) */
#include "../src/enemy/obj_CBF04.h"       /* Obj_CBEEA (0x0CBEEA) */

typedef struct { uint32_t d[8], a[8]; } f3regs_t;   /* a[7] = SP */

typedef int (*fn_adapter_t)(const f3mem_t *m, f3regs_t *r);

typedef struct { int reg; uint32_t mask; } reg_check_t;   /* reg: CR_* index */

typedef struct {
    const char *name;
    const char *cap;
    fn_adapter_t fn;
    reg_check_t out[4];   /* mask 0 ends the list */
} fn_entry_t;

/* ---- adapters ---------------------------------------------------------- */

/* ROM 0x005666: (8,A6)=src.l (0xC,A6)=dst.l (0x10,A6)=col.w; D0 = end dst */
static int ad_005666(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    r->d[0] = Text_DrawString(m, rd32(m, sp + 4u), rd32(m, sp + 8u), rd16(m, sp + 12u));
    return 0;
}

/* ROM 0x0C77BE: A0 = player state; D0.b = result (upper bytes not modelled) */
static int ad_0C77BE(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | Player_CoinCheckA(m, r->a[0]);
    return 0;
}

/* ROM 0x0CBEEA: jmp target, A0 = obj, D1.w = child count, A1 = child fn */
static int ad_0CBEEA(const f3mem_t *m, f3regs_t *r)
{
    return Obj_CBEEA(m, r->a[0], (uint16_t)r->d[1], r->a[1]);
}

static const fn_entry_t fns[] = {
    { "Fn 0x005666 Text_DrawString",   "work/ram/fn_005666", ad_005666, { { CR_D0, 0xFFFFFFFFu } } },
    { "Fn 0x0C77BE Player_CoinCheckA", "work/ram/fn_0C77BE", ad_0C77BE, { { CR_D0, 0x000000FFu } } },
    { "Fn 0x0CBEEA Obj_CBEEA",         "work/ram/fn_0CBEEA", ad_0CBEEA, { { 0, 0u } } },
};

/* ---- runner ------------------------------------------------------------ */

static uint8_t *slurp(const char *path, long *len) { return cap_slurp(path, len); }

static int run(const fn_entry_t *e, const uint8_t *rom)
{
    capture_t c;
    if (cap_load(&c, e->cap) || c.pairs == 0) {
        printf("fn: %-34s SKIP (no %s)\n", e->name, e->cap);
        if (c.wram) cap_free(&c);
        return 0;
    }
    uint8_t *w = malloc(F3_WRAM_SIZE);
    uint8_t *g = c.gfx ? malloc(F3_GFX_SIZE) : NULL;
    uint8_t *s = c.spr ? malloc(F3_SPR_SIZE) : NULL;
    int ok = 0, undec = 0, budget = 12;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        if (g) memcpy(g, cap_gfx(&c, i, 0), F3_GFX_SIZE);
        if (s) memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        f3mem_t m = { w, rom, NULL, s, g };
        f3regs_t r;
        for (int k = 0; k < 8; k++) { r.d[k] = c.pre[i][CR_D0 + k]; r.a[k] = c.pre[i][CR_A0 + k]; }
        if (e->fn(&m, &r) < 0) { undec++; continue; }
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff(e->name, i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x100u, sp, &budget);
        if (g) d += cap_diff(e->name, i, F3_GFX_BASE, g, cap_gfx(&c, i, 1), F3_GFX_SIZE, 0, 0, &budget);
        if (s) d += cap_diff(e->name, i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
        for (int k = 0; k < 4 && e->out[k].mask; k++) {
            int x = e->out[k].reg;
            uint32_t got = (x < 8 ? r.d[x] : r.a[x - 8]) & e->out[k].mask;
            uint32_t want = c.post[i][x] & e->out[k].mask;
            if (got != want) {
                if (budget-- > 0)
                    printf("  %s pair %d %c%d got=%08X want=%08X\n", e->name, i,
                           x < 8 ? 'D' : 'A', x & 7, got, want);
                d++;
            }
        }
        if (d == 0) ok++;
    }
    int obs = c.pairs - undec;
    printf("fn: %-34s %3d/%3d calls bit-exact (%d undecompiled, gfx %s, spr %s)%s\n",
           e->name, ok, obs, undec, g ? "yes" : "no", s ? "yes" : "no",
           obs == 0 ? " REJECT no observable call" : "");
    free(w); free(g); free(s); cap_free(&c);
    return (obs - ok) + (obs == 0 ? 1 : 0);
}

int main(void)
{
    long rl = 0;
    uint8_t *rom = slurp("work/maincpu.bin", &rl);
    if (!rom || rl < (long)F3_ROM_SIZE) { printf("fn: SKIP (need work/maincpu.bin)\n"); free(rom); return 0; }
    const char *only = getenv("F3_ONLY");
    int bad = 0;
    for (size_t k = 0; k < sizeof fns / sizeof fns[0]; k++) {
        if (only && !strstr(fns[k].name, only)) continue;
        bad += run(&fns[k], rom);
    }
    printf("fn: %s (%d failing calls)\n", bad ? "FAIL" : "PASS", bad);
    free(rom);
    return bad ? 1 : 0;
}
