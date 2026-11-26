/* ROM 0x0FB1DE: laser/beam segment initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0F824C (0x0CEDE4), installs animation from
 * 0x0FB274[+0x58] (0x0D0090), applies scroll (A5+0x4224, A5+0x4228), sets
 * +0x1D = 0x84, draws sprite (0x001282), plays sound 0x12 (0x13F238+0x0039D8),
 * and arms 0x0FB22E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0090: install animation record from base + table 0x0D00B4[d0]. */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    int16_t off = (int16_t)rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)d0w * 2u);
    uint32_t ptr = rd32(m, (uint32_t)((int32_t)a1 + (int32_t)off));
    wr32(m, obj + 0x48u, ptr);
    wr16(m, obj + 0x4Cu, rd16(m, ptr));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, ptr + 2u));
    wr32(m, obj + 0x10u, rd32(m, ptr + 4u));
    wr32(m, obj + 0x68u, rd32(m, ptr + 8u));
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

int Fn_0FB1DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    Obj_0CEDE4(m, obj, 0x000F824Cu);                    /* 0x0FB1DE..0x0FB1E2 */
    a1 = 0x000FB274u;                                   /* 0x0FB1E8 */
    d0 = setw(d0, rd16(m, obj + 0x58u));                /* 0x0FB1EC */
    sub_0D0090(m, obj, a1, (uint16_t)d0);               /* 0x0FB1F0 */

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0FB1F6 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0FB1FA */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0FB1FE */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0FB202 */
    wr8(m, obj + 0x1Du, 0x84u);                         /* 0x0FB206 */

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0FB20C */
    Sound_PlayHooked(m, 0x0012u);                       /* 0x0FB212..0x0FB222 */
    wr32(m, obj, 0x000FB22Eu);                          /* 0x0FB226 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
