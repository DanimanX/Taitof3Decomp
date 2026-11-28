#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

/* ROM 0x131088: explosion/smoke particle tick (A0 = object).
 *
 * Freed at once when A5-0x144B bit 4 (pause) or A5+0x1D58 bit 1 is set;
 * otherwise the object takes its parent's (+0x08) position, adds the signed
 * bytes at +0x0C / +0x0D, steps the animation table 0x1312EE through 0x0CBD52,
 * and draws the sprite 16 pixels up/left of the position (0x001282). At tick
 * 0x140 of the +0x06 countdown it plays sound 0x20 and spawns two 0x131144
 * (group 5, arg 0); the countdown then decrements and, while still positive,
 * re-arms 0x131088 (0x131080 thunk). On the way out the +0x48 successor's
 * +0x44 are cleared before 0x0105EC frees the object. */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 with the D0/D1/A1 register outputs. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {                    /* 0x0013CE */
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {                                         /* 0x001294 */
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_131088(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)                  /* 0x131088 btst #4 */
        goto drop;
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u)                  /* 0x131092 btst #1 */
        goto drop;

    {
        uint32_t a1 = rd32(m, obj + 0x08u);                       /* 0x13109C */
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));                /* 0x1310A0 */
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));                /* 0x1310A6 */
    }
    {
        uint16_t d0 = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu); /* 0x1310AC bfexts */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    }
    {
        uint16_t d0 = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du); /* 0x1310B6 bfexts */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    }

    Obj_Animate(m, obj, 0x001312EEu);                             /* 0x1310C0 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x10u)); /* 0x1310CC */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x10u)); /* 0x1310D2 */
    draw_sprite_regs(m, r);                                       /* 0x1310D8 -> 0x001282 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));  /* 0x1310DE */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));  /* 0x1310E2 */

    if (rd16(m, obj + 0x06u) == 0x0140u) {                        /* 0x1310E6 */
        Sound_PlayHooked(m, 0x20u);                               /* 0x1310F0 */
        (void)Obj_Spawn(m, obj, 0x00131144u, 0x0005u, 2u, 0u);    /* 0x131104 */
    }

    {
        int16_t t = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x131120 */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {
            wr32(m, obj + 0x00u, 0x00131088u);                    /* 0x131080 */
            return 0;
        }
    }

drop:
    {
        uint32_t d0 = rd32(m, obj + 0x48u);                       /* 0x131128 */
        r->d[0] = d0;
        if (d0 != 0u) {
            r->a[1] = d0;                                         /* 0x131130 */
            wr32(m, d0 + 0x44u, 0u);                              /* 0x131132 */
        }
    }
    Obj_Free(m, obj);                                             /* 0x131136 */
    return 0;
}
