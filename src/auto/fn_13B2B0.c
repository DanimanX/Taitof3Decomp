/* ROM 0x13B2B0: segmented dragon/serpent boss extend-and-track tick (A0 =
 * object). When A5-0x144B bit 4 is set or +0x24 <= 0, awards 0x7530 score via
 * 0x0CBDDE (when +0x24 <= 0), sets +0x24 = -1, spawns 0x13E4EE, plays sound
 * 0x40, sets +0x06 = 0x14, draws segments (0x13B55A) and transitions to
 * 0x13B498. When A5+0x32D6 >= 2, increments A5+0x32DA and frees self
 * (0x0105EC). Otherwise draws segments via 0x13B55A, steers head angle
 * +0x50[+0x22.b] toward player if +0x22.b > 1 (0x006422), propagates angles
 * forward along +0x50..+0x50[+0x22.b], grows +0x22 until > 10 then counts +0x06
 * to 0x64 (transitioning to 0x13B3A6), appends to collision lists (0x0CBDA4),
 * and re-arms 0x13B2B0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x0CBDA4: append obj to both A5+0x8FE and A5+0x906 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj); a1 += 4u;
    wr32(m, F3_A5 + 0x08FEu, a1);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj); a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_out = a1;
}

/* ROM 0x13B654: draw sprite centered by sprite-def {h,w}*8 */
static void sub_13B654(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t saved_x = rd32(m, obj + 0x14u);
    uint32_t saved_y = rd32(m, obj + 0x18u);
    uint32_t spr = rd32(m, obj + 0x10u);
    uint16_t dy = (uint16_t)(rd16(m, spr) << 3);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
    uint16_t dx = (uint16_t)(rd16(m, spr + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, SPR_CURSOR);
    if (rd32(m, spr) == 0x00010001u) {
        uint32_t d0 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
        d0 &= 0x0FFF0FFFu;
        d0 = setw(d0, rd16(m, spr + 6u));
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u));
    } else {
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(rd16(m, spr) - 1u) << 16) | 0xFFFFu;
    }

    wr32(m, obj + 0x18u, saved_y);
    wr32(m, obj + 0x14u, saved_x);
}

/* ROM 0x13B55A: draw body chain + head segment */
static void sub_13B55A(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    Obj_11C32C(m, obj);                                  /* 0x13B55A */
    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x13B560 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, a1))); /* 0x13B564..0x13B56A */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, a1 + 2u))); /* 0x13B56E..0x13B576 */

    uint32_t d1 = 0u;                                    /* 0x13B57A */
    a1 = obj + 0x50u;                                    /* 0x13B57C */
    uint32_t a2 = 0x0013B680u;                           /* 0x13B580 */
    uint32_t a3 = 0x000CC10Eu;                           /* 0x13B584 */
    wr16(m, obj + 0x42u, 0x0020u);                       /* 0x13B58A */

    while ((int8_t)d1 < (int8_t)rd8(m, obj + 0x22u)) {   /* 0x13B590..0x13B594 */
        uint16_t ang = rd8(m, a1);                       /* 0x13B59C..0x13B59E */
        uint16_t adj = (uint16_t)(ang + 4u);             /* 0x13B5A0..0x13B5A2 */
        uint16_t flip_idx = (uint16_t)((adj >> 6) & 3u); /* 0x13B5A4 */
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, a3 + flip_idx) ^ 0x02u)); /* 0x13B5A8..0x13B5AE */
        uint16_t spr_idx = (uint16_t)((adj >> 3) & 0x0Fu); /* 0x13B5B4 */
        wr32(m, obj + 0x10u, rd32(m, a2 + (uint32_t)spr_idx * 4u)); /* 0x13B5B8 */
        Obj_MoveByAngle(m, obj, ang);                    /* 0x13B5C2 */
        {
            uint32_t tmp_d0 = ang, tmp_d1 = adj;
            sub_13B654(m, obj, &tmp_d0, &tmp_d1, a1_io); /* 0x13B5C8 */
        }
        Obj_MoveByAngle(m, obj, ang);                    /* 0x13B5D0 */
        d1 = setw(d1, (uint16_t)((uint16_t)d1 + 1u));    /* 0x13B5DA */
        a1 += 1u;                                        /* 0x13B5DC */
    }

    *d1_io = d1;
    *a1_io = a1;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)              /* 0x13B5E0..0x13B5E4 */
        return;

    if ((int8_t)rd8(m, obj + 0x22u) <= 1) {              /* 0x13B5E8..0x13B5EE */
        uint32_t idx = (rd16(m, obj + 0x22u) >> 6) & 7u; /* 0x13B5F2 bfextu {7:3} */
        uint32_t desc = rd32(m, obj + 0x0Cu);            /* 0x13B5F8 */
        wr8(m, obj + 0x1Cu, rd8(m, desc + 9u));          /* 0x13B5FC */
        wr32(m, obj + 0x10u, rd32(m, desc + 0x0Eu + idx * 4u)); /* 0x13B602 */
    } else {
        uint16_t ang = rd8(m, a1);                       /* 0x13B60C..0x13B60E */
        uint16_t adj = (uint16_t)(ang + 0x0044u);        /* 0x13B610 */
        uint8_t flip = (adj & 0x0080u) ? 0xFFu : 0x00u;  /* 0x13B614 bfexts {0x18:1} */
        flip = (uint8_t)((flip & 3u) ^ 1u);              /* 0x13B618..0x13B61C */
        wr8(m, obj + 0x1Cu, flip);                       /* 0x13B620 */
        uint16_t spr_idx = (uint16_t)((adj >> 3) & 0x0Fu); /* 0x13B624 */
        wr32(m, obj + 0x10u, rd32(m, 0x0013B6C0u + (uint32_t)spr_idx * 4u)); /* 0x13B628 */
        *d1_io = adj;
    }

    {
        uint32_t spd = (rd8(m, obj + 0x23u) >> 2) & 0x3Fu; /* 0x13B632 bfextu {8:6} */
        uint32_t desc = rd32(m, obj + 0x0Cu);            /* 0x13B638 */
        uint16_t d0w = (uint16_t)((uint16_t)spd + rd16(m, desc + 4u)); /* 0x13B63C */
        wr16(m, obj + 0x42u, d0w);                       /* 0x13B640 */
        uint16_t ang = rd8(m, a1);                       /* 0x13B644..0x13B646 */
        *d0_io = ang;
        {
            int16_t sp = (int16_t)d0w;
            int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
            *d1_io = (uint32_t)((int32_t)sp * (int32_t)cos_v);
        }
        Obj_MoveByAngle(m, obj, ang);                    /* 0x13B648 */
        sub_13B654(m, obj, d0_io, d1_io, a1_io);         /* 0x13B64E */
    }
}

