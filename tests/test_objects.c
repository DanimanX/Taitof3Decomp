#include <string.h>
#include <stdlib.h>
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
#include "../src/enemy/obj_EAD28.h"
#include "../src/enemy/obj_1027B4.h"
#include "../src/enemy/obj_102860.h"
#include "../src/enemy/obj_102928.h"
#include "../src/enemy/obj_C9F2E.h"
#include "../src/enemy/obj_10252E.h"
#include "../src/enemy/obj_102A9A.h"
#include "../src/enemy/obj_119990.h"
#include "../src/enemy/obj_CBF5E.h"
#include "../src/enemy/obj_119CBA.h"
#include "../src/enemy/obj_117D06.h"
#include "../src/enemy/obj_D5464.h"
#include "../src/enemy/obj_13E5B2.h"
#include "../src/enemy/obj_CAB36.h"
#include "../src/objects/obj_common.h"
#include "../src/enemy/obj_119714.h"
#include "../src/enemy/obj_CBF8A.h"
#include "../src/enemy/obj_DCDF2.h"
#include "../src/enemy/obj_DC8BE.h"
#include "../src/enemy/obj_F30F6.h"
#include "../src/enemy/obj_E04CC.h"
#include "../src/enemy/obj_DECF8.h"
#include "../src/enemy/obj_EACDE.h"
#include "../src/enemy/obj_E0278.h"
#include "../src/enemy/obj_117BDE.h"
#include "../src/enemy/obj_F302E.h"
#include "../src/enemy/obj_DC3F6.h"
#include "../src/enemy/obj_C6866.h"
#include "../src/enemy/obj_C9978.h"
#include "../src/enemy/obj_C9D7E.h"
#include "../src/enemy/obj_DC542.h"
#include "../src/enemy/obj_E02C4.h"
#include "../src/enemy/obj_CBF6C.h"
#include "../src/enemy/obj_D540A.h"
#include "../src/enemy/obj_D901A.h"
#include "../src/enemy/obj_CC0F4.h"
#include "../src/enemy/obj_D9092.h"
#include "../src/enemy/obj_DC456.h"
#include "../src/enemy/obj_DC586.h"
#include "../src/enemy/obj_DEC9E.h"
#include "../src/enemy/obj_E029E.h"
#include "../src/enemy/obj_E0266.h"
#include "../src/enemy/obj_F30B4.h"
#include "../src/enemy/obj_102F52.h"
#include "../src/enemy/obj_117B44.h"
#include "../src/enemy/obj_117C28.h"
#include "../src/enemy/obj_13E4EE.h"
#include "../src/enemy/obj_DC328.h"
#include "../src/enemy/obj_DC0F8.h"
#include "../src/enemy/obj_DF7EA.h"
#include "../src/enemy/obj_DF844.h"
#include "../src/enemy/obj_CBC88.h"
#include "../src/enemy/obj_CBC90.h"
#include "../src/enemy/obj_BB83E.h"
#include "../src/enemy/obj_BC3EC.h"
#include "../src/enemy/obj_BCA00.h"
#include "../src/enemy/obj_BCD82.h"
#include "../src/enemy/obj_CBC98.h"
#include "../src/enemy/obj_11C39A.h"
#include "../src/enemy/obj_BC07E.h"
#include "../src/enemy/obj_BD544.h"
#include "../src/enemy/obj_C685A.h"
#include "../src/enemy/obj_CB406.h"
#include "../src/enemy/obj_CB0DA.h"
#include "../src/enemy/obj_BC152.h"
#include "../src/enemy/obj_BC1CE.h"
#include "../src/enemy/obj_BC3DE.h"
#include "../src/enemy/obj_BC064.h"
#include "../src/enemy/obj_BC638.h"
#include "../src/enemy/obj_BD298.h"
#include "../src/enemy/obj_CB412.h"
#include "../src/enemy/obj_12EF24.h"
#include "../src/enemy/obj_BBD4A.h"
#include "../src/enemy/obj_BBE06.h"
#include "../src/enemy/obj_BC4A8.h"
#include "../src/enemy/obj_BD256.h"
#include "../src/enemy/obj_BBBE0.h"
#include "../src/enemy/obj_BC008.h"
#include "../src/enemy/obj_BD340.h"
#include "../src/enemy/obj_D7570.h"
#include "../src/enemy/obj_BBFE8.h"
#include "../src/enemy/obj_BCE8E.h"
#include "../src/enemy/obj_BD354.h"
#include "../src/enemy/obj_12E1FA.h"
#include "../src/enemy/obj_BCF08.h"
#include "../src/enemy/obj_BCE38.h"
#include "../src/enemy/obj_12F26A.h"
#include "../src/enemy/obj_12F27A.h"
#include "../src/enemy/obj_BBD24.h"
#include "../src/enemy/obj_BBDE0.h"
#include "../src/enemy/obj_BBE9A.h"
#include "../src/enemy/obj_CABAC.h"
#include "../src/enemy/obj_BB746.h"
#include "../src/enemy/obj_BC03C.h"
#include "../src/enemy/obj_CB2C6.h"
#include "../src/enemy/obj_BCD90.h"
#include "../src/enemy/obj_BBCDA.h"
#include "../src/enemy/obj_BBD6E.h"
#include "../src/enemy/obj_BBE2A.h"
#include "../src/enemy/obj_11C1DE.h"
#include "../src/enemy/obj_12EA9E.h"
#include "../src/enemy/obj_BC540.h"
#include "../src/enemy/obj_11A87E.h"
#include "../src/enemy/obj_12E21A.h"
#include "../src/enemy/obj_CA28C.h"
#include "../src/enemy/obj_CAF3E.h"
#include "../src/enemy/obj_BC3FA.h"
#include "../src/enemy/obj_BC404.h"
#include "../src/enemy/obj_BC40E.h"
#include "../src/enemy/obj_BC418.h"
#include "../src/enemy/obj_BC422.h"
#include "../src/enemy/obj_BC24A.h"
#include "../src/enemy/obj_CAF1A.h"
#include "../src/enemy/obj_BB692.h"
#include "../src/enemy/obj_BCEB0.h"
#include "../src/enemy/obj_12E1BE.h"
#include "../src/enemy/obj_BBD00.h"
#include "../src/enemy/obj_BCF30.h"
#include "../src/enemy/obj_BE262.h"
#include "../src/enemy/obj_BB84C.h"
#include "../src/enemy/obj_BB856.h"
#include "../src/enemy/obj_BB860.h"
#include "../src/enemy/obj_BB86A.h"
#include "../src/enemy/obj_BB874.h"
#include "../src/enemy/obj_BCDC0.h"
#include "../src/enemy/obj_CAF5E.h"
#include "../src/enemy/obj_12EDA8.h"
#include "../src/enemy/obj_12E19A.h"
#include "../src/enemy/obj_BC680.h"
#include "../src/enemy/obj_BCFF4.h"
#include "../src/enemy/obj_12EB0E.h"
#include "../src/enemy/obj_BB5C2.h"
#include "../src/enemy/obj_BBE50.h"
#include "../src/enemy/obj_BCE52.h"
#include "../src/enemy/obj_DBDA2.h"
#include "../src/enemy/obj_ECF3E.h"
#include "../src/enemy/obj_BB952.h"
#include "../src/enemy/obj_BBD94.h"
#include "../src/enemy/obj_BBEC0.h"
#include "../src/enemy/obj_DBDE8.h"
#include "../src/enemy/obj_DC674.h"
#include "../src/enemy/obj_105268.h"
#include "../src/enemy/obj_BBB18.h"
#include "../src/enemy/obj_CAEBA.h"
#include "../src/enemy/obj_EA28C.h"
#include "../src/enemy/obj_EA836.h"
#include "../src/enemy/obj_10524A.h"
#include "../src/enemy/obj_1095B0.h"
#include "../src/enemy/obj_F34F6.h"
#include "../src/enemy/obj_1116E2.h"
#include "../src/enemy/obj_12EA24.h"
#include "../src/enemy/obj_BD990.h"
#include "../src/enemy/obj_12EAD4.h"
#include "../src/enemy/obj_12CE5A.h"
#include "../src/enemy/obj_CB2EC.h"
#include "../src/enemy/obj_10290C.h"
#include "../src/enemy/obj_E186A.h"
#include "../src/enemy/obj_102798.h"
#include "../src/enemy/obj_102844.h"
#include "../src/enemy/obj_102A70.h"
#include "../src/enemy/obj_BBAE4.h"
#include "../src/enemy/obj_CBC78.h"
#include "../src/enemy/obj_DA0D6.h"
#include "../src/enemy/obj_ECA04.h"
#include "../src/enemy/obj_EA418.h"
#include "../src/enemy/obj_ECF0C.h"
#include "../src/enemy/obj_ECFAE.h"
#include "../src/enemy/obj_BBBF4.h"
#include "../src/enemy/obj_BC576.h"
#include "../src/enemy/obj_BDE42.h"
#include "../src/enemy/obj_EA29E.h"
#include "../src/enemy/obj_104EFA.h"
#include "../src/enemy/obj_C9B3E.h"
#include "../src/enemy/obj_CBA34.h"
#include "../src/enemy/obj_ECFDA.h"
#include "../src/enemy/obj_BD308.h"
#include "../src/enemy/obj_CB054.h"
#include "../src/enemy/obj_CB422.h"
#include "../src/enemy/obj_CAF90.h"
#include "../src/enemy/obj_F3582.h"
#include "../src/enemy/obj_BC2A0.h"
#include "../src/enemy/obj_10963E.h"
#include "../src/enemy/obj_EACEA.h"
#include "../src/enemy/obj_BCC96.h"
#include "../src/enemy/obj_BD2C2.h"
#include "../src/enemy/obj_BDDB8.h"
#include "../src/enemy/obj_11C1FC.h"
#include "../src/enemy/obj_ECAC8.h"
#include "../src/enemy/obj_CB9EE.h"
#include "../src/enemy/obj_11C3A8.h"
#include "../src/enemy/obj_BD700.h"
#include "../src/enemy/obj_CB97E.h"
#include "../src/enemy/obj_E1B36.h"
#include "../src/enemy/obj_C9DEC.h"
#include "../src/enemy/obj_CAFB8.h"
#include "../src/enemy/obj_12CFD4.h"
#include "../src/enemy/obj_12F29E.h"
#include "../src/enemy/obj_BD0EC.h"
#include "../src/enemy/obj_12EF40.h"
#include "../src/enemy/obj_BC16C.h"
#include "../src/enemy/obj_BC1E8.h"
#include "../src/enemy/obj_11A8A8.h"
#include "../src/enemy/obj_BDDE2.h"
#include "../src/enemy/obj_BDC5C.h"
#include "../src/enemy/obj_BC08C.h"
#include "../src/enemy/obj_BD3A4.h"
#include "../src/enemy/obj_BD550.h"
#include "../src/enemy/obj_BD7E4.h"
#include "../src/enemy/obj_BE16C.h"
#include "../src/enemy/obj_13EFA0.h"
#include "../src/enemy/obj_104F26.h"
#include "../src/enemy/obj_BD9BE.h"
#include "../src/enemy/obj_BC70A.h"
#include "../src/enemy/obj_D9FC8.h"
#include "../src/enemy/obj_12EE24.h"
#include "../src/enemy/obj_DA138.h"
#include "../src/enemy/obj_12EB90.h"
#include "../src/enemy/obj_D7306.h"
#include "../src/enemy/obj_BDE92.h"
#include "../src/enemy/obj_E9DE0.h"
#include "../src/enemy/obj_E96C8.h"
#include "../src/enemy/obj_E1886.h"
#include "../src/enemy/obj_EA848.h"
#include "../src/enemy/obj_DB074.h"
#include "../src/enemy/obj_10967C.h"
#include "../src/enemy/obj_F35B4.h"
#include "../src/enemy/obj_DC6D0.h"
#include "../src/enemy/obj_E1328.h"
#include "../src/enemy/obj_EA45E.h"
#include "../src/enemy/obj_1049C4.h"
#include "../src/enemy/obj_ECB3A.h"
#include "../src/enemy/obj_D740E.h"
#include "../src/enemy/obj_DC5B6.h"
#include "../src/enemy/obj_D747E.h"
#include "../src/enemy/obj_CD646.h"
#include "../src/enemy/obj_CD504.h"
#include "../src/enemy/obj_CD5EA.h"
#include "../src/enemy/obj_CD556.h"
#include "../src/enemy/obj_1180F8.h"
#include "../src/enemy/obj_E077A.h"
#include "../src/enemy/obj_E1036.h"
#include "../src/enemy/obj_112D1A.h"
#include "../src/enemy/obj_E101A.h"
#include "../src/enemy/obj_1134B2.h"
#include "../src/enemy/obj_113ED0.h"
#include "../src/enemy/obj_112C1A.h"
#include "../src/enemy/obj_1140BE.h"
#include "../src/enemy/obj_112C3C.h"
#include "../src/enemy/obj_118CF6.h"
#include "../src/enemy/obj_112D88.h"
#include "../src/enemy/obj_114108.h"
#include "../src/enemy/obj_1134BA.h"
#include "../src/enemy/obj_118DDC.h"
#include "../src/enemy/obj_118D7C.h"
#include "../src/enemy/obj_F0788.h"
#include "../src/enemy/obj_F0846.h"
#include "../src/enemy/obj_108936.h"
#include "../src/enemy/obj_1087E8.h"
#include "../src/enemy/obj_DC486.h"
#include "../src/enemy/obj_E598C.h"
#include "../src/enemy/obj_F0814.h"
#include "../src/enemy/obj_1088F0.h"
#include "../src/enemy/obj_E6166.h"
#include "../src/enemy/obj_E61AA.h"
#include "../src/enemy/obj_E5CBC.h"
#include "../src/enemy/obj_DC358.h"
#include "../src/enemy/obj_E60EA.h"
#include "../src/enemy/obj_E5C2A.h"
#include "../src/enemy/obj_F5AFE.h"
#include "../src/enemy/obj_E6082.h"
#include "../src/enemy/obj_F5D58.h"
#include "../src/enemy/obj_F5CF6.h"
#include "../src/objects/objscript_CF1E4.h"
#include "../src/objects/objscript_CF4B8.h"

