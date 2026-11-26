/* ROM 0x13BD74: homing missile launch-sequence animation tick (A0 = object).
 * Pops next sprite pointer from list (+0x0C) into +0x10, draws sprite via 0x001282,
 * and decrements +0x06. While > 0 re-arms 0x13BD74; once <= 0 stores target A5+0x1D28
 * into +0x0C, clears +0x3A and +0x22, copies +0x20 -> +0x3C, offsets (+0x14, +0x18)
 * by (-8, -8), sets +0x06 = 0x28, and transitions to 0x13BDBC. */
#include "fn_auto.h"
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
    *a1_io = rd32(m, SPR_CURSOR);

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

int Fn_13BD74(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    uint32_t a1 = rd32(m, obj + 0x0Cu);                     /* 0x13BD74 */
    wr32(m, obj + 0x10u, rd32(m, a1));                      /* 0x13BD78 */
    a1 += 4u;
    wr32(m, obj + 0x0Cu, a1);                               /* 0x13BD7C */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);                /* 0x13BD80 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x13BD86 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x13BD8A */
        wr32(m, obj, 0x0013BD74u);                          /* 0x13BD6C */
    } else {
        wr32(m, obj + 0x0Cu, rd32(m, F3_A5 + 0x1D28u));     /* 0x13BD8C */
        wr16(m, obj + 0x3Au, 0u);                           /* 0x13BD92 */
        wr16(m, obj + 0x3Cu, rd16(m, obj + 0x20u));         /* 0x13BD96 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u)); /* 0x13BD9C */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u)); /* 0x13BDA2 */
        wr16(m, obj + 0x22u, 0u);                           /* 0x13BDA8 */
        wr16(m, obj + 0x06u, 0x0028u);                      /* 0x13BDAE */
        wr32(m, obj, 0x0013BDBCu);                          /* 0x13BDB4 */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
