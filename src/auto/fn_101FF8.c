/* ROM 0x101FF8: parent-linked sprite draw proxy (A0 = object).
 * Loads parent A1 = +0x08. If parent->+0x06.b == 0, marks self dying via
 * 0x0CEE74 (Obj_StartDying); otherwise switches A0 to parent and draws via
 * 0x001282 (Obj_DrawSprite). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
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

int Fn_101FF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);              /* 0x101FF8 */
    r->a[1] = parent;

    if (rd8(m, parent + 0x06u) == 0u) {                  /* 0x101FFC..0x102000 */
        Obj_StartDying(m, obj);                          /* 0x0CEE74 */
        return 0;
    }

    r->a[0] = parent;                                    /* 0x102006 */
    draw_sprite_regs(m, r);                              /* 0x102008 -> 0x001282 */
    return 0;                                            /* 0x10200E */
}
