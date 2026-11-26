/* ROM 0x120D30: homing/converging sub-object tick (A0 = object).
 * Frees self (0x0105EC) if A5-0x144B bit 4 is set or within +-16 px of
 * (A5+0x30D2 - 0x1C, A5+0x30D6 - 0x1A). Otherwise computes angle toward that
 * target via 0x006422, moves along it via 0x0CBE12, draws sprite via 0x001282,
 * and re-arms 0x120D30. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle]. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * angle);
    uint32_t dx = (uint32_t)((int32_t)speed * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
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

int Fn_120D30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {    /* 0x120D30..0x120D36 */
        Obj_Free(m, obj);                               /* 0x120D8A */
        return 0;
    }

    int16_t d0w = (int16_t)(rd16(m, F3_A5 + 0x30D2u) - rd16(m, obj + 0x14u) - 0x001Cu); /* 0x120D3A..0x120D42 */
    int16_t d1w = (int16_t)(rd16(m, F3_A5 + 0x30D6u) - rd16(m, obj + 0x18u) - 0x001Au); /* 0x120D46..0x120D4E */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->d[1] = setw(r->d[1], (uint16_t)d1w);

    if (d0w >= -0x0010 && d0w <= 0x0010 &&              /* 0x120D52..0x120D6E */
        d1w >= -0x0010 && d1w <= 0x0010) {
        Obj_Free(m, obj);                               /* 0x120D8A */
        return 0;
    }

    uint16_t ang = Obj_Atan2(m, (uint16_t)d0w, (uint16_t)d1w); /* 0x120D76 */
    r->d[0] = setw(r->d[0], ang);
    move_by_angle_regs(m, r, ang);                      /* 0x120D7C */
    draw_sprite_regs(m, r);                             /* 0x120D82 */
    wr32(m, obj, 0x00120D30u);                          /* 0x120D88 -> 0x120D28 */
    return 0;
}
