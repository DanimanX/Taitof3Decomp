/* ROM 0x13C43E: oscillating enemy tick (A0 = object).
 * On A5-0x144B bit 4, spawns explosion 0x13E4EE (arg from 0x13E4D0 with 2),
 * plays hooked sound 0x35, and frees self (0x0105EC). Otherwise animates from
 * 0x13C5D4 (0x0CBD52); on bomb hit (A5+0x1D58 bit 1, X in [-10, 284]) relays
 * +0x4C timer to (A5+0x1D50)->+0x48 and tail-calls Obj_ExplodeTick (0x0CBB2C).
 * On +0x35 == 1, clears it, sets +0x42 = 10, and aims +0x20 away from
 * (+0x36)->+0x08 via 0x0CC3C6 + 0x80. Decays +0x42, moves along +0x20 via
 * 0x0CBE12, oscillates +0x06 between -0x180 and +0x180 (stepping by +0x23,
 * direction +0x22), drifts X by -0x8000 and Y by (+0x06 << 8), draws
 * (0x001282), adds to collision list1/list3 and sets hitboxes based on
 * A5+0x879 bit 0, and frees self if X <= -0x30 else re-arms 0x13C43E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

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
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CBE12: advance +0x14/+0x18 by +0x42 speed along angle D0w. */
static void move_angle_0CBE12(const f3mem_t *m, uint32_t obj, uint16_t d0w,
                              uint32_t *d1_io)
{
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 2u;
    int32_t dx = (int32_t)spd * (int16_t)rd16(m, 0x00027E40u + 0x80u + off);
    int32_t dy = (int32_t)spd * (int16_t)rd16(m, 0x00027E40u + off);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    *d1_io = (uint32_t)dx;
}

int Fn_13C43E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {       /* 0x13C43E */
        d0 = Obj_13E4D0(m, obj, 2u);                      /* 0x13C57C..0x13C57E */
        d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0);  /* 0x13C584..0x13C594 */
        Sound_PlayHooked(m, 0x0035u);                     /* 0x13C59E..0x13C5AA */
        Obj_Free(m, obj);                                 /* 0x13C5B2..0x13C5B4 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = 0x0013C5D4u;                                     /* 0x13C448 */
    Obj_Animate(m, obj, a1);                              /* 0x13C44C */
    d0 = setw(d0, rd16(m, obj + 0x3Au));
    d1 = setw(d1, rd16(m, obj + 0x3Eu));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x13C452 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);        /* 0x13C45A */
        if (x >= -0x000A && x <= 0x011C) {                /* 0x13C460..0x13C468 */
            a1 = rd32(m, F3_A5 + 0x1D50u);                /* 0x0CBD12 */
            d0 = (uint32_t)rd16(m, obj + 0x4Cu);          /* 0x0CBD16..0x0CBD18 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0); /* 0x0CBD1C */
            Obj_ExplodeTick(m, obj);                      /* 0x0CBD20 -> 0x0CBB2C */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if (rd8(m, obj + 0x35u) == 1u) {                      /* 0x13C470 */
        wr8(m, obj + 0x35u, 0u);                          /* 0x13C47A */
        wr16(m, obj + 0x42u, 0x000Au);                    /* 0x13C47E */
        a1 = rd32(m, rd32(m, obj + 0x36u) + 8u);          /* 0x13C484..0x13C488 */
        uint16_t ang = Obj_0CC3C6(m, obj, a1);            /* 0x13C48C */
        d0 = (d0 & 0xFFFFFF00u) | (uint8_t)((uint8_t)ang + 0x80u); /* 0x13C492 */
        wr16(m, obj + 0x20u, (uint16_t)d0);               /* 0x13C496 */
    }

    if ((int16_t)rd16(m, obj + 0x42u) > 0)                /* 0x13C49A */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u)); /* 0x13C4A2 */

    d0 = setw(d0, rd16(m, obj + 0x20u));                  /* 0x13C4A6 */
    move_angle_0CBE12(m, obj, (uint16_t)d0, &d1);         /* 0x13C4AA */

    d0 = (uint32_t)rd8(m, obj + 0x23u);                   /* 0x13C4B0..0x13C4B2 */
    if (rd8(m, obj + 0x22u) == 0u) {                      /* 0x13C4B6 */
        int16_t v = (int16_t)(rd16(m, obj + 0x06u) + (uint16_t)d0); /* 0x13C4BE */
        wr16(m, obj + 0x06u, (uint16_t)v);
        if (v >= 0x0180) {                                /* 0x13C4C2 */
            wr16(m, obj + 0x06u, 0x0180u);                /* 0x13C4CC */
            wr8(m, obj + 0x22u, 0xFFu);                   /* 0x13C4D2 */
        }
    } else {
        int16_t v = (int16_t)(rd16(m, obj + 0x06u) - (uint16_t)d0); /* 0x13C4DA */
        wr16(m, obj + 0x06u, (uint16_t)v);
        if (v <= -0x0180) {                               /* 0x13C4DE */
            wr16(m, obj + 0x06u, (uint16_t)(int16_t)-0x0180); /* 0x13C4E8 */
            wr8(m, obj + 0x22u, 0u);                      /* 0x13C4EE */
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x8000u); /* 0x13C4F2 */
    d0 = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x06u) << 8); /* 0x13C4FA..0x13C500 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);      /* 0x13C502 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                               /* 0x13C506 */
    d0 = r->d[0];
    d1 = r->d[1];

    if ((rd8(m, F3_A5 + 0x0879u) & 0x01u) == 0u) {        /* 0x13C50C */
        a1 = rd32(m, F3_A5 + 0x08FEu);                    /* 0x13C516 */
        wr32(m, a1, obj);                                 /* 0x13C51A */
        a1 += 4u;
        wr32(m, F3_A5 + 0x08FEu, a1);                     /* 0x13C51C */
        wr16(m, obj + 0x2Cu, 0x0013u);                    /* 0x13C520 */
        wr16(m, obj + 0x30u, 0x0013u);                    /* 0x13C526 */
        wr16(m, obj + 0x2Eu, 0x001Cu);                    /* 0x13C52C */
        wr16(m, obj + 0x32u, 0x001Cu);                    /* 0x13C532 */
    } else {
        a1 = rd32(m, F3_A5 + 0x0906u);                    /* 0x13C53C */
        wr32(m, a1, obj);                                 /* 0x13C540 */
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);                     /* 0x13C542 */
        wr16(m, obj + 0x2Cu, 0x000Cu);                    /* 0x13C546 */
        wr16(m, obj + 0x30u, 0x000Cu);                    /* 0x13C54C */
        wr16(m, obj + 0x2Eu, 0x0023u);                    /* 0x13C552 */
        wr16(m, obj + 0x32u, 0x0023u);                    /* 0x13C558 */
    }

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0030)         /* 0x13C55E */
        Obj_Free(m, obj);                                 /* 0x13C5B2..0x13C5B4 */
    else
        wr32(m, obj, 0x0013C43Eu);                        /* 0x13C568 -> 0x13C436 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
