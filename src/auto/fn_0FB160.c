/* ROM 0x0FB160: delayed-start explosion/effect tick (A0 = object).
 * Subtracts scroll (A5+0x4224/0x4228), counts down +0x23 (drawing sprite
 * while > 0), installs behaviour 0x0FB1AE and adjusts (-8, -8) / clears +0x1D
 * when +0x23 reaches 0, then steps script (0x0D0690) and starts dying
 * (0x0CEE74) when +0x4C == 0, else draws sprite (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

#define REC_0FB1AE 0x000FB1AEu

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0FB160(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        uint32_t dx = rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - dx);
        uint32_t dy = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - dy);
        r->d[0] = dy;
    }

    if (rd8(m, obj + 0x23u) != 0u) {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t != 0u) {
            draw_sprite_regs(m, obj, r);
            return 0;
        }
        r->a[1] = REC_0FB1AE;
        Obj_StartBehaviour(m, obj, REC_0FB1AE);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
        wr8(m, obj + 0x1Du, 0u);
    }

    step_script_regs(m, obj, &r->d[0], &r->a[1]);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    draw_sprite_regs(m, obj, r);
    return 0;
}
