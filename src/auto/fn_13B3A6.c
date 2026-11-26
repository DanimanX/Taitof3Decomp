/* ROM 0x13B3A6: segmented dragon/serpent boss retract tick (A0 = object).
 * Same death/clear/explosion preamble as 0x13B2B0. Otherwise draws segments
 * via 0x13B55A, eases head angle +0x50 toward descriptor +0x0C->+0x08 by +/-2,
 * propagates angles backward from +0x50[+0x22.b] down to +0x50 by +/-2,
 * decrements +0x22.w by 0x40 until <= 0 (resetting +0x50 = desc+0x08, +0x22 =
 * 0 and transitioning to 0x13B25E), else appends to collision lists
 * (0x0CBDA4) and re-arms 0x13B3A6. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
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
    Obj_11C32C(m, obj);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, a1 + 2u)));

    uint32_t d1 = 0u;
    a1 = obj + 0x50u;
    uint32_t a2 = 0x0013B680u;
    uint32_t a3 = 0x000CC10Eu;
    wr16(m, obj + 0x42u, 0x0020u);

    while ((int8_t)d1 < (int8_t)rd8(m, obj + 0x22u)) {
        uint16_t ang = rd8(m, a1);
        uint16_t adj = (uint16_t)(ang + 4u);
        uint16_t flip_idx = (uint16_t)((adj >> 6) & 3u);
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, a3 + flip_idx) ^ 0x02u));
        uint16_t spr_idx = (uint16_t)((adj >> 3) & 0x0Fu);
        wr32(m, obj + 0x10u, rd32(m, a2 + (uint32_t)spr_idx * 4u));
        Obj_MoveByAngle(m, obj, ang);
        {
            uint32_t tmp_d0 = ang, tmp_d1 = adj;
            sub_13B654(m, obj, &tmp_d0, &tmp_d1, a1_io);
        }
        Obj_MoveByAngle(m, obj, ang);
        d1 = setw(d1, (uint16_t)((uint16_t)d1 + 1u));
        a1 += 1u;
    }

    *d1_io = d1;
    *a1_io = a1;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        return;

    if ((int8_t)rd8(m, obj + 0x22u) <= 1) {
        uint32_t idx = (rd16(m, obj + 0x22u) >> 6) & 7u;
        uint32_t desc = rd32(m, obj + 0x0Cu);
        wr8(m, obj + 0x1Cu, rd8(m, desc + 9u));
        wr32(m, obj + 0x10u, rd32(m, desc + 0x0Eu + idx * 4u));
    } else {
        uint16_t ang = rd8(m, a1);
        uint16_t adj = (uint16_t)(ang + 0x0044u);
        uint8_t flip = (adj & 0x0080u) ? 0xFFu : 0x00u;
        flip = (uint8_t)((flip & 3u) ^ 1u);
        wr8(m, obj + 0x1Cu, flip);
        uint16_t spr_idx = (uint16_t)((adj >> 3) & 0x0Fu);
        wr32(m, obj + 0x10u, rd32(m, 0x0013B6C0u + (uint32_t)spr_idx * 4u));
        *d1_io = adj;
    }

    {
        uint32_t spd = (rd8(m, obj + 0x23u) >> 2) & 0x3Fu;
        uint32_t desc = rd32(m, obj + 0x0Cu);
        uint16_t d0w = (uint16_t)((uint16_t)spd + rd16(m, desc + 4u));
        wr16(m, obj + 0x42u, d0w);
        uint16_t ang = rd8(m, a1);
        *d0_io = ang;
        {
            int16_t sp = (int16_t)d0w;
            int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
            *d1_io = (uint32_t)((int32_t)sp * (int32_t)cos_v);
        }
        Obj_MoveByAngle(m, obj, ang);
        sub_13B654(m, obj, d0_io, d1_io, a1_io);
    }
}

int Fn_13B3A6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x13B3A6..0x13B3AC */
        goto die_explode;

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 2) {        /* 0x13B3B0..0x13B3B6 */
        wr8(m, F3_A5 + 0x32DAu, (uint8_t)(rd8(m, F3_A5 + 0x32DAu) + 1u)); /* 0x13B548 */
        Obj_Free(m, obj);                                /* 0x13B54E (preserves D0,A0,A1) */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x13B3BA..0x13B3BE */
        a1 = Obj_0CBDDE(m, obj);                         /* 0x13B43E */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u); /* 0x13B444 */
        goto die_explode;
    }

    sub_13B55A(m, obj, &d0, &d1, &a1);                   /* 0x13B3C2 */

    {
        uint32_t desc = rd32(m, obj + 0x0Cu);            /* 0x13B3C6 */
        uint8_t target = rd8(m, desc + 8u);              /* 0x13B3CA */
        a1 = obj + 0x50u;                                /* 0x13B3CE */
        int8_t diff = (int8_t)(target - rd8(m, a1));     /* 0x13B3D2 */
        if (diff > 2)
            diff = 2;
        else if (diff < -2)
            diff = -2;
        wr8(m, a1, (uint8_t)(rd8(m, a1) + (uint8_t)diff)); /* 0x13B3E8 */
    }

    d1 = rd8(m, obj + 0x22u);                            /* 0x13B3EE */
    a1 = obj + 0x50u + d1;                               /* 0x13B3EA..0x13B3F4 */
    wr8(m, a1 + 1u, rd8(m, a1));                         /* 0x13B3F6 */
    d0 = rd8(m, obj + 0x22u);                            /* 0x13B3FE */
    a1 = obj + 0x50u + d0;                               /* 0x13B3FA..0x13B404 */

    {
        uint16_t cnt = (uint16_t)d0;
        for (;;) {
            int8_t diff = (int8_t)(rd8(m, a1) - rd8(m, a1 + 1u)); /* 0x13B406..0x13B408 */
            if (diff > 2)
                diff = 2;
            else if (diff < -2)
                diff = -2;
            wr8(m, a1 + 1u, (uint8_t)(rd8(m, a1 + 1u) + (uint8_t)diff)); /* 0x13B420 */
            a1 -= 1u;                                    /* 0x13B424 */
            d1 = (d1 & 0xFFFFFF00u) | (uint8_t)diff;
            cnt--;
            if (cnt == 0xFFFFu)
                break;
        }
        d0 = setw(d0, 0xFFFFu);
    }

    {
        uint16_t v = (uint16_t)(rd16(m, obj + 0x22u) - 0x0040u); /* 0x13B42A */
        wr16(m, obj + 0x22u, v);
        if ((int16_t)v <= 0) {                           /* 0x13B430 */
            a1 = obj + 0x50u;                            /* 0x13B246 */
            uint32_t a2 = rd32(m, obj + 0x0Cu);          /* 0x13B24A */
            wr8(m, a1, rd8(m, a2 + 8u));                 /* 0x13B24E */
            wr16(m, obj + 0x22u, 0u);                    /* 0x13B252 */
            wr32(m, obj + 0x00u, 0x0013B25Eu);           /* 0x13B256 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    sub_0CBDA4(m, obj, &a1);                             /* 0x13B434 */
    wr32(m, obj + 0x00u, 0x0013B3A6u);                   /* 0x13B43A -> 0x13B39E */
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
