/* ROM 0x0D8AF2: animated hazard/projectile tick (A0 = object).
 * On bomb-clear (A5-0x144B bit 5) plays hooked sound +0x65 (0x13F238/0x39D8),
 * packs sprite center via 0x13E4D0, spawns 0x13E4EE, and parks at 0x0D8BAC.
 * Otherwise if +0x35 is non-zero and != 2 clears (+0x36)->+0x24, steps
 * animation 0x0D8BC6 via 0x0CBD24 (resetting +0x3A/+0x3E on end if +0x35 != 0),
 * clears +0x35, calls Obj_SubScrollVel (0x0CBE00), adds to collision lists
 * (0x0CBDA4), draws sprite (0x001282), and frees (0x0105EC) if X <= -0x20
 * else re-arms 0x0D8AF2. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBDA4: append obj to A5+0x8FE and A5+0x906 with A1 tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0D8AF2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x0D8AF2 */
        d0 = (uint32_t)rd8(m, obj + 0x65u);             /* 0x0D8B6A..0x0D8B6C */
        Sound_PlayHooked(m, (uint16_t)d0);              /* 0x0D8B70..0x0D8B7E */
        d0 = Obj_13E4D0(m, obj, 2u);                    /* 0x0D8B82..0x0D8B84 */
        d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0); /* 0x0D8B8A..0x0D8BA0 */
        wr32(m, obj + 0x00u, 0x000D8BACu);              /* 0x0D8BA4 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    uint8_t hit = rd8(m, obj + 0x35u);                  /* 0x0D8AFC */
    if (hit != 0u && hit != 2u) {                       /* 0x0D8B00..0x0D8B0A */
        a1 = rd32(m, obj + 0x36u);                      /* 0x0D8B0E */
        wr32(m, a1 + 0x24u, 0u);                        /* 0x0D8B12 */
    }

    a1 = 0x000D8BC6u;                                   /* 0x0D8B16 */
    Obj_AnimFrameStep(m, obj, a1);                      /* 0x0D8B1A */
    d0 = setw(d0, rd16(m, obj + 0x3Au));
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)d0) <= 0) { /* 0x0D8B20 */
        if (rd8(m, obj + 0x35u) != 0u) {                /* 0x0D8B28 */
            wr16(m, obj + 0x3Au, 0u);                   /* 0x0D8B30 */
            wr16(m, obj + 0x3Eu, 0u);                   /* 0x0D8B34 */
        }
    }

    wr8(m, obj + 0x35u, 0u);                            /* 0x0D8B38 */
    Obj_SubScrollVel(m, obj);                           /* 0x0D8B3C */
    add_coll_regs(m, obj, &a1);                         /* 0x0D8B42 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0D8B48 */

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0020)       /* 0x0D8B4E */
        Obj_Free(m, obj);                               /* 0x0D8BAC */
    else
        wr32(m, obj + 0x00u, 0x000D8AF2u);              /* 0x0D8AEA */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
