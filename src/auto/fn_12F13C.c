/* ROM 0x12F13C: attached child/effect sprite tick (A0 = object).
 * Syncs flags/pos/palette from parent (+0x0C) via 0x12F242 (0x12FF36), offsets
 * (+0x14, +0x18) by (+0x10, +0x20), draws sprite (0x001282), then if A5+0x30F2
 * bit 0 is set clears parent +0x3E and frees self (0x0105EC); else steps anim
 * table 0x12F1D8 via 0x0CBD24 and sets parent +0x3E = (+0x10 != 0x28740) while
 * active, or clears parent +0x3E and transitions to 0x12EF40 when finished. */
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
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x12FF36: mirror hit-flash palette byte from *(A5+0x32AA) onto (+0x1D). */
static void sub_12FF36(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);                  /* 0x12FF36 */
    uint8_t d0b = rd8(m, a1 + 0x1Du);                        /* 0x12FF3A */
    r->a[1] = a1;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | d0b;
    if (d0b == rd8(m, a1 + 0x4Fu))                           /* 0x12FF3E..0x12FF42 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));            /* 0x12FF46 */
    else
        wr8(m, obj + 0x1Du, d0b);                            /* 0x12FF50 */
}

/* ROM 0x0CBD24 with D0/D1 register tracking. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);                 /* 0x0CBD24 */
        *d0_io = setw(*d0_io, off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off); /* 0x0CBD28 */
        if (dur <= 0)                                        /* 0x0CBD2C */
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30..0x0CBD34 */
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = setw(*d1_io, cnt);
        if ((int16_t)cnt <= dur) {                           /* 0x0CBD38..0x0CBD3C */
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u)); /* 0x0CBD4A */
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                            /* 0x0CBD40 */
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));          /* 0x0CBD44 */
    }
}

int Fn_12F13C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x12F242: copy parent fields and update +0x1D */
    uint32_t parent = rd32(m, obj + 0x0Cu);                  /* 0x12F242 */
    wr8(m, obj + 0x1Cu, rd8(m, parent + 0x1Cu));             /* 0x12F246 */
    wr16(m, obj + 0x1Eu, rd16(m, parent + 0x1Eu));           /* 0x12F24C */
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));           /* 0x12F252 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));           /* 0x12F258 */
    wr8(m, obj + 0x4Fu, 0x40u);                              /* 0x12F25E */
    sub_12FF36(m, r, obj);                                   /* 0x12F264 */

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x12F140 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0020u)); /* 0x12F146 */
    draw_sprite_regs(m, r);                                  /* 0x12F14C */

    /* 0x12F106 */
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u) {           /* 0x12F106..0x12F10C */
        wr8(m, parent + 0x3Eu, 0x00u);                       /* 0x12F160..0x12F164 */
        r->a[1] = parent;
        Obj_Free(m, obj);                                    /* 0x12F168..0x12F170 */
        return 0;
    }

    uint32_t tbl = 0x0012F1D8u;                              /* 0x12F110 (0x12F112 + 0xC6) */
    sub_0CBD24(m, obj, tbl, &r->d[0], &r->d[1]);             /* 0x12F116 */
    int16_t d0w = (int16_t)(uint16_t)r->d[0];
    if ((int16_t)rd16(m, tbl + (uint32_t)(int32_t)d0w) <= 0) { /* 0x12F11C..0x12F120 */
        wr8(m, parent + 0x3Eu, 0x00u);                       /* 0x12F154..0x12F158 */
        r->a[1] = parent;
        wr32(m, obj, 0x0012EF40u);                           /* 0x12F15C -> 0x12EF38 */
        return 0;
    }

    uint8_t active = (rd32(m, obj + 0x10u) != 0x00028740u) ? 0xFFu : 0x00u; /* 0x12F128..0x12F130 */
    wr8(m, parent + 0x3Eu, active);
    r->a[1] = parent;
    wr32(m, obj, 0x0012F13Cu);                               /* 0x12F134 */
    return 0;
}