typedef int (*tick_fn)(const f3mem_t *, uint32_t);
typedef int (*tick_a1_fn)(const f3mem_t *, uint32_t, uint32_t);

static int run_any(const char *name, const char *pfx, tick_fn fn, tick_a1_fn fn_a1, const uint8_t *rom)
{
    capture_t c;
    if (cap_load(&c, pfx) || !c.spr) { printf("objects: %-22s SKIP (no %s)\n", name, pfx); return 0; }
    uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE), *g = c.gfx ? malloc(F3_GFX_SIZE) : NULL;
    int ok = 0, unimpl = 0, budget = 8, died = 0, freed = 0;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        if (g) memcpy(g, cap_gfx(&c, i, 0), F3_GFX_SIZE);
        f3mem_t m = { w, (uint8_t *)rom, NULL, s, g };
        uint32_t obj = c.pre[i][CR_A0], fn0 = rd32(&m, obj);
        int r = fn ? fn(&m, obj) : fn_a1(&m, obj, c.pre[i][CR_A1]);
        if (r < 0) { unimpl++; continue; }
        if (rd32(&m, obj) != fn0) died++;   /* switched state (dying) */
        if (memcmp(w + 0x10000 - 0, cap_wram(&c, i, 0) + 0x10000, 0x30)) freed++; /* free masks changed */
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff(name, i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x80, sp, &budget)
              + cap_diff(name, i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget)
              + (g ? cap_diff(name, i, F3_GFX_BASE, g, cap_gfx(&c, i, 1), F3_GFX_SIZE, 0, 0, &budget) : 0);
        ok += d == 0;
    }
    int tested = c.pairs - unimpl;
    printf("objects: %-22s %3d/%3d ticks bit-exact (%d state changes, %d alloc/free; %d reach undecompiled paths)\n",
           name, ok, tested, died, freed, unimpl);
    free(w); free(s); free(g); cap_free(&c);
    return tested - ok;
}

