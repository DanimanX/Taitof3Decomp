/* ROM 0x120114: boss/enemy articulated segment tick (A0 = object).
 * If parent (+0x08)->+0x59 == 0 and not breaking off (A5-0x144B bit 4 or
 * (rng & 0xFF) == 0), selects frame record from (+0x0C) via A5+0x3262 (or
 * animates (+0x0C)+0x10 via 0x0CBD52), unpacks position/hitbox via 0x12048C,
 * draws via 0x11B09A, and re-arms 0x120114. Otherwise aims toward screen
 * origin via 0x006422, sets speed 0x18, clears +0x1D, and transitions to
 * 0x120174. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
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
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x11B09A with D0/D1/A1 register tracking. */
static void sub_11B09A_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        *a1_io = a1;
        *d0_io = setw(*d0_io, (uint16_t)h);
        if ((int16_t)(h + y) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            *a1_io = a1;
            *d0_io = setw(*d0_io, (uint16_t)w);
            if ((int16_t)(w + x) > 0)
                draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
        }
    } else {
        *d0_io = setw(*d0_io, (uint16_t)y);
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t sy = rd16(m, obj + 0x56u);
    *d0_io = setw(*d0_io, sy);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy));
}

/* ROM 0x12048C: unpack offset, sprite ptr, and hitbox bytes from A1. */
static uint32_t sub_12048C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint32_t d0 = *d0_io;
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x50u, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Cu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)d0);
    *d0_io = d0;
    return a1;
}

/* ROM 0x1201E0: select record from (+0x0C) and unpack via 0x12048C. */
static void sub_1201E0(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x1201E0 */
    uint32_t d0 = (uint32_t)rd8(m, F3_A5 + 0x3262u);    /* 0x1201E4..0x1201E6 */
    if (d0 != 0u) {                                     /* 0x1201EA */
        a1 = rd32(m, a1 + (uint32_t)((int32_t)(int16_t)d0 * 4 - 4)); /* 0x1201EE */
    } else {
        a1 += 0x10u;                                    /* 0x1201F6 */
        Obj_Animate(m, obj, a1);                        /* 0x1201FA */
        d0 = setw(d0, rd16(m, obj + 0x3Au));
        *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
        a1 = rd32(m, obj + 0x10u);                      /* 0x120200 */
    }
    *a1_io = sub_12048C(m, obj, a1, &d0);               /* 0x120204 */
    *d0_io = d0;
}

int Fn_120114(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x120114 */

    if (rd8(m, a1 + 0x59u) != 0u)                      /* 0x120118..0x12011C */
        goto detach;

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x120120..0x120126 */
        d0 = Rng_NextMem(m);                            /* 0x12012A */
        d0 = setw(d0, (uint16_t)(d0 & 0x00FFu));        /* 0x120130 */
        if ((uint16_t)d0 == 0u)                         /* 0x120134 */
            goto detach;
    }

    sub_1201E0(m, obj, &d0, &d1, &a1);                  /* 0x120138 */
    wr32(m, obj + 0x10u, rd32(m, a1));                  /* 0x12013C */
    sub_11B09A_regs(m, obj, &d0, &d1, &a1);             /* 0x120140 */
    wr32(m, obj, 0x00120114u);                          /* 0x120146 -> 0x12010C */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

detach:
    {
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, F3_A5 + 0x30D2u)); /* 0x120148 */
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, F3_A5 + 0x30D6u)); /* 0x120150 */
        uint16_t ang = Obj_Atan2(m, dx, dy);            /* 0x120158 */
        d0 = setw(d0, ang);
        d1 = setw(d1, dy);
        wr16(m, obj + 0x20u, ang);                      /* 0x12015E */
        wr16(m, obj + 0x42u, 0x0018u);                  /* 0x120162 */
        wr8(m, obj + 0x1Du, 0u);                        /* 0x120168 */
        wr32(m, obj, 0x00120174u);                      /* 0x12016C */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }
}
