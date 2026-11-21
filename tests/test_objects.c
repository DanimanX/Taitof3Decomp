/* Differential tests for individual object tick routines, captured at the
 * pool call site with a conditional breakpoint, e.g.
 *   F3_SPR=1 F3_COND="a1==102fe6" scripts/capture_fn.sh 104EE 104F0 400 work/ram/obj_102FE6
 * Compares all Work RAM (except the stack window) and sprite RAM. Pairs where
 * the C reaches a not-yet-decompiled path (returns -1) are counted apart. */
#include "capture.h"
#include "../src/enemy/enemy_bullet.h"
#include "../src/player/player_shot.h"
#include "../src/enemy/obj_D9116.h"
#include "../src/enemy/obj_CABF4.h"
#include "../src/objects/objscript_CF1E4.h"
#include "../src/objects/objscript_CF4B8.h"

typedef int (*tick_fn)(const f3mem_t *, uint32_t);
typedef int (*tick_a1_fn)(const f3mem_t *, uint32_t, uint32_t);

static int run_any(const char *name, const char *pfx, tick_fn fn, tick_a1_fn fn_a1, const uint8_t *rom)
{
    capture_t c;
    if (cap_load(&c, pfx) || !c.spr) { printf("objects: %-22s SKIP (no %s)\n", name, pfx); return 0; }
    uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE);
    int ok = 0, unimpl = 0, budget = 8, died = 0, freed = 0;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        f3mem_t m = { w, (uint8_t *)rom, NULL, s };
        uint32_t obj = c.pre[i][CR_A0], fn0 = rd32(&m, obj);
        int r = fn ? fn(&m, obj) : fn_a1(&m, obj, c.pre[i][CR_A1]);
        if (r < 0) { unimpl++; continue; }
        if (rd32(&m, obj) != fn0) died++;   /* switched state (dying) */
        if (memcmp(w + 0x10000 - 0, cap_wram(&c, i, 0) + 0x10000, 0x30)) freed++; /* free masks changed */
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff(name, i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x80, sp, &budget)
              + cap_diff(name, i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
        ok += d == 0;
    }
    int tested = c.pairs - unimpl;
    printf("objects: %-22s %3d/%3d ticks bit-exact (%d state changes, %d alloc/free; %d reach undecompiled paths)\n",
           name, ok, tested, died, freed, unimpl);
    free(w); free(s); cap_free(&c);
    return tested - ok;
}

static int run(const char *name, const char *pfx, tick_fn fn, const uint8_t *rom)
{ return run_any(name, pfx, fn, NULL, rom); }
static int run_a1(const char *name, const char *pfx, tick_a1_fn fn, const uint8_t *rom)
{ return run_any(name, pfx, NULL, fn, rom); }

int main(void)
{
    long rl;
    uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
    (void)rl;
    if (!rom) { printf("objects: SKIP (need work/maincpu.bin)\n"); return 0; }
    int bad = 0;
    bad += run("EnemyBullet 0x102FE6", "work/ram/obj_102FE6", Obj_EnemyBullet_Tick, rom);
    bad += run("PlayerShot 0x0C9C6A", "work/ram/obj_C9C6A", Obj_PlayerShot_Tick, rom);
    bad += run("Obj 0x0D9116", "work/ram/obj_D9116", Obj_D9116_Tick, rom);
    bad += run("Obj 0x0CABF4", "work/ram/obj_CABF4", Obj_CABF4_Tick, rom);
    /* captured with F3_EXIT=ret at the routine entry */
    bad += run("ObjScript 0x0CF1E4", "work/ram/vm_CF1E4", ObjScript_CF1E4, rom);
    bad += run_a1("ObjMove 0x0CF4B8", "work/ram/vm_CF4B8", ObjScript_CF4B8, rom);
    /* every script opcode forced through a synthetic script at 0x41E000:
       F3_POKE32=41E000=<op>0010,41E004=00100000,41E008=0 F3_POKEA0B=44=00,45=41,46=E0,47=00
       (ops 17/24/25 jump back by their parameter: captured with parameter 0) */
    for (int op = 0; op < 29; op++) {
        char pfx[64], name[32];
        snprintf(pfx, sizeof pfx, "work/ram/vmop_CF1E4_%02d", op);
        snprintf(name, sizeof name, "ObjScript_CF1E4 op %2d", op);
        bad += run(name, pfx, ObjScript_CF1E4, rom);
    }
    /* every movement mode forced: F3_POKEA0B=20=<mode> (CF4B8 dispatches on obj+0x20) */
    for (int op = 0; op < 29; op++) {
        char pfx[64], name[32];
        snprintf(pfx, sizeof pfx, "work/ram/vmop_CF4B8_%02d", op);
        snprintf(name, sizeof name, "ObjMove_CF4B8 mode %2d", op);
        bad += run_a1(name, pfx, ObjScript_CF4B8, rom);
    }
    free(rom);
    return bad ? 1 : 0;
}