int Fn_13B2B0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x13B2B0..0x13B2B6 */
        goto die_explode;

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 2) {        /* 0x13B2BA..0x13B2C0 */
        wr8(m, F3_A5 + 0x32DAu, (uint8_t)(rd8(m, F3_A5 + 0x32DAu) + 1u)); /* 0x13B548 */
        Obj_Free(m, obj);                                /* 0x13B54E (preserves D0,A0,A1) */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x13B2C4..0x13B2C8 */
        a1 = Obj_0CBDDE(m, obj);                         /* 0x13B43E */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u); /* 0x13B444 */
        goto die_explode;
    }

    sub_13B55A(m, obj, &d0, &d1, &a1);                   /* 0x13B2CC */

    if ((int8_t)rd8(m, obj + 0x22u) > 1) {               /* 0x13B2D0..0x13B2D6 */
        uint32_t p = rd32(m, F3_A5 + 0x30E2u);           /* 0x13B2DA */
        uint16_t dx = (uint16_t)(rd16(m, p + 0x14u) + 0x18u - rd16(m, obj + 0x14u)); /* 0x13B2DE..0x13B2E6 */
        uint16_t dy = (uint16_t)(rd16(m, p + 0x18u) + 0x10u - rd16(m, obj + 0x18u)); /* 0x13B2EA..0x13B2F2 */
        uint16_t ang = Obj_Atan2(m, dx, dy);             /* 0x13B2F6 */
        d1 = rd8(m, obj + 0x22u);                        /* 0x13B300 */
        a1 = obj + 0x50u + d1;                           /* 0x13B2FC..0x13B306 */
        int8_t diff = (int8_t)((uint8_t)ang - rd8(m, a1)); /* 0x13B308 */
        if (diff > 3)
            diff = 3;
        else if (diff < -3)
            diff = -3;
        wr8(m, a1, (uint8_t)(rd8(m, a1) + (uint8_t)diff)); /* 0x13B31E */
        d0 = (ang & 0xFFFFFF00u) | (uint8_t)diff;
        d1 = (d1 & 0xFFFFFF00u) | (uint8_t)(diff > 3 || diff < -3 ? (diff < 0 ? 0xFD : 3) : 0xFD);
    }

    d1 = rd8(m, obj + 0x22u);                            /* 0x13B324 */
    a1 = obj + 0x50u + d1;                               /* 0x13B320..0x13B32A */
    wr8(m, a1 + 1u, rd8(m, a1));                         /* 0x13B32C */
    d0 = rd8(m, obj + 0x22u);                            /* 0x13B334 */
    a1 = obj + 0x50u;                                    /* 0x13B330 */

    {
        uint16_t cnt = (uint16_t)d0;
        for (;;) {
            int8_t diff = (int8_t)(rd8(m, a1 + 1u) - rd8(m, a1)); /* 0x13B33A..0x13B33E */
            if (diff > 6)
                diff = 6;
            else if (diff < -6)
                diff = -6;
            wr8(m, a1, (uint8_t)(rd8(m, a1) + (uint8_t)diff)); /* 0x13B354 */
            a1 += 1u;
            d1 = (d1 & 0xFFFFFF00u) | (uint8_t)diff;
            cnt--;
            if (cnt == 0xFFFFu)
                break;
        }
        d0 = setw(d0, 0xFFFFu);
    }

    if ((int8_t)rd8(m, obj + 0x22u) <= 0x0A) {           /* 0x13B35A..0x13B360 */
        if ((int8_t)rd8(m, obj + 0x22u) <= 1)            /* 0x13B364..0x13B36A */
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0010u)); /* 0x13B36E */
        else
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0080u)); /* 0x13B378 */
    } else {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) + 1u); /* 0x13B382 */
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t >= 0x0064) {                      /* 0x13B386..0x13B38C */
            wr16(m, obj + 0x06u, 0u);                    /* 0x13B39A */
            wr32(m, obj + 0x00u, 0x0013B3A6u);           /* 0x13B39E */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    sub_0CBDA4(m, obj, &a1);                             /* 0x13B390 */
    wr32(m, obj + 0x00u, 0x0013B2B0u);                   /* 0x13B396 -> 0x13B2A8 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

die_explode:
    wr32(m, obj + 0x24u, 0xFFFFFFFFu);                   /* 0x13B44C */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u);     /* 0x13B454..0x13B468 */
    Sound_PlayHooked(m, 0x0040u);                        /* 0x13B472..0x13B482 */
    wr16(m, obj + 0x06u, 0x0014u);                       /* 0x13B486 */
    sub_13B55A(m, obj, &d0, &d1, &a1);                   /* 0x13B48C */
    wr32(m, obj + 0x00u, 0x0013B498u);                   /* 0x13B490 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
