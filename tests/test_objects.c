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

static int run(const char *name, const char *pfx, tick_fn fn, const uint8_t *rom)
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
        if (fn(&m, obj) < 0) { unimpl++; continue; }
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
    bad += run("ObjScript 0x0CF4B8", "work/ram/vm_CF4B8", ObjScript_CF4B8, rom);
    free(rom);
    return bad ? 1 : 0;
}