/* F3_ONLY=<substring>: run only the tests whose name contains it */
static int skip(const char *name)
{ const char *o = getenv("F3_ONLY"); return o && *o && !strstr(name, o); }
static int run(const char *name, const char *pfx, tick_fn fn, const uint8_t *rom)
{ return skip(name) ? 0 : run_any(name, pfx, fn, NULL, rom); }
static int run_a1(const char *name, const char *pfx, tick_a1_fn fn, const uint8_t *rom)
{ return skip(name) ? 0 : run_any(name, pfx, NULL, fn, rom); }

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
    bad += run("Obj 0x0EAD28", "work/ram/obj_EAD28", Obj_EAD28_Tick, rom);
    bad += run("Obj 0x1027B4", "work/ram/obj_1027B4", Obj_1027B4_Tick, rom);
    bad += run("Obj 0x102860", "work/ram/obj_102860", Obj_102860_Tick, rom);
    bad += run("Obj 0x102928", "work/ram/obj_102928", Obj_102928_Tick, rom);
    bad += run("Obj 0x0C9F2E", "work/ram/obj_C9F2E", Obj_C9F2E_Tick, rom);
    bad += run("Obj 0x10252E", "work/ram/obj_10252E", Obj_10252E_Tick, rom);
    bad += run("Obj 0x102A9A", "work/ram/obj_102A9A", Obj_102A9A_Tick, rom);
    bad += run("Obj 0x119990", "work/ram/obj_119990", Obj_119990_Tick, rom);
    bad += run("Obj 0x0CBF5E", "work/ram/obj_CBF5E", Obj_CBF5E_Tick, rom);
    bad += run("Obj 0x119CBA", "work/ram/obj_119CBA", Obj_119CBA_Tick, rom);
    bad += run("Obj 0x117D06", "work/ram/obj_117D06", Obj_117D06_Tick, rom);
    bad += run("Obj 0x0D5464", "work/ram/obj_D5464", Obj_D5464_Tick, rom);
    bad += run("Obj 0x13E5B2", "work/ram/obj_13E5B2", Obj_13E5B2_Tick, rom);
    bad += run("Obj 0x0CAB36", "work/ram/obj_CAB36", Obj_CAB36_Tick, rom);
    bad += run("Obj_Dying1 0x0CEE7C", "work/ram/obj_CEE7C", Obj_Dying1_Tick, rom);
    bad += run("Obj_Dying2 0x0CEE84", "work/ram/obj_CEE84", Obj_Dying2_Tick, rom);
    bad += run("Obj_Dying3 0x0CEE8C", "work/ram/obj_CEE8C", Obj_Dying3_Tick, rom);
    bad += run("Obj 0x119714", "work/ram/obj_119714", Obj_119714_Tick, rom);
    bad += run("Obj 0x0CBF8A", "work/ram/obj_CBF8A", Obj_CBF8A_Tick, rom);
    bad += run("Obj 0x0DCDF2", "work/ram/obj_DCDF2", Obj_DCDF2_Tick, rom);
    bad += run("Obj 0x0DC8BE", "work/ram/obj_DC8BE", Obj_DC8BE_Tick, rom);
    bad += run("Obj 0x0F30F6", "work/ram/obj_F30F6", Obj_F30F6_Tick, rom);
    bad += run("Obj 0x0E04CC", "work/ram/obj_E04CC", Obj_E04CC_Tick, rom);
    bad += run("Obj 0x0DECF8", "work/ram/obj_DECF8", Obj_DECF8_Tick, rom);
    bad += run("Obj 0x0EACDE", "work/ram/obj_EACDE", Obj_EACDE_Tick, rom);
    bad += run("Obj 0x0E0278", "work/ram/obj_E0278", Obj_E0278_Tick, rom);
    bad += run("Obj 0x117BDE", "work/ram/obj_117BDE", Obj_117BDE_Tick, rom);
    bad += run("Obj 0x0F302E", "work/ram/obj_F302E", Obj_F302E_Tick, rom);
    bad += run("Obj 0x0DC3F6", "work/ram/obj_DC3F6", Obj_DC3F6_Tick, rom);
    bad += run("Obj 0x0C6866", "work/ram/obj_C6866", Obj_C6866_Tick, rom);
    bad += run("Obj 0x0C9978", "work/ram/obj_C9978", Obj_C9978_Tick, rom);
    bad += run("Obj 0x0C9D7E", "work/ram/obj_C9D7E", Obj_C9D7E_Tick, rom);
    bad += run("Obj 0x0DC542", "work/ram/obj_DC542", Obj_DC542_Tick, rom);
    bad += run("Obj 0x0E02C4", "work/ram/obj_E02C4", Obj_E02C4_Tick, rom);
    bad += run("Obj 0x0CBF6C", "work/ram/obj_CBF6C", Obj_CBF6C_Tick, rom);
    bad += run("Obj 0x0D540A", "work/ram/obj_D540A", Obj_D540A_Tick, rom);
    bad += run("Obj 0x0D901A", "work/ram/obj_D901A", Obj_D901A_Tick, rom);
    bad += run("Obj 0x0CC0F4", "work/ram/obj_CC0F4", Obj_CC0F4_Tick, rom);
    bad += run("Obj 0x0D9092", "work/ram/obj_D9092", Obj_D9092_Tick, rom);
    bad += run("Obj 0x0DC456", "work/ram/obj_DC456", Obj_DC456_Tick, rom);
    bad += run("Obj 0x0DC586", "work/ram/obj_DC586", Obj_DC586_Tick, rom);
    bad += run("Obj 0x0DEC9E", "work/ram/obj_DEC9E", Obj_DEC9E_Tick, rom);
    bad += run("Obj 0x0E029E", "work/ram/obj_E029E", Obj_E029E_Tick, rom);
    bad += run("Obj 0x0E0266", "work/ram/obj_E0266", Obj_E0266_Tick, rom);
    bad += run("Obj 0x0F30B4", "work/ram/obj_F30B4", Obj_F30B4_Tick, rom);
    bad += run("Obj 0x102F52", "work/ram/obj_102F52", Obj_102F52_Tick, rom);
    bad += run("Obj 0x117B44", "work/ram/obj_117B44", Obj_117B44_Tick, rom);
    bad += run("Obj 0x117C28", "work/ram/obj_117C28", Obj_117C28_Tick, rom);
    bad += run("Obj 0x13E4EE", "work/ram/obj_13E4EE", Obj_13E4EE_Tick, rom);
    bad += run("Obj 0x0DC328", "work/ram/obj_DC328", Obj_DC328_Tick, rom);
    bad += run("Obj 0x0DC0F8", "work/ram/obj_DC0F8", Obj_DC0F8_Tick, rom);
    bad += run("Obj 0x0DF7EA", "work/ram/obj_DF7EA", Obj_DF7EA_Tick, rom);
    bad += run("Obj 0x0DF844", "work/ram/obj_DF844", Obj_DF844_Tick, rom);
    bad += run("Obj 0x0CBC88", "work/ram/obj_CBC88", Obj_CBC88_Tick, rom);
    bad += run("Obj 0x0CBC90", "work/ram/obj_CBC90", Obj_CBC90_Tick, rom);
    bad += run("Obj 0x0BB83E", "work/ram/obj_BB83E", Obj_BB83E_Tick, rom);
    bad += run("Obj 0x0BC3EC", "work/ram/obj_BC3EC", Obj_BC3EC_Tick, rom);
    bad += run("Obj 0x0BCA00", "work/ram/obj_BCA00", Obj_BCA00_Tick, rom);
    bad += run("Obj 0x0BCD82", "work/ram/obj_BCD82", Obj_BCD82_Tick, rom);
    bad += run("Obj 0x0CBC98", "work/ram/obj_CBC98", Obj_CBC98_Tick, rom);
    bad += run("Obj 0x11C39A", "work/ram/obj_11C39A", Obj_11C39A_Tick, rom);
    bad += run("Obj 0x0BC07E", "work/ram/obj_BC07E", Obj_BC07E_Tick, rom);
    bad += run("Obj 0x0BD544", "work/ram/obj_BD544", Obj_BD544_Tick, rom);
    bad += run("Obj 0x0C685A", "work/ram/obj_C685A", Obj_C685A_Tick, rom);
    bad += run("Obj 0x0CB406", "work/ram/obj_CB406", Obj_CB406_Tick, rom);
    bad += run("Obj 0x0CB0DA", "work/ram/obj_CB0DA", Obj_CB0DA_Tick, rom);
    bad += run("Obj 0x0BC152", "work/ram/obj_BC152", Obj_BC152_Tick, rom);
    bad += run("Obj 0x0BC1CE", "work/ram/obj_BC1CE", Obj_BC1CE_Tick, rom);
    bad += run("Obj 0x0BC3DE", "work/ram/obj_BC3DE", Obj_BC3DE_Tick, rom);
    bad += run("Obj 0x0BC064", "work/ram/obj_BC064", Obj_BC064_Tick, rom);
    bad += run("Obj 0x0BC638", "work/ram/obj_BC638", Obj_BC638_Tick, rom);
    bad += run("Obj 0x0BD298", "work/ram/obj_BD298", Obj_BD298_Tick, rom);
    bad += run("Obj 0x0CB412", "work/ram/obj_CB412", Obj_CB412_Tick, rom);
    bad += run("Obj 0x12EF24", "work/ram/obj_12EF24", Obj_12EF24_Tick, rom);
    bad += run("Obj 0x0BBD4A", "work/ram/obj_BBD4A", Obj_BBD4A_Tick, rom);
    bad += run("Obj 0x0BBE06", "work/ram/obj_BBE06", Obj_BBE06_Tick, rom);
    bad += run("Obj 0x0BC4A8", "work/ram/obj_BC4A8", Obj_BC4A8_Tick, rom);
    bad += run("Obj 0x0BD256", "work/ram/obj_BD256", Obj_BD256_Tick, rom);
    bad += run("Obj 0x0BBBE0", "work/ram/obj_BBBE0", Obj_BBBE0_Tick, rom);
    bad += run("Obj 0x0BC008", "work/ram/obj_BC008", Obj_BC008_Tick, rom);
    bad += run("Obj 0x0BD340", "work/ram/obj_BD340", Obj_BD340_Tick, rom);
    bad += run("Obj 0x0D7570", "work/ram/obj_D7570", Obj_D7570_Tick, rom);
    bad += run("Obj 0x0BBFE8", "work/ram/obj_BBFE8", Obj_BBFE8_Tick, rom);
    bad += run("Obj 0x0BCE8E", "work/ram/obj_BCE8E", Obj_BCE8E_Tick, rom);
    bad += run("Obj 0x0BD354", "work/ram/obj_BD354", Obj_BD354_Tick, rom);
    bad += run("Obj 0x12E1FA", "work/ram/obj_12E1FA", Obj_12E1FA_Tick, rom);
    bad += run("Obj 0x0BCF08", "work/ram/obj_BCF08", Obj_BCF08_Tick, rom);
    bad += run("Obj 0x0BCE38", "work/ram/obj_BCE38", Obj_BCE38_Tick, rom);
    bad += run("Obj 0x12F26A", "work/ram/obj_12F26A", Obj_12F26A_Tick, rom);
    bad += run("Obj 0x12F27A", "work/ram/obj_12F27A", Obj_12F27A_Tick, rom);
    bad += run("Obj 0x0BBD24", "work/ram/obj_BBD24", Obj_BBD24_Tick, rom);
    bad += run("Obj 0x0BBDE0", "work/ram/obj_BBDE0", Obj_BBDE0_Tick, rom);
    bad += run("Obj 0x0BBE9A", "work/ram/obj_BBE9A", Obj_BBE9A_Tick, rom);
    bad += run("Obj 0x0CABAC", "work/ram/obj_CABAC", Obj_CABAC_Tick, rom);
    bad += run("Obj 0x0BB746", "work/ram/obj_BB746", Obj_BB746_Tick, rom);
    bad += run("Obj 0x0BC03C", "work/ram/obj_BC03C", Obj_BC03C_Tick, rom);
    bad += run("Obj 0x0CB2C6", "work/ram/obj_CB2C6", Obj_CB2C6_Tick, rom);
    bad += run("Obj 0x0BCD90", "work/ram/obj_BCD90", Obj_BCD90_Tick, rom);
    bad += run("Obj 0x0BBCDA", "work/ram/obj_BBCDA", Obj_BBCDA_Tick, rom);
    bad += run("Obj 0x0BBD6E", "work/ram/obj_BBD6E", Obj_BBD6E_Tick, rom);
    bad += run("Obj 0x0BBE2A", "work/ram/obj_BBE2A", Obj_BBE2A_Tick, rom);
    bad += run("Obj 0x11C1DE", "work/ram/obj_11C1DE", Obj_11C1DE_Tick, rom);
    bad += run("Obj 0x12EA9E", "work/ram/obj_12EA9E", Obj_12EA9E_Tick, rom);
    bad += run("Obj 0x0BC540", "work/ram/obj_BC540", Obj_BC540_Tick, rom);
    bad += run("Obj 0x11A87E", "work/ram/obj_11A87E", Obj_11A87E_Tick, rom);
    bad += run("Obj 0x12E21A", "work/ram/obj_12E21A", Obj_12E21A_Tick, rom);
    bad += run("Obj 0x0CA28C", "work/ram/obj_CA28C", Obj_CA28C_Tick, rom);
    bad += run("Obj 0x0CAF3E", "work/ram/obj_CAF3E", Obj_CAF3E_Tick, rom);
    bad += run("Obj 0x0BC3FA", "work/ram/obj_BC3FA", Obj_BC3FA_Tick, rom);
    bad += run("Obj 0x0BC404", "work/ram/obj_BC404", Obj_BC404_Tick, rom);
    bad += run("Obj 0x0BC40E", "work/ram/obj_BC40E", Obj_BC40E_Tick, rom);
    bad += run("Obj 0x0BC418", "work/ram/obj_BC418", Obj_BC418_Tick, rom);
    bad += run("Obj 0x0BC422", "work/ram/obj_BC422", Obj_BC422_Tick, rom);
    bad += run("Obj 0x0BC24A", "work/ram/obj_BC24A", Obj_BC24A_Tick, rom);
    bad += run("Obj 0x0CAF1A", "work/ram/obj_CAF1A", Obj_CAF1A_Tick, rom);
    bad += run("Obj 0x0BB692", "work/ram/obj_BB692", Obj_BB692_Tick, rom);
    bad += run("Obj 0x0BCEB0", "work/ram/obj_BCEB0", Obj_BCEB0_Tick, rom);
    bad += run("Obj 0x12E1BE", "work/ram/obj_12E1BE", Obj_12E1BE_Tick, rom);
    bad += run("Obj 0x0BBD00", "work/ram/obj_BBD00", Obj_BBD00_Tick, rom);
    bad += run("Obj 0x0BCF30", "work/ram/obj_BCF30", Obj_BCF30_Tick, rom);
    bad += run("Obj 0x0BE262", "work/ram/obj_BE262", Obj_BE262_Tick, rom);
    bad += run("Obj 0x0BB84C", "work/ram/obj_BB84C", Obj_BB84C_Tick, rom);
    bad += run("Obj 0x0BB856", "work/ram/obj_BB856", Obj_BB856_Tick, rom);
    bad += run("Obj 0x0BB860", "work/ram/obj_BB860", Obj_BB860_Tick, rom);
    bad += run("Obj 0x0BB86A", "work/ram/obj_BB86A", Obj_BB86A_Tick, rom);
    bad += run("Obj 0x0BB874", "work/ram/obj_BB874", Obj_BB874_Tick, rom);
    bad += run("Obj 0x0BCDC0", "work/ram/obj_BCDC0", Obj_BCDC0_Tick, rom);
    bad += run("Obj 0x0CAF5E", "work/ram/obj_CAF5E", Obj_CAF5E_Tick, rom);
    bad += run("Obj 0x12EDA8", "work/ram/obj_12EDA8", Obj_12EDA8_Tick, rom);
    bad += run("Obj 0x12E19A", "work/ram/obj_12E19A", Obj_12E19A_Tick, rom);
    bad += run("Obj 0x0BC680", "work/ram/obj_BC680", Obj_BC680_Tick, rom);
    bad += run("Obj 0x0BCFF4", "work/ram/obj_BCFF4", Obj_BCFF4_Tick, rom);
    bad += run("Obj 0x12EB0E", "work/ram/obj_12EB0E", Obj_12EB0E_Tick, rom);
    bad += run("Obj 0x0BB5C2", "work/ram/obj_BB5C2", Obj_BB5C2_Tick, rom);
    bad += run("Obj 0x0BBE50", "work/ram/obj_BBE50", Obj_BBE50_Tick, rom);
    bad += run("Obj 0x0BCE52", "work/ram/obj_BCE52", Obj_BCE52_Tick, rom);
    bad += run("Obj 0x0DBDA2", "work/ram/obj_DBDA2", Obj_DBDA2_Tick, rom);
    bad += run("Obj 0x0ECF3E", "work/ram/obj_ECF3E", Obj_ECF3E_Tick, rom);
    bad += run("Obj 0x0BB952", "work/ram/obj_BB952", Obj_BB952_Tick, rom);
    bad += run("Obj 0x0BBD94", "work/ram/obj_BBD94", Obj_BBD94_Tick, rom);
    bad += run("Obj 0x0BBEC0", "work/ram/obj_BBEC0", Obj_BBEC0_Tick, rom);
    bad += run("Obj 0x0DBDE8", "work/ram/obj_DBDE8", Obj_DBDE8_Tick, rom);
    bad += run("Obj 0x0DC674", "work/ram/obj_DC674", Obj_DC674_Tick, rom);
    bad += run("Obj 0x105268", "work/ram/obj_105268", Obj_105268_Tick, rom);
    bad += run("Obj 0x0BBB18", "work/ram/obj_BBB18", Obj_BBB18_Tick, rom);
    bad += run("Obj 0x0CAEBA", "work/ram/obj_CAEBA", Obj_CAEBA_Tick, rom);
    bad += run("Obj 0x0EA28C", "work/ram/obj_EA28C", Obj_EA28C_Tick, rom);
    bad += run("Obj 0x0EA836", "work/ram/obj_EA836", Obj_EA836_Tick, rom);
    bad += run("Obj 0x10524A", "work/ram/obj_10524A", Obj_10524A_Tick, rom);
    bad += run("Obj 0x1095B0", "work/ram/obj_1095B0", Obj_1095B0_Tick, rom);
    bad += run("Obj 0x0F34F6", "work/ram/obj_F34F6", Obj_F34F6_Tick, rom);
    bad += run("Obj 0x1116E2", "work/ram/obj_1116E2", Obj_1116E2_Tick, rom);
    bad += run("Obj 0x12EA24", "work/ram/obj_12EA24", Obj_12EA24_Tick, rom);
    bad += run("Obj 0x0BD990", "work/ram/obj_BD990", Obj_BD990_Tick, rom);
    bad += run("Obj 0x12EAD4", "work/ram/obj_12EAD4", Obj_12EAD4_Tick, rom);
    bad += run("Obj 0x12CE5A", "work/ram/obj_12CE5A", Obj_12CE5A_Tick, rom);
    bad += run("Obj 0x0CB2EC", "work/ram/obj_CB2EC", Obj_CB2EC_Tick, rom);
    bad += run("Obj 0x10290C", "work/ram/obj_10290C", Obj_10290C_Tick, rom);
    bad += run("Obj 0x0E186A", "work/ram/obj_E186A", Obj_E186A_Tick, rom);
    bad += run("Obj 0x102798", "work/ram/obj_102798", Obj_102798_Tick, rom);
    bad += run("Obj 0x102844", "work/ram/obj_102844", Obj_102844_Tick, rom);
    bad += run("Obj 0x102A70", "work/ram/obj_102A70", Obj_102A70_Tick, rom);
    bad += run("Obj 0x0BBAE4", "work/ram/obj_BBAE4", Obj_BBAE4_Tick, rom);
    bad += run("Obj 0x0CBC78", "work/ram/obj_CBC78", Obj_CBC78_Tick, rom);
    bad += run("Obj 0x0DA0D6", "work/ram/obj_DA0D6", Obj_DA0D6_Tick, rom);
    bad += run("Obj 0x0ECA04", "work/ram/obj_ECA04", Obj_ECA04_Tick, rom);
    bad += run("Obj 0x0EA418", "work/ram/obj_EA418", Obj_EA418_Tick, rom);
    bad += run("Obj 0x0ECF0C", "work/ram/obj_ECF0C", Obj_ECF0C_Tick, rom);
    bad += run("Obj 0x0ECFAE", "work/ram/obj_ECFAE", Obj_ECFAE_Tick, rom);
    bad += run("Obj 0x0BBBF4", "work/ram/obj_BBBF4", Obj_BBBF4_Tick, rom);
    bad += run("Obj 0x0BC576", "work/ram/obj_BC576", Obj_BC576_Tick, rom);
    bad += run("Obj 0x0BDE42", "work/ram/obj_BDE42", Obj_BDE42_Tick, rom);
    bad += run("Obj 0x0EA29E", "work/ram/obj_EA29E", Obj_EA29E_Tick, rom);
    bad += run("Obj 0x104EFA", "work/ram/obj_104EFA", Obj_104EFA_Tick, rom);
    bad += run("Obj 0x0C9B3E", "work/ram/obj_C9B3E", Obj_C9B3E_Tick, rom);
    bad += run("Obj 0x0CBA34", "work/ram/obj_CBA34", Obj_CBA34_Tick, rom);
    bad += run("Obj 0x0ECFDA", "work/ram/obj_ECFDA", Obj_ECFDA_Tick, rom);
    bad += run("Obj 0x0BD308", "work/ram/obj_BD308", Obj_BD308_Tick, rom);
    bad += run("Obj 0x0CB054", "work/ram/obj_CB054", Obj_CB054_Tick, rom);
    bad += run("Obj 0x0CB422", "work/ram/obj_CB422", Obj_CB422_Tick, rom);
    bad += run("Obj 0x0CAF90", "work/ram/obj_CAF90", Obj_CAF90_Tick, rom);
    bad += run("Obj 0x0F3582", "work/ram/obj_F3582", Obj_F3582_Tick, rom);
    bad += run("Obj 0x0BC2A0", "work/ram/obj_BC2A0", Obj_BC2A0_Tick, rom);
    bad += run("Obj 0x10963E", "work/ram/obj_10963E", Obj_10963E_Tick, rom);
    bad += run("Obj 0x0EACEA", "work/ram/obj_EACEA", Obj_EACEA_Tick, rom);
    bad += run("Obj 0x0BCC96", "work/ram/obj_BCC96", Obj_BCC96_Tick, rom);
    bad += run("Obj 0x0BD2C2", "work/ram/obj_BD2C2", Obj_BD2C2_Tick, rom);
    bad += run("Obj 0x0BDDB8", "work/ram/obj_BDDB8", Obj_BDDB8_Tick, rom);
    bad += run("Obj 0x11C1FC", "work/ram/obj_11C1FC", Obj_11C1FC_Tick, rom);
    bad += run("Obj 0x0ECAC8", "work/ram/obj_ECAC8", Obj_ECAC8_Tick, rom);
    bad += run("Obj 0x0CB9EE", "work/ram/obj_CB9EE", Obj_CB9EE_Tick, rom);
    bad += run("Obj 0x11C3A8", "work/ram/obj_11C3A8", Obj_11C3A8_Tick, rom);
    bad += run("Obj 0x0BD700", "work/ram/obj_BD700", Obj_BD700_Tick, rom);
    bad += run("Obj 0x0CB97E", "work/ram/obj_CB97E", Obj_CB97E_Tick, rom);
    bad += run("Obj 0x0E1B36", "work/ram/obj_E1B36", Obj_E1B36_Tick, rom);
    bad += run("Obj 0x0C9DEC", "work/ram/obj_C9DEC", Obj_C9DEC_Tick, rom);
    bad += run("Obj 0x0CAFB8", "work/ram/obj_CAFB8", Obj_CAFB8_Tick, rom);
    bad += run("Obj 0x12CFD4", "work/ram/obj_12CFD4", Obj_12CFD4_Tick, rom);
    bad += run("Obj 0x12F29E", "work/ram/obj_12F29E", Obj_12F29E_Tick, rom);
    bad += run("Obj 0x0BD0EC", "work/ram/obj_BD0EC", Obj_BD0EC_Tick, rom);
    bad += run("Obj 0x12EF40", "work/ram/obj_12EF40", Obj_12EF40_Tick, rom);
    bad += run("Obj 0x0BC16C", "work/ram/obj_BC16C", Obj_BC16C_Tick, rom);
    bad += run("Obj 0x0BC1E8", "work/ram/obj_BC1E8", Obj_BC1E8_Tick, rom);
    bad += run("Obj 0x11A8A8", "work/ram/obj_11A8A8", Obj_11A8A8_Tick, rom);
    bad += run("Obj 0x0BDDE2", "work/ram/obj_BDDE2", Obj_BDDE2_Tick, rom);
    bad += run("Obj 0x0BDC5C", "work/ram/obj_BDC5C", Obj_BDC5C_Tick, rom);
    bad += run("Obj 0x0BC08C", "work/ram/obj_BC08C", Obj_BC08C_Tick, rom);
    bad += run("Obj 0x0BD3A4", "work/ram/obj_BD3A4", Obj_BD3A4_Tick, rom);
    bad += run("Obj 0x0BD550", "work/ram/obj_BD550", Obj_BD550_Tick, rom);
    bad += run("Obj 0x0BD7E4", "work/ram/obj_BD7E4", Obj_BD7E4_Tick, rom);
    bad += run("Obj 0x0BE16C", "work/ram/obj_BE16C", Obj_BE16C_Tick, rom);
    bad += run("Obj 0x13EFA0", "work/ram/obj_13EFA0", Obj_13EFA0_Tick, rom);
    bad += run("Obj 0x104F26", "work/ram/obj_104F26", Obj_104F26_Tick, rom);
    bad += run("Obj 0x0BD9BE", "work/ram/obj_BD9BE", Obj_BD9BE_Tick, rom);
    bad += run("Obj 0x0BC70A", "work/ram/obj_BC70A", Obj_BC70A_Tick, rom);
    bad += run("Obj 0x0D9FC8", "work/ram/obj_D9FC8", Obj_D9FC8_Tick, rom);
    bad += run("Obj 0x12EE24", "work/ram/obj_12EE24", Obj_12EE24_Tick, rom);
    bad += run("Obj 0x0DA138", "work/ram/obj_DA138", Obj_DA138_Tick, rom);
    bad += run("Obj 0x12EB90", "work/ram/obj_12EB90", Obj_12EB90_Tick, rom);
    bad += run("Obj 0x0D7306", "work/ram/obj_D7306", Obj_D7306_Tick, rom);
    bad += run("Obj 0x0BDE92", "work/ram/obj_BDE92", Obj_BDE92_Tick, rom);
    bad += run("Obj 0x0E9DE0", "work/ram/obj_E9DE0", Obj_E9DE0_Tick, rom);
    bad += run("Obj 0x0E96C8", "work/ram/obj_E96C8", Obj_E96C8_Tick, rom);
    bad += run("Obj 0x0E1886", "work/ram/obj_E1886", Obj_E1886_Tick, rom);
    bad += run("Obj 0x0EA848", "work/ram/obj_EA848", Obj_EA848_Tick, rom);
    bad += run("Obj 0x0DB074", "work/ram/obj_DB074", Obj_DB074_Tick, rom);
    bad += run("Obj 0x10967C", "work/ram/obj_10967C", Obj_10967C_Tick, rom);
    bad += run("Obj 0x0F35B4", "work/ram/obj_F35B4", Obj_F35B4_Tick, rom);
    bad += run("Obj 0x0DC6D0", "work/ram/obj_DC6D0", Obj_DC6D0_Tick, rom);
    bad += run("Obj 0x0E1328", "work/ram/obj_E1328", Obj_E1328_Tick, rom);
    bad += run("Obj 0x0EA45E", "work/ram/obj_EA45E", Obj_EA45E_Tick, rom);
    bad += run("Obj 0x1049C4", "work/ram/obj_1049C4", Obj_1049C4_Tick, rom);
    bad += run("Obj 0x0ECB3A", "work/ram/obj_ECB3A", Obj_ECB3A_Tick, rom);
    bad += run("Obj 0x0D740E", "work/ram/obj_D740E", Obj_D740E_Tick, rom);
    bad += run("Obj 0x0DC5B6", "work/ram/obj_DC5B6", Obj_DC5B6_Tick, rom);
    bad += run("Obj 0x0D747E", "work/ram/obj_D747E", Obj_D747E_Tick, rom);
    bad += run("Obj 0x0CD646", "work/ram/obj_CD646", Obj_CD646_Tick, rom);
    bad += run("Obj 0x0CD504", "work/ram/obj_CD504", Obj_CD504_Tick, rom);
    bad += run("Obj 0x0CD5EA", "work/ram/obj_CD5EA", Obj_CD5EA_Tick, rom);
    bad += run("Obj 0x0CD556", "work/ram/obj_CD556", Obj_CD556_Tick, rom);
    bad += run("Obj 0x1180F8", "work/ram/obj_1180F8", Obj_1180F8_Tick, rom);
    bad += run("Obj 0x0E077A", "work/ram/obj_E077A", Obj_E077A_Tick, rom);
    bad += run("Obj 0x0E1036", "work/ram/obj_E1036", Obj_E1036_Tick, rom);
    bad += run("Obj 0x112D1A", "work/ram/obj_112D1A", Obj_112D1A_Tick, rom);
    bad += run("Obj 0x0E101A", "work/ram/obj_E101A", Obj_E101A_Tick, rom);
    bad += run("Obj 0x1134B2", "work/ram/obj_1134B2", Obj_1134B2_Tick, rom);
    bad += run("Obj 0x113ED0", "work/ram/obj_113ED0", Obj_113ED0_Tick, rom);
    bad += run("Obj 0x112C1A", "work/ram/obj_112C1A", Obj_112C1A_Tick, rom);
    bad += run("Obj 0x1140BE", "work/ram/obj_1140BE", Obj_1140BE_Tick, rom);
    bad += run("Obj 0x112C3C", "work/ram/obj_112C3C", Obj_112C3C_Tick, rom);
    bad += run("Obj 0x118CF6", "work/ram/obj_118CF6", Obj_118CF6_Tick, rom);
    bad += run("Obj 0x112D88", "work/ram/obj_112D88", Obj_112D88_Tick, rom);
    bad += run("Obj 0x114108", "work/ram/obj_114108", Obj_114108_Tick, rom);
    bad += run("Obj 0x1134BA", "work/ram/obj_1134BA", Obj_1134BA_Tick, rom);
    bad += run("Obj 0x118DDC", "work/ram/obj_118DDC", Obj_118DDC_Tick, rom);
    bad += run("Obj 0x118D7C", "work/ram/obj_118D7C", Obj_118D7C_Tick, rom);
    bad += run("Obj 0x0F0788", "work/ram/obj_F0788", Obj_F0788_Tick, rom);
    bad += run("Obj 0x0F0846", "work/ram/obj_F0846", Obj_F0846_Tick, rom);
    bad += run("Obj 0x108936", "work/ram/obj_108936", Obj_108936_Tick, rom);
    bad += run("Obj 0x1087E8", "work/ram/obj_1087E8", Obj_1087E8_Tick, rom);
    bad += run("Obj 0x0DC486", "work/ram/obj_DC486", Obj_DC486_Tick, rom);
    bad += run("Obj 0x0E598C", "work/ram/obj_E598C", Obj_E598C_Tick, rom);
    bad += run("Obj 0x0F0814", "work/ram/obj_F0814", Obj_F0814_Tick, rom);
    bad += run("Obj 0x1088F0", "work/ram/obj_1088F0", Obj_1088F0_Tick, rom);
    bad += run("Obj 0x0E6166", "work/ram/obj_E6166", Obj_E6166_Tick, rom);
    bad += run("Obj 0x0E61AA", "work/ram/obj_E61AA", Obj_E61AA_Tick, rom);
    bad += run("Obj 0x0E5CBC", "work/ram/obj_E5CBC", Obj_E5CBC_Tick, rom);
    bad += run("Obj 0x0DC358", "work/ram/obj_DC358", Obj_DC358_Tick, rom);
    bad += run("Obj 0x0E60EA", "work/ram/obj_E60EA", Obj_E60EA_Tick, rom);
    bad += run("Obj 0x0E5C2A", "work/ram/obj_E5C2A", Obj_E5C2A_Tick, rom);
    bad += run("Obj 0x0F5AFE", "work/ram/obj_F5AFE", Obj_F5AFE_Tick, rom);
    bad += run("Obj 0x0E6082", "work/ram/obj_E6082", Obj_E6082_Tick, rom);
    bad += run("Obj 0x0F5D58", "work/ram/obj_F5D58", Obj_F5D58_Tick, rom);
    bad += run("Obj 0x0F5CF6", "work/ram/obj_F5CF6", Obj_F5CF6_Tick, rom);
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
