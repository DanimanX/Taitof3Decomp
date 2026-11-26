/* ROM 0x11DBBA: armored enemy part tick (A0 = object).
 * While +0x24 > 0: frees immediately if parent (+0x08)->+0x59 != 0; otherwise
 * animates (+0x0C) via 0x0CBD52, registers collision & flash via 0x11B06E,
 * selects base colour (+0x4F) from tables 0x11DE76/0x11DE8E (+0x40 vs A5+0x3258)
 * via 0x11DEAA, draws sprite via 0x11B09A, and re-arms 0x11DBBA. When +0x24 <= 0:
 * adds 0x12C5F twice to A5+0x0886, spawns 0x11B1BE (grp 0) and 0x11BF5C (grp 5)
 * with arg (-8,+0x0C), loads +0x10 from (-4,+0x0C), awards +0x4C to score
 * receiver (0x0CBDDE), resets +0x1D/+0x35, and transitions to 0x11B12C. */
#include "fn_auto.h"
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

/* ROM 0x11B06E: register in collision lists if active & hitbox, then Obj_11C32C. */
static void sub_11B06E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        *a1_io = p2 + 4u;
        wr32(m, F3_A5 + 0x0906u, *a1_io);
    }
    Obj_11C32C(m, obj);
}

/* ROM 0x11DEAA: pick base colour +0x4F from 0x11DE76 or 0x11DE8E. */
static void sub_11DEAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x0011DE76u;                           /* 0x11DEAA */
    uint16_t d0w = rd16(m, obj + 0x40u);                 /* 0x11DEAE */
    if ((int16_t)d0w >= 0x000C) {                        /* 0x11DEB2..0x11DEB6 */
        a1 = 0x0011DE8Eu;                                /* 0x11DEBA */
        d0w = 0u;                                        /* 0x11DEBE */
    }
    d0w = (uint16_t)(d0w + rd16(m, F3_A5 + 0x3258u));    /* 0x11DEC0 */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + (uint32_t)(int32_t)(int16_t)d0w)); /* 0x11DEC4 */
    r->d[0] = setw(r->d[0], d0w);
    r->a[1] = a1;
}

/* ROM 0x11B09A: compute screen position, draw sprite if visible, restore offset. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, d0w);
    d0w = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, d0w);
    r->d[0] = setw(r->d[0], d0w);

    if ((int16_t)rd16(m, obj + 0x18u) < 0x00E8 && (int16_t)rd16(m, obj + 0x14u) < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->d[0] = setw(r->d[0], (uint16_t)h);
        if ((int16_t)(h + (int16_t)rd16(m, obj + 0x18u)) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->d[0] = setw(r->d[0], (uint16_t)w);
            r->a[1] = a1;
            if ((int16_t)(w + (int16_t)rd16(m, obj + 0x14u)) > 0)
                draw_sprite_regs(m, r);
        } else {
            r->a[1] = a1;
        }
    }

    uint16_t sx = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - sx));
    uint16_t sy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy));
    r->d[0] = setw(r->d[0], sy);
}

int Fn_11DBBA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {             /* 0x11DBBA..0x11DBBE */
        uint32_t a1 = rd32(m, obj + 0x08u);              /* 0x11DBC2 */
        r->a[1] = a1;
        if (rd8(m, a1 + 0x59u) != 0u) {                  /* 0x11DBC6 */
            Obj_Free(m, obj);                            /* 0x11DC60 */
            return 0;
        }
        a1 = rd32(m, obj + 0x0Cu);                       /* 0x11DBCE */
        r->a[1] = a1;
        Obj_Animate(m, obj, a1);                         /* 0x11DBD2 */
        r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
        r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
        sub_11B06E(m, obj, &r->a[1]);                    /* 0x11DBD8 */
        sub_11DEAA(m, r);                                /* 0x11DBDE */
        sub_11B09A(m, r);                                /* 0x11DBE2 */
        wr32(m, obj, 0x0011DBBAu);                       /* 0x11DBB2 */
        return 0;
    }

    /* 0x11DBEA..0x11DC34 -> 0x11B102..0x11B12A */
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x11DBF2 */
    uint32_t arg = rd32(m, a1 - 8u);                     /* 0x11DBF6 */
    (void)Obj_Spawn(m, obj, 0x0011B1BEu, 0u, 1u, arg);   /* 0x11DC08 */
    (void)Obj_Spawn(m, obj, 0x0011BF5Cu, 5u, 2u, arg);   /* 0x11DC24 */
    wr32(m, obj + 0x10u, rd32(m, a1 - 4u));              /* 0x11DC2E */

    a1 = Obj_0CBDDE(m, obj);                             /* 0x11B102 */
    r->a[1] = a1;
    uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);       /* 0x11B108..0x11B10A */
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + pts);      /* 0x11B10E */
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x11B112 */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));            /* 0x11B11A */
    wr8(m, obj + 0x35u, 0u);                             /* 0x11B120 */
    wr32(m, obj, 0x0011B12Cu);                           /* 0x11B124 */
    r->d[0] = pts;
    return 0;
}
