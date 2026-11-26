/* ROM 0x11D88A: sub-part/wing object tick (A0 = object, A0+0x08 = parent).
 * Frees itself if parent+0x59 != 0. If A5-0x144B bit 4 is set, clears +0x1D,
 * plays sound 0x16, spawns 0x13E4EE (via 0x13E4D0) and transitions to 0x11D980.
 * Otherwise animates via +0x0C (or locks sprite 0x32F48 when A5+0x30EE bit 1
 * from MSB is set), reflects sprite==0x32F48 in A5+0x30EE MSB bit 0, rolls
 * 0x0CC2BC(0x32) when A5+0x30EE bit 5 from MSB is set to spawn two 0x11B5E6
 * shots, calls 0x11B06E + 0x11DECC + 0x11B09A, and re-arms 0x11D88A. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0) >> 8). */
static uint16_t sub_0CC2BC(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint16_t div = (uint16_t)d1;
    uint16_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    r->d[0] = ((uint32_t)quot << 16) | (uint32_t)rem;
    r->d[1] = d1;
    return rem;
}

/* ROM 0x11B09A with D0/D1/A1 register tracking. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->a[1] = a1;
        if ((int16_t)(h + y) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->a[1] = a1;
            if ((int16_t)(w + x) > 0)
                draw_sprite_regs(m, r);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t d0w = rd16(m, obj + 0x56u);
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
}

/* ROM 0x11DECC: inherit parent+0x1D or look up table 0x11DE76 / 0x11DE8E. */
static void sub_11DECC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x11DECC */
    r->a[1] = a1;
    uint8_t c = rd8(m, a1 + 0x1Du);                     /* 0x11DED0 */
    wr8(m, obj + 0x1Du, c);
    if (c != 0u)                                        /* 0x11DED6 */
        return;

    a1 = 0x0011DE76u;                                   /* 0x11DEDA */
    int16_t d0w = (int16_t)rd16(m, obj + 0x40u);        /* 0x11DEDE */
    if (d0w >= 0x0C) {                                  /* 0x11DEE2..0x11DEE6 */
        a1 = 0x0011DE8Eu;                               /* 0x11DEEA */
        d0w = 0;                                        /* 0x11DEEE */
    }
    d0w = (int16_t)(d0w + (int16_t)rd16(m, F3_A5 + 0x3258u)); /* 0x11DEF0 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + (uint32_t)(int32_t)d0w)); /* 0x11DEF4 */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->a[1] = a1;
}

int Fn_11D88A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x11D88A */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                     /* 0x11D88E..0x11D892 */
        Obj_Free(m, obj);                               /* 0x11DA1E..0x11DA2A */
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x11D896..0x11D89C */
        wr8(m, obj + 0x1Du, 0u);                        /* 0x11D93E */
        Sound_PlayHooked(m, 0x0016u);                   /* 0x11D942..0x11D952 */
        uint32_t d0 = Obj_13E4D0(m, obj, 5u);           /* 0x11D956..0x11D958 */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0); /* 0x11D95E..0x11D974 */
        wr32(m, obj, 0x0011D980u);                      /* 0x11D978 */
        return 0;
    }

    /* 0x11D8A0: bftst (0x30ee,A5){1:1} = 0x40 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) == 0u) {
        a1 = rd32(m, obj + 0x0Cu);                      /* 0x11D8AA */
        r->a[1] = a1;
        Obj_Animate(m, obj, a1);                        /* 0x11D8AE */
        r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
        r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
    } else {
        wr32(m, obj + 0x10u, 0x00032F48u);              /* 0x11D8B8 */
    }

    /* 0x11D8C0..0x11D8CA: bfins (0x10,A0)==0x32F48 -> A5+0x30EE MSB bit 0 (0x80) */
    {
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        if (rd32(m, obj + 0x10u) == 0x00032F48u) {
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | 0xFFu;
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(b | 0x80u));
        } else {
            r->d[0] &= 0xFFFFFF00u;
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(b & ~0x80u));
        }
    }

    /* 0x11D8D0: bftst (0x30ee,A5){5:1} = 0x04 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u) {
        if (sub_0CC2BC(m, r, 0x0032u) == 0u) {          /* 0x11D8DA..0x11D8E6 */
            (void)Obj_Spawn(m, obj, 0x0011B5E6u, 5u, 2u, 0x3914E000u);
            r->d[1] = 0x3A282000u;
            r->d[0] = Obj_Spawn(m, obj, 0x0011B5E6u, 5u, 2u, 0x3A282000u);
        }
    }

    /* 0x11D92A */
    Obj_11B06E(m, obj);
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
        r->a[1] = rd32(m, F3_A5 + 0x0906u);

    sub_11DECC(m, r);                                   /* 0x11D930 */
    sub_11B09A(m, r);                                   /* 0x11D934 */
    wr32(m, obj, 0x0011D88Au);                          /* 0x11D93A -> 0x11D882 */
    return 0;
}
