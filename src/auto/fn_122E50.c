/* ROM 0x122E50: child turret/pod object tick (A0 = object, A0+0x08 = parent).
 * Frees itself if parent+0x59 is non-zero; otherwise cycles through 4 sprite
 * frames (0x122F74[+0x3A]), copies parent+0x1D, draws via 0x11B09A, ramps
 * animation speed +0x3E up to 0x10000 after countdown +0x6, and every 6 cycles
 * spawns two pairs of child shots (0x0CC402/0x0CC756 and 0x0CDCA0) when
 * A5+0x30EE bit 4 (from MSB) is set. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values for ALLREGS. */
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

/* ROM 0x11B09A: compute screen position from +0x50/+0x54/+0x30D2 and draw
 * if visible, then subtract +0x54/+0x56 back out. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);                  /* 0x11B0A6 */
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);                  /* 0x11B0B6 */

    if (y < 0x00E8 && x < 0x0140) {                     /* 0x11B0BA..0x11B0CA */
        uint32_t a1 = rd32(m, obj + 0x10u);             /* 0x11B0CE */
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->a[1] = a1;
        if ((int16_t)(h + y) > 0) {                     /* 0x11B0D6..0x11B0DA */
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->a[1] = a1;
            if ((int16_t)(w + x) > 0)                   /* 0x11B0E2..0x11B0E6 */
                draw_sprite_regs(m, r);                 /* 0x11B0EA */
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u))); /* 0x11B0F4 */
    uint16_t d0w = rd16(m, obj + 0x56u);                /* 0x11B0F8 */
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w)); /* 0x11B0FC */
}

int Fn_122E50(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x122E50 */
    r->a[1] = rd32(m, obj + 0x08u);
    if (rd8(m, r->a[1] + 0x59u) != 0u) {                /* 0x122E54..0x122E58 */
        Obj_Free(m, obj);                               /* 0x122F68 */
        return 0;                                       /* 0x122F72 */
    }

    /* 0x122E5C: sprite table at 0x122F74 */
    r->a[1] = 0x00122F74u;
    int16_t idx = (int16_t)rd16(m, obj + 0x3Au);        /* 0x122E62 */
    wr32(m, obj + 0x10u, rd32(m, r->a[1] + (uint32_t)((int32_t)idx * 4))); /* 0x122E66 */
    uint32_t spd = rd32(m, obj + 0x3Eu);                /* 0x122E6C */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + spd);   /* 0x122E70 */
    if ((int16_t)rd16(m, obj + 0x3Au) >= 4) {           /* 0x122E74..0x122E7A */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x122E7E */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u)); /* 0x122E82 */
    }

    /* 0x122E86 */
    r->a[1] = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, rd8(m, r->a[1] + 0x1Du));       /* 0x122E8A */
    sub_11B09A(m, r);                                   /* 0x122E90 */

    /* 0x122E96 */
    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) < 6) {
        wr32(m, obj, 0x00122E50u);                      /* 0x122E48 */
        return 0;
    }

    /* 0x122E9E */
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                             /* 0x122EA2 */
        wr32(m, obj, 0x00122E50u);                      /* 0x122E48 */
        return 0;
    }

    /* 0x122EA4 */
    if ((int32_t)rd32(m, obj + 0x3Eu) < 0x10000)
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x100u); /* 0x122EB0 */

    /* 0x122EB8 */
    if ((int16_t)rd16(m, obj + 0x42u) < 6) {
        wr32(m, obj, 0x00122E50u);                      /* 0x122E48 */
        return 0;
    }

    wr16(m, obj + 0x42u, 0u);                           /* 0x122EC0 */

    /* 0x122EC4: bftst (0x30ee,A5){3:1} -> MSB bit 3 = 0x10 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x10u) != 0u) {
        uint32_t d2 = (rd16(m, F3_A5 + 0x32DCu) == 7u) ? 0x000CC402u : 0x000CC756u; /* 0x122ECE..0x122EE2 */
        uint32_t d1 = 0x0904C000u;                      /* 0x122EE8 */
        uint8_t d0b = (uint8_t)(((uint8_t)rd16(m, obj + 0x22u) << 2) + 3u); /* 0x122EEE..0x122EF4 */
        d1 = (d1 & 0xFFFFFF00u) | d0b;                  /* 0x122EF6 */

        (void)Obj_Spawn(m, obj, d2, 9u, 2u, d1);        /* 0x122F04 */
        (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, d1); /* 0x122F1E */

        d1 += 0x00180000u;                              /* 0x122F28 */
        d1 = setw(d1, (uint16_t)((uint16_t)d1 + 0x8000u)); /* 0x122F2E */
        (void)Obj_Spawn(m, obj, d2, 9u, 2u, d1);        /* 0x122F3E */
        r->d[0] = Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, d1); /* 0x122F58 */
        r->d[1] = d1;
    }

    wr32(m, obj, 0x00122E50u);                          /* 0x122F62 -> 0x122E48 */
    return 0;
}
