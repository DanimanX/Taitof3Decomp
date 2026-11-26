/* ROM 0x10F786: falling terrain-colliding projectile/drop tick (A0 = object).
 * Integrates (+0x50, +0x54) minus scroll (A5+0x4224, A5+0x4228) into
 * (+0x14, +0x18). Calls Obj_StartDying (0x0CEE74) if Y > 0xE8 or terrain
 * at (X+8, Y+8) is solid (0x0C6632). Otherwise steps 12-byte script via
 * 0x0D0690, draws sprite (0x001282), and adds gravity 0xFA0 to +0x54. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0C6632 with exact D0/D1 register outputs. */
static void terrain_test_regs(const f3mem_t *m, int16_t x, int16_t y,
                              uint32_t *d0_io, uint32_t *d1_io)
{
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u);
        return;
    }
    uint16_t cx = (uint16_t)((uint16_t)(x + (int16_t)rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x03FFu;
    uint16_t cy = (uint16_t)((uint16_t)(y + (int16_t)rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x003Fu;
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)cy * 0x0280);
    uint32_t bit = (uint32_t)cx + d1;
    int hit = Terrain_TestPoint(m, x, y);
    *d1_io = d1;
    *d0_io = (bit & 0xFFFFFF00u) | (hit ? 0xFFu : 0x00u);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_10F786(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t vx = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u); /* 0x10F786 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + vx);                /* 0x10F78E */
    uint32_t vy = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u); /* 0x10F792 */
    r->d[0] = vy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);                /* 0x10F79A */

    if ((int16_t)rd16(m, obj + 0x18u) > 0x00E8) {                   /* 0x10F79E..0x10F7A4 */
        Obj_StartDying(m, obj);                                     /* 0x0CEE74 */
        return 0;
    }

    int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 8u);              /* 0x10F7AA..0x10F7AE */
    int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + 8u);              /* 0x10F7B0..0x10F7B4 */
    r->d[0] = setw(r->d[0], (uint16_t)tx);
    r->d[1] = setw(r->d[1], (uint16_t)ty);
    terrain_test_regs(m, tx, ty, &r->d[0], &r->d[1]);               /* 0x10F7B6 */
    if ((uint8_t)r->d[0] != 0u) {                                   /* 0x10F7BC..0x10F7BE */
        Obj_StartDying(m, obj);                                     /* 0x0CEE74 */
        return 0;
    }

    Obj_StepScript(m, obj);                                         /* 0x10F7C4 */
    draw_sprite_regs(m, r);                                         /* 0x10F7CA */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x00000FA0u);       /* 0x10F7D0 */
    return 0;
}
