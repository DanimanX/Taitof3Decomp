/* ROM 0x10C242: enemy init (A0 = object), installs tick 0x10C2F4.
 * Offsets (+0x14, +0x18) by (-15, -16), seeds enemy stats (0x0CEDE4 from
 * 0x10C488) and behaviour (0x0D0678 from 0x10C4A8), sets +0x23 = 5, +0x07 bit 4,
 * +0x34 bits 1..2, randomizes heading (+0x58) and speed (+0x60) via 0x0D0E3A
 * and difficulty table 0x0D0D82 (0x0D0CB0), computes velocity (+0x50/+0x54) via
 * 0x0D2D44, subtracts scroll deltas (A5+0x4224/4228), draws sprite (0x001282),
 * and sets +0x00 = 0x10C2F4. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x001282 register outputs after Obj_DrawSprite(m, obj) */
static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, SPR_CURSOR);
    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D0CB0: difficulty speed modifier from table at 0x0D0D82 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t idx = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D82u + (uint32_t)idx * 2u);
}

/* ROM 0x0D2D44: scale (0x0D2D64[d0w], 0x0D2ECC[d0w]) by d1w (0..50) */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

int Fn_10C242(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x000Fu)); /* 0x10C242 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u)); /* 0x10C248 */

    Obj_0CEDE4(m, obj, 0x0010C488u);                     /* 0x10C24E..0x10C252 */
    Obj_StartBehaviour(m, obj, 0x0010C4A8u);             /* 0x10C258..0x10C25C */

    wr8(m, obj + 0x23u, 5u);                             /* 0x10C262 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x10C268 */
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u)); /* 0x10C26E..0x10C274 */

    {
        int16_t j = (int16_t)((Obj_Next4Way(m) & 0x003Fu) - 0x0020); /* 0x10C27A..0x10C284 */
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) + (uint16_t)j)); /* 0x10C288 */
        j = (int16_t)((Obj_Next4Way(m) & 0x001Fu) - 0x0010); /* 0x10C28C..0x10C296 */
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) + (uint16_t)j)); /* 0x10C29A */
    }

    {
        uint16_t spd = (uint16_t)(sub_0D0CB0(m) + 0x0012u); /* 0x10C29E..0x10C2A4 */
        spd = (uint16_t)(spd + (Obj_Next4Way(m) & 0x0003u)); /* 0x10C2AA..0x10C2B4 */
        wr16(m, obj + 0x60u, spd);                       /* 0x10C2B6 */

        uint32_t vx, vy;
        sub_0D2D44(m, rd16(m, obj + 0x58u), spd, &vx, &vy); /* 0x10C2BC..0x10C2C0 */
        wr32(m, obj + 0x50u, vx);                        /* 0x10C2C6 */
        wr32(m, obj + 0x54u, vy);                        /* 0x10C2CA */
    }

    {
        uint32_t sx = rd32(m, F3_A5 + 0x4224u);          /* 0x10C2CE */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - sx); /* 0x10C2D2 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - sx); /* 0x10C2D6 */

        uint32_t sy = rd32(m, F3_A5 + 0x4228u);          /* 0x10C2DA */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - sy); /* 0x10C2DE */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sy); /* 0x10C2E2 */
    }

    Obj_DrawSprite(m, obj);                              /* 0x10C2E6 */
    draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
    wr32(m, obj + 0x00u, 0x0010C2F4u);                   /* 0x10C2EC */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
