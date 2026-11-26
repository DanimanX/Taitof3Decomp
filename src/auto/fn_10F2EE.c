/* ROM 0x10F2EE: boss death burst spawner (A0 = object).
 * Plays hooked sound 0x3B, computes screen-relative position (+0x14 - A5+0x4224,
 * +0x18 - A5+0x4228), and spawns 4 child projectiles (0x10F38A or 0x10F706
 * depending on sign bit of 0x0D0E3A 4-way RNG) in group 9 (count 2) at that
 * position. Transitions to Obj_StartDying (0x0CEE74) on pool-full or completion. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_10F2EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Sound_PlayHooked(m, 0x003Bu);                        /* 0x10F2EE..0x10F2FE */

    uint32_t d1 = rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u); /* 0x10F304 */
    uint32_t d2 = rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u); /* 0x10F30C */
    uint32_t d0 = 3u;                                    /* 0x10F302 moveq 3,D7 */
    uint32_t a1 = r->a[1];

    r->d[1] = d1;
    r->d[2] = d2;

    for (int i = 0; i < 4; i++) {                        /* 0x10F314..0x10F380 */
        uint16_t rnd = Obj_Next4Way(m);                  /* 0x10F314 */
        d0 = setw(d0, rnd);
        uint32_t fn = ((int8_t)(uint8_t)rnd >= 0) ? 0x0010F38Au : 0x0010F706u;
        d0 = Obj_Spawn(m, obj, fn, 9u, 2u, 0u);          /* 0x10F332 / 0x10F364 */
        if ((int32_t)d0 < 0) {                           /* 0x10F33C / 0x10F36E */
            r->d[0] = d0;
            r->d[7] = setw(r->d[7], (uint16_t)(3 - i));
            r->a[1] = a1;
            Obj_StartDying(m, obj);
            return 0;
        }
        a1 = d0;
        wr32(m, a1 + 0x14u, d1);                         /* 0x10F346 / 0x10F378 */
        wr32(m, a1 + 0x18u, d2);                         /* 0x10F34A / 0x10F37C */
    }

    r->d[0] = d0;
    r->d[7] = setw(r->d[7], 0xFFFFu);
    r->a[1] = a1;
    Obj_StartDying(m, obj);                              /* 0x10F384 */
    return 0;
}
