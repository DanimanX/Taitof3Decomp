/* ROM 0x12EF86: tentacle/arm deploy animation tick (A0 = object).
 * Inherits transform and flash colour from +0x0C owner via 0x12F242 + 0x12FF36,
 * offsets (+0x14, +0x18) by (+0x10, +0x20), draws sprite via 0x001282, then
 * checks A5+0x30F2 bit 0 (clear owner+0x3E and free) and steps deploy table
 * 0x12F176 via 0x0CBD24. While dur > 0 updates owner+0x3E and re-arms
 * 0x12EF86; once <= 0 initializes 4 limb angle/speed pairs (+0x4A) from
 * 0x12F22E (flipped when owner+0x1C bit 0 set) and enters active state
 * 0x12EFFE or retract/death states 0x12F0A8 / 0x12EF40 / 0x12F13C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x0CBD24 with D0/D1 register tracking. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0_io = setw(*d0_io, off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = setw(*d1_io, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

/* ROM 0x12F242 + 0x12FF36: inherit owner transform/colour and sync flash. */
static void sub_12F242(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t owner = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x1Cu, rd8(m, owner + 0x1Cu));
    wr16(m, obj + 0x1Eu, rd16(m, owner + 0x1Eu));
    wr32(m, obj + 0x14u, rd32(m, owner + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, owner + 0x18u));
    wr8(m, obj + 0x4Fu, 0x40u);

    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);                   /* 0x12FF36 */
    uint8_t d0b = rd8(m, a1 + 0x1Du);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | d0b;
    if (d0b == rd8(m, a1 + 0x4Fu))
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    else
        wr8(m, obj + 0x1Du, d0b);
    r->a[1] = a1;
}

static int free_and_clear_owner(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                       /* 0x12F160 */
    wr8(m, a1 + 0x3Eu, 0u);
    r->a[1] = a1;
    Obj_Free(m, obj);
    return 0;
}

int Fn_12EF86(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_12F242(m, r);                                         /* 0x12EF86 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x12EF8A */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0020u)); /* 0x12EF90 */
    draw_sprite_regs(m, r);                                   /* 0x12EF96 */

    /* 0x12EF9C -> 0x12EF50 */
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u)                 /* 0x12EF50 */
        return free_and_clear_owner(m, r, obj);

    r->a[1] = 0x0012F176u;                                    /* 0x12EF5A */
    sub_0CBD24(m, obj, 0x0012F176u, &r->d[0], &r->d[1]);      /* 0x12EF60 */
    if ((int16_t)rd16(m, 0x0012F176u + (int16_t)(uint16_t)r->d[0]) > 0) { /* 0x12EF66..0x12EF6A */
        uint32_t a1 = rd32(m, obj + 0x0Cu);                   /* 0x12EF6E */
        wr8(m, a1 + 0x3Eu, (rd32(m, obj + 0x10u) != 0x00028740u) ? 0xFFu : 0u); /* 0x12EF72 */
        r->a[1] = a1;
        wr32(m, obj, 0x0012EF86u);                            /* 0x12EF7E */
        return 0;
    }

    /* 0x12EF9E */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x44u, 0u);
    wr16(m, obj + 0x46u, 0u);
    wr16(m, obj + 0x48u, 0u);

    uint32_t a1 = obj + 0x4Au;                                /* 0x12EFB2 */
    uint32_t a2 = 0x0012F22Eu;                                /* 0x12EFB6 */
    uint32_t a3 = rd32(m, obj + 0x0Cu);                       /* 0x12EFBC */
    uint16_t d2w = (uint16_t)r->d[2];
    for (int i = 3; i >= 0; i--) {                            /* 0x12EFC0..0x12EFDE */
        d2w = rd16(m, a2); a2 += 2u;
        if ((rd8(m, a3 + 0x1Cu) & 1u) != 0u)                  /* 0x12EFC6 */
            d2w = (uint16_t)(0x4000 - (int16_t)(d2w - 0x4000));
        wr16(m, a1, d2w); a1 += 2u;
        wr16(m, a1, rd16(m, a2)); a1 += 2u; a2 += 2u;
    }
    r->d[0] = setw(r->d[0], 0xFFFFu);
    r->d[2] = setw(r->d[2], d2w);
    r->a[1] = a1;
    r->a[2] = a2;
    r->a[3] = a3;

    /* 0x12EFE2 */
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u)
        return free_and_clear_owner(m, r, obj);

    uint8_t b = rd8(m, F3_A5 + 0x30EFu);                      /* 0x12EFEC bfclr (0x30EE,A5){0xA:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(b & ~0x20u));
    if ((b & 0x20u) == 0u) {                                  /* 0x12EFF2 */
        wr32(m, obj, 0x0012EFFEu);                            /* 0x12EFF6 */
        return 0;
    }

    /* 0x12F08E */
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u)
        return free_and_clear_owner(m, r, obj);

    int16_t cnt = (int16_t)(rd16(m, obj + 0x48u) - 1u);       /* 0x12F098 */
    wr16(m, obj + 0x48u, (uint16_t)cnt);
    if (cnt >= 0) {                                           /* 0x12F09C */
        wr32(m, obj, 0x0012F0A8u);                            /* 0x12F0A0 */
        return 0;
    }

    /* 0x12F0FE */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u)                 /* 0x12F106 */
        return free_and_clear_owner(m, r, obj);

    r->a[1] = 0x0012F1D8u;                                    /* 0x12F110 */
    sub_0CBD24(m, obj, 0x0012F1D8u, &r->d[0], &r->d[1]);      /* 0x12F116 */
    if ((int16_t)rd16(m, 0x0012F1D8u + (int16_t)(uint16_t)r->d[0]) <= 0) { /* 0x12F11C */
        a1 = rd32(m, obj + 0x0Cu);                            /* 0x12F154 */
        wr8(m, a1 + 0x3Eu, 0u);
        r->a[1] = a1;
        wr32(m, obj, 0x0012EF40u);                            /* 0x12EF38 */
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);                                /* 0x12F124 */
    wr8(m, a1 + 0x3Eu, (rd32(m, obj + 0x10u) != 0x00028740u) ? 0xFFu : 0u);
    r->a[1] = a1;
    wr32(m, obj, 0x0012F13Cu);                                /* 0x12F134 */
    return 0;
}
