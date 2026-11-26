/* ROM 0x0D9622: ground-turret aimed shot initializer (A0 = object).
 * Offsets (+0x14, +0x18) by (-7, -9), seeds stats 0x0D98BE via 0x0CEDE4,
 * clears +0x34 bit 1, installs behaviour 0x0D98DE via 0x0D0678, copies
 * parent (+0x08)->+0x4F to +0x1D, sets speed = 0x19 + tbl_0D0CB0 to +0x60,
 * computes velocity vector for angle 0x87 via 0x0D2D44 into (+0x50, +0x54),
 * subtracts scroll (A5+0x4224, A5+0x4228), draws sprite (0x001282), sets
 * +0x23 = 5, plays hooked sound 0x11, and arms 0x0D96B2. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0CB0: difficulty-scaled speed bonus from table 0x0D0D7A. */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D82u + (uint32_t)idx * 2u);
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

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

int Fn_0D9622(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 7u)); /* 0x0D962A */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 9u)); /* 0x0D962E */

    a1 = 0x000D98BEu;                                   /* 0x0D9634 */
    Obj_0CEDE4(m, obj, a1);                             /* 0x0D9638 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x0D963E */

    a1 = 0x000D98DEu;                                   /* 0x0D9644 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0D9648 */

    a1 = rd32(m, obj + 0x08u);                          /* 0x0D964E */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x4Fu));            /* 0x0D9652 */

    d0 = setw(d0, (uint16_t)(sub_0D0CB0(m) + 0x0019u)); /* 0x0D9658..0x0D965E */
    wr16(m, obj + 0x60u, (uint16_t)d0);                 /* 0x0D9662 */
    d1 = setw(d1, (uint16_t)d0);                        /* 0x0D9666 */
    d0 = setw(d0, 0x0087u);                             /* 0x0D9668 */
    sub_0D2D44(m, 0x0087u, (uint16_t)d1, &d0, &d1);     /* 0x0D966C */
    wr32(m, obj + 0x50u, d0);                           /* 0x0D9672 */
    wr32(m, obj + 0x54u, d1);                           /* 0x0D9676 */

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0D967A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0D967E */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0D9682 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0D9686 */

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0D968A */
    wr8(m, obj + 0x23u, 5u);                            /* 0x0D9690 */
    Sound_PlayHooked(m, 0x0011u);                       /* 0x0D9696..0x0D96A6 */
    wr32(m, obj + 0x00u, 0x000D96B2u);                  /* 0x0D96AA */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
