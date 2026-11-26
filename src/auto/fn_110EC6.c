/* ROM 0x110EC6: multi-part boss/enemy controller tick (A0 = object, A0+0x08 = parent).
 * Frees itself (clearing bit (A0+0x0C)+2 in A5+0x1D5E) if parent+0x06 == 0.
 * When parent+0x06 == 3 and +0x06 != 2, transitions to state 2 and queues palette
 * ramp 0x00183C. Tracks parent position + (+0x4C, 0xA3), steps script 0xCF1E4
 * and movement 0xCF4B8, records +0x58 into the ring buffer at (A0+0x0C)+4, runs
 * state handler (0x110FF6 for state 1), draws active children linked via +0x54,
 * and optionally draws grandparent when +0x4E == 2 and grandparent+0x06 != 3. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
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

/* ROM 0x110FF6: state 1 handler (called via jump table 0x110FEA[1]). */
static void sub_110FF6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t c = rd8(m, obj + 0x23u);
    if (c != 0u) {                                      /* 0x110FF6..0x110FFA */
        wr8(m, obj + 0x23u, (uint8_t)(c - 1u));         /* 0x110FFE */
    } else if (rd16(m, obj + 0x48u) != 0x000Eu) {       /* 0x111006..0x11100C */
        wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x4E20u); /* 0x111010 */
    }

    wr8(m, obj + 0x1Du, 0u);                            /* 0x111018 */
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x11101C */
    r->a[1] = a1;
    int8_t v20 = (int8_t)rd8(m, a1 + 0x20u);            /* 0x111020 */
    if (v20 > 0)
        wr8(m, obj + 0x1Du, 0x40u);                     /* 0x11102C */
    else if (v20 < 0)
        wr8(m, obj + 0x1Du, 0x42u);                     /* 0x111036 */
}

int Fn_110EC6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x110EC6 */
    r->a[1] = a1;
    uint8_t d0b = rd8(m, a1 + 0x06u);                   /* 0x110ECA */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | d0b;

    if (d0b == 0u) {                                    /* 0x110ECE -> 0x110FD4 */
        uint32_t rec = rd32(m, obj + 0x0Cu);            /* 0x110FD4 */
        r->a[1] = rec;
        uint8_t bit = rd8(m, rec + 0x02u);              /* 0x110FD8 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | bit;
        wr8(m, F3_A5 + 0x1D5Eu, (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) & ~(1u << (bit & 7u)))); /* 0x110FDC */
        wr8(m, obj + 0x06u, 0u);                        /* 0x110FE0 */
        Obj_StartDying(m, obj);                         /* 0x110FE4 */
        return 0;
    }

    if (rd8(m, obj + 0x06u) != 2u && d0b == 3u) {       /* 0x110ED2..0x110EE0 */
        wr8(m, obj + 0x1Du, 0u);                        /* 0x110EE4 */
        wr8(m, obj + 0x06u, 2u);                        /* 0x110EE8 */
        r->d[0] = Pal_QueueRamp(m, 0x00017D00u, 0x00110B9Cu, 0x00005900u, 0x0000003Cu, 0x00000020u); /* 0x110F0C */
    }

    /* 0x110F16 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    uint16_t off_x = rd16(m, obj + 0x4Cu);              /* 0x110F1C */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + off_x));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x00A3u));

    /* 0x110F30: bclr #1,(0x7,A0) */
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x02u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)                /* 0x110F3A */
            return -1;
    }

    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x110F40..0x110F44 */
        if (ObjScript_CF4B8(m, obj, 0x0010C8E2u) < 0)   /* 0x110F4C */
            return -1;
        uint8_t op = rd8(m, obj + 0x20u);
        if (op >= 12u && op <= 15u) {
            uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x58u) * 4u;
            uint32_t vy = rd32(m, 0x000D2ECCu + off) * (uint32_t)rd16(m, obj + 0x60u);
            r->d[1] = vy;
        }
    }

    /* 0x110F52 */
    a1 = rd32(m, obj + 0x0Cu);
    r->a[1] = a1;
    uint16_t cursor = rd16(m, a1);                      /* 0x110F56 */
    uint32_t ring = rd32(m, a1 + 0x04u);
    wr16(m, ring + (uint32_t)(int32_t)(int16_t)cursor, rd16(m, obj + 0x58u)); /* 0x110F58 */
    cursor = (uint16_t)((cursor + 2u) & 0x01FFu);       /* 0x110F60..0x110F62 */
    wr16(m, a1, cursor);                                /* 0x110F66 */

    /* 0x110F68..0x110F6E: jump table 0x110FEA[state] */
    uint8_t st = rd8(m, obj + 0x06u);
    r->d[0] = setw(r->d[0], st);
    if (st == 1u) {
        sub_110FF6(m, r);
    } else if (st != 2u) {
        return -1;
    }

    /* 0x110F76: walk linked list via +0x54 */
    uint32_t d0 = rd32(m, obj + 0x54u);
    do {
        r->a[0] = d0;                                   /* 0x110F7A */
        if (rd8(m, r->a[0] + 0x06u) == 0u) {            /* 0x110F7C..0x110F80 */
            wr32(m, r->a[0] + 0x18u, rd32(m, r->a[0] + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x110F84..0x110F88 */
            draw_sprite_regs(m, r);                     /* 0x110F8C */
        }
        d0 = rd32(m, r->a[0] + 0x54u);                  /* 0x110F92 */
        r->d[0] = d0;
    } while ((int32_t)d0 >= 0);                         /* 0x110F96 */

    /* 0x110F98 */
    r->a[0] = rd32(m, r->a[0] + 0x08u);
    if (rd8(m, r->a[0] + 0x4Eu) == 2u) {                /* 0x110F9C..0x110FA2 */
        r->a[0] = rd32(m, r->a[0] + 0x08u);             /* 0x110FA6 */
        if (rd8(m, r->a[0] + 0x06u) != 3u) {            /* 0x110FAA..0x110FB0 */
            wr16(m, r->a[0] + 0x14u, (uint16_t)(rd16(m, r->a[0] + 0x14u) + 0x0060u)); /* 0x110FB4 */
            wr16(m, r->a[0] + 0x18u, (uint16_t)(rd16(m, r->a[0] + 0x18u) + 0x0070u)); /* 0x110FBA */
            draw_sprite_regs(m, r);                     /* 0x110FC0 */
            wr16(m, r->a[0] + 0x14u, (uint16_t)(rd16(m, r->a[0] + 0x14u) - 0x0060u)); /* 0x110FC6 */
            wr16(m, r->a[0] + 0x18u, (uint16_t)(rd16(m, r->a[0] + 0x18u) - 0x0070u)); /* 0x110FCC */
        }
    }
    return 0;                                           /* 0x110FD2 */
}
