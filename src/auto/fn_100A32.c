/* ROM 0x100A32: spawn-position fixup + one-shot sprite tick (A0 = object).
 * 0x0CEC4C sets the mover up from 0x1086FC; the word at +0x10 indexes the
 * 8-byte {dx.w, dy.w, +0x58.w, +0x60.w} table at 0x100B00 (indexed EA
 * 0x100A40: base 0x100A42 + 0x00BE, scale 8), offsetting +0x14/+0x18.
 * 0x0D0678 installs animation 0x100B28, then the scroll offsets A5+0x4224 /
 * A5+0x4228 are subtracted from the position, the sprite is drawn (0x001282),
 * sound 0x24 is played hooked (0x13F238 + 0x39D8) and the tick is re-armed to
 * the bare rts 0x100A98. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"      /* Obj_InitMover, Obj_SetAnim, Sound_PlayHooked */
#include "../video/sprite.h"          /* Obj_DrawSprite */

#define TBL_100B00 0x00100B00u
#define ANM_100B28 0x00100B28u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

int Fn_100A32(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1;

    Obj_InitMover(m, obj, 0x001086FCu);                 /* 0x100A32 lea (0x7cc8,PC) */
    d0 = setw(d0, rd16(m, obj + 0x10u));                /* 0x100A3C */
    a1 = TBL_100B00 + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 8u; /* 0x100A40 */
    d0 = setw(d0, rd16(m, a1));                         /* 0x100A48 */
    a1 += 2u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x100A4A */
    d0 = setw(d0, rd16(m, a1));                         /* 0x100A4E */
    a1 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x100A50 */
    wr16(m, obj + 0x58u, rd16(m, a1));                  /* 0x100A54 */
    a1 += 2u;
    wr16(m, obj + 0x60u, rd16(m, a1));                  /* 0x100A58 */

    Obj_SetAnim(m, obj, ANM_100B28);                    /* 0x100A60 lea (0xca,PC) */

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x100A66 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x100A6A */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x100A6E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x100A72 */

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x100A76 */
    Sound_PlayHooked(m, 0x0024u);                       /* 0x100A7C..0x100A8C */

    wr32(m, obj + 0x00u, 0x00100A98u);                  /* 0x100A90 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
