/* ROM 0x10DAB2: child part/hitbox tracker attached to parent +0x08 (A0 = object).
 * Dies via 0x0CEE74 if parent +0x06.b == 0. If +0x24 < 0, forwards damage
 * (+0x24, +0x35, +0x36) to parent and clears own +0x35/+0x24. Moves by
 * (+0x50 + A5+0x4224, +0x54 + A5+0x4228), checks horizontal alignment against
 * (+0x0C)->+0x14 + 0x10 - 8 within |d| <= 0x0C (else 0x0CEE74), and registers
 * in collision/hazard lists via 0x0D0DC2. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"

int Fn_10DAB2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x10DAB2 */
    r->a[4] = a4;

    if (rd8(m, a4 + 0x06u) == 0u) {                      /* 0x10DAB6 */
        Obj_StartDying(m, obj);                          /* 0x10DABA -> 0x0CEE74 */
        return 0;
    }

    int32_t hp = (int32_t)rd32(m, obj + 0x24u);          /* 0x10DAC0 */
    if (hp < 0) {                                        /* 0x10DAC4 bge */
        wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));         /* 0x10DAC8 */
        wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));       /* 0x10DACE */
        r->d[0] = (uint32_t)hp;                          /* 0x10DAD4 */
        wr32(m, a4 + 0x24u, rd32(m, a4 + 0x24u) + (uint32_t)hp); /* 0x10DAD8 */
        wr8(m, obj + 0x35u, 0u);                         /* 0x10DADC */
        wr32(m, obj + 0x24u, 0u);                        /* 0x10DAE0 */
    }

    uint32_t d0 = rd32(m, obj + 0x50u) + rd32(m, F3_A5 + 0x4224u); /* 0x10DAE4..0x10DAE8 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);     /* 0x10DAEC */
    d0 = rd32(m, obj + 0x54u) + rd32(m, F3_A5 + 0x4228u); /* 0x10DAF0..0x10DAF4 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);     /* 0x10DAF8 */

    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x10DAFC */
    r->a[1] = a1;

    uint16_t d0w = (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u); /* 0x10DB00..0x10DB04 */
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x14u));        /* 0x10DB08 */
    d0w = (uint16_t)(d0w - 8u);                          /* 0x10DB0C */
    if ((int16_t)d0w < 0)                                /* 0x10DB0E bpl */
        d0w = (uint16_t)(0u - d0w);                      /* 0x10DB12 */
    r->d[0] = (d0 & 0xFFFF0000u) | d0w;

    if (d0w <= 0x000Cu) {                                /* 0x10DB14..0x10DB18 bls */
        Obj_StartDying(m, obj);
        return 0;
    }

    Coll_AddEnemyHazard(m, obj);                         /* 0x10DB1E -> 0x0D0DC2 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    return 0;
}
