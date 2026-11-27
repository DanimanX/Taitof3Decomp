/* ROM 0x112EC0: boss "core" object tick (A0 = object).
 *   parent (+0x8) flag +0x50 bit 4 -> death path 0x113162: sound 0x35, spawn
 *     0x13E4EE (group 3) with the sprite centre packed by 0x13E4D0, arm 0x1131A0.
 *   own +0x35.b != 0 -> hit path 0x112F5A: sound 0x1D, flag the parent +0x50
 *     bit 1, hand it the object in +0x36, extend its +0x48 by 0x7530 and bump
 *     +0x69.b, spawn 0x117B54 with the current position, spawn 0x1131EA, reset
 *     +0x20/+0x06, arm 0x112FF6.
 *   else the live path: append to A5+0x906, move by the angle in +0x20
 *     (0x0CBE12), nudge -0xC000/+0x4000, +0x21.b -= 0xC, animate 0x1131B8,
 *     draw (0x001282) and finally either keep going (0x112EC0) or, when the
 *     off-screen bounds 0x0CC112/0x0CC116 reject the position, arm 0x112F42. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_112EC0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);
    int16_t v;

    if ((rd8(m, a1 + 0x50u) & 0x10u) != 0u) {                  /* 0x112EC4 bne 0x113162 */
        Sound_PlayHooked(m, 0x0035u);                          /* 0x113162 */
        d0 = Obj_13E4D0(m, obj, 3u);                           /* 0x113176..0x113178 */
        d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 3u, 2u, d0);       /* 0x113180..0x11318E */
        wr32(m, obj + 0x00u, 0x001131A0u);                     /* 0x113198 */
        goto out;
    }

    if (rd8(m, obj + 0x35u) != 0u) {                           /* 0x112ED2 bne 0x112F5A */
        Sound_PlayHooked(m, 0x001Du);                          /* 0x112F5A */
        a1 = rd32(m, obj + 0x08u);                             /* 0x112F6E */
        wr8(m, a1 + 0x50u, (uint8_t)(rd8(m, a1 + 0x50u) | 0x02u)); /* 0x112F72 bset #1 */
        {
            uint32_t a2 = rd32(m, obj + 0x36u);                /* 0x112F78 */
            wr32(m, a1 + 0x36u, a2);                           /* 0x112F7C */
            wr32(m, a2 + 0x48u, rd32(m, a2 + 0x48u) + 0x00007530u); /* 0x112F80 */
            wr8(m, a2 + 0x69u, (uint8_t)(rd8(m, a2 + 0x69u) + 1u)); /* 0x112F88 */
        }
        d0 = Obj_Spawn(m, obj, 0x00117B54u, 3u, 2u, 0x00007530u); /* 0x112F8C */
        if ((int32_t)d0 >= 0) {                                /* 0x112FAA tst.l ; bmi */
            a1 = d0;                                           /* 0x112FB0 */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));         /* 0x112FB2 */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));         /* 0x112FB8 */
        }
        wr8(m, obj + 0x50u, 0u);                               /* 0x112FBE sf */
        d0 = Obj_Spawn(m, obj, 0x001131EAu, 3u, 2u, 0u);       /* 0x112FC2 */
        wr16(m, obj + 0x20u, 0u);                              /* 0x112FDE clr.w */
        wr16(m, obj + 0x42u, 0x0060u);                         /* 0x112FE2 */
        wr16(m, obj + 0x06u, 0x0050u);                         /* 0x112FE8 */
        wr32(m, obj + 0x00u, 0x00112FF6u);                     /* 0x112FEE */
        goto out;
    }

    /* 0x112ED6: append this object to the A5+0x906 collision list */
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);

    {
        uint16_t ang = (uint16_t)rd16(m, obj + 0x20u);         /* 0x112EE0 */
        d0 = setw(d0, ang);
        Obj_MoveByAngle(m, obj, ang);                          /* 0x112EE4 */
        /* 0x0CBE12/0x0CBE1E/0x0CBE28: the helper also leaves D1 = speed*cos and
         * D2 = speed*sin (full 32-bit multiplies) and A3 = 0x27E40. Only D1's
         * high word survives to the rts (everything after it is a word op). */
        d1 = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x42u)
                        * (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x0080u
                                                 + 2u * (uint32_t)ang));
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x0000C000u); /* 0x112EEA subi.l */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x00004000u); /* 0x112EF2 addi.l */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 0x0Cu)); /* 0x112EFA subi.b */

    Obj_Animate(m, obj, 0x001131B8u);                          /* 0x112F00..0x112F06 */

    r->d[0] = d0;                                              /* 0x112F0C */
    r->d[1] = d1;
    draw_sprite_regs(m, r);
    d0 = r->d[0];
    d1 = r->d[1];
    a1 = r->a[1];                       /* 0x001282 leaves A1 = A5-0xCA0 cursor */

    v = (int16_t)rd16(m, obj + 0x14u);                         /* 0x112F12 */
    d0 = setw(d0, (uint16_t)v);
    if (v < (int16_t)rd16(m, 0x000CC112u)
        || v > (int16_t)rd16(m, 0x000CC114u))                  /* 0x112F16 cmp2.w */
        goto lab_112F34;

    v = (int16_t)rd16(m, obj + 0x18u);                         /* 0x112F22 */
    d0 = setw(d0, (uint16_t)v);
    if (v < (int16_t)rd16(m, 0x000CC116u)
        || v > (int16_t)rd16(m, 0x000CC118u))                  /* 0x112F26 cmp2.w */
        goto lab_112F34;

    wr32(m, obj + 0x00u, 0x00112EC0u);                         /* 0x112EB8 */
    goto out;

lab_112F34:
    wr16(m, obj + 0x06u, 0x003Cu);                             /* 0x112F34 */
    wr32(m, obj + 0x00u, 0x00112F42u);                         /* 0x112F3A */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
