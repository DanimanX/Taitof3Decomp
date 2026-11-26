/* ROM 0x0CC69E: player/enemy shot or pickup animation tick (A0 = object).
 * On A5-0x144B bit 4 converts to score/pickup via 0x0CD3D6 (table 0x0CD444[1],
 * plays hooked sound if non-zero, resets anim fields, arms 0x0CD412).
 * Otherwise animates 0x0CC742 via 0x0CBD52, draws sprite (0x001282), appends
 * to list3 (A5+0x906), checks bounds 0x0CC112..0x0CC118, and decrements +0x06
 * (re-arming 0x0CC69E if > 0 else freeing via Obj_Free 0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

int Fn_0CC69E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x0CC69E */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u)); /* 0x0CC704 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u)); /* 0x0CC708 */
        a1 = rd32(m, 0x000CD444u + 4u);                 /* 0x0CD3D6 (D0w = 1) */
        uint16_t snd = rd16(m, a1);                     /* 0x0CD3DE */
        a1 += 2u;
        d0 = setw(d0, snd);
        if (snd != 0u)                                  /* 0x0CD3E0 */
            Sound_PlayHooked(m, snd);                   /* 0x0CD3E4..0x0CD3F2 */
        wr32(m, obj + 0x0Cu, a1);                       /* 0x0CD3F6 */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x0CD3FA */
        wr16(m, obj + 0x3Eu, 0u);                       /* 0x0CD3FE */
        wr16(m, obj + 0x1Eu, 0u);                       /* 0x0CD402 */
        wr16(m, obj + 0x1Cu, 0u);                       /* 0x0CD406 */
        wr32(m, obj + 0x00u, 0x000CD412u);              /* 0x0CD40A */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = 0x000CC742u;                                   /* 0x0CC6A8 */
    Obj_Animate(m, obj, a1);                            /* 0x0CC6AE */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0CC6B4 */

    a1 = rd32(m, F3_A5 + 0x0906u);                      /* 0x0CC6BA */
    wr32(m, a1, obj);                                   /* 0x0CC6BE */
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);                       /* 0x0CC6C0 */

    int16_t x = (int16_t)rd16(m, obj + 0x14u);          /* 0x0CC6C4 */
    d1 = setw(d1, (uint16_t)x);
    if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u)) { /* 0x0CC6C8 */
        Obj_Free(m, obj);                               /* 0x0CC6EA */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    int16_t y = (int16_t)rd16(m, obj + 0x18u);          /* 0x0CC6D4 */
    d1 = setw(d1, (uint16_t)y);
    if (y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) { /* 0x0CC6D8 */
        Obj_Free(m, obj);                               /* 0x0CC6EA */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x0CC6E4 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0)                                        /* 0x0CC6E8 */
        wr32(m, obj + 0x00u, 0x000CC69Eu);              /* 0x0CC696 */
    else
        Obj_Free(m, obj);                               /* 0x0CC6EA */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
