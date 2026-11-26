/* ROM 0x13C2D4: boss/enemy sub-part spawn-in animation and activation tick (A0 = object).
 * Steps 6-byte animation table at 0x13C5C0 via 0x0CBD24. While table entry >= 0,
 * positions relative to parent (+0x08) + (+0x50, +0x52) with flip-dependent
 * sprite-height Y adjustment when +0x21 < 0, draws sprite (0x001282), and
 * re-arms 0x13C2D4. When entry < 0, initializes hitboxes/HP (+0x4C=1000, +0x28=1,
 * +0x24=20, +0x34|=2, +0x10=0x3BB5C, +0x06=30) and transitions to 0x13C3AA. */
#include "fn_auto.h"
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

int Fn_13C2D4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t tbl = 0x0013C5C0u;                         /* 0x13C2D4: lea (0x2ea,PC),A1 */
    r->a[1] = tbl;

    sub_0CBD24(m, obj, tbl, &r->d[0], &r->d[1]);        /* 0x13C2D8 */

    int16_t d0_idx = (int16_t)(uint16_t)r->d[0];
    if ((int16_t)rd16(m, tbl + (uint32_t)(int32_t)d0_idx) < 0) { /* 0x13C2DE */
        /* 0x13C334 */
        wr8(m, obj + 0x1Cu, 0u);
        wr16(m, obj + 0x4Cu, 0x03E8u);
        wr8(m, obj + 0x4Fu, 0u);

        uint32_t a1 = rd32(m, obj + 0x08u);
        r->a[1] = a1;
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));

        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x50u) - 0x0010u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x52u) - 0x0010u);
        r->d[0] = setw(r->d[0], d0w);
        r->d[1] = setw(r->d[1], d1w);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d1w));

        wr16(m, obj + 0x2Cu, 0x000Bu);
        wr16(m, obj + 0x30u, 0x000Bu);
        wr16(m, obj + 0x2Eu, 0x0014u);
        wr16(m, obj + 0x32u, 0x0014u);
        wr16(m, obj + 0x28u, 0x0001u);
        wr16(m, obj + 0x24u, 0x0014u);
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr32(m, obj + 0x10u, 0x0003BB5Cu);
        wr16(m, obj + 0x06u, 0x001Eu);
        wr32(m, obj + 0x00u, 0x0013C3AAu);
        return 0;
    }

    /* 0x13C2E6 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        r->a[1] = a1;
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    }

    uint16_t d0w = rd16(m, obj + 0x50u);                /* 0x13C2F6 */
    uint16_t d1w = rd16(m, obj + 0x52u);                /* 0x13C2FA */
    if ((int8_t)rd8(m, obj + 0x21u) < 0) {              /* 0x13C2FE */
        wr8(m, obj + 0x1Cu, 0u);                        /* 0x13C310 */
        uint32_t a1 = rd32(m, obj + 0x10u);
        r->a[1] = a1;
        uint16_t d2w = (uint16_t)(rd16(m, a1) << 4);
        r->d[2] = setw(r->d[2], d2w);
        d1w = (uint16_t)(d1w - d2w);
    } else {
        wr8(m, obj + 0x1Cu, 2u);                        /* 0x13C306 */
    }

    d0w = (uint16_t)(d0w - 8u);                         /* 0x13C320 */
    r->d[0] = setw(r->d[0], d0w);
    r->d[1] = setw(r->d[1], d1w);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d1w));

    draw_sprite_regs(m, r);                             /* 0x13C32C */
    wr32(m, obj + 0x00u, 0x0013C2D4u);                  /* 0x13C332 -> 0x13C2CC */
    return 0;
}
