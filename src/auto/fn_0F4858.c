/* ROM 0x0F4858: extending beam/arm segment tick (A0 = object).
 * If parent (+0x08)->+0x06 == 0, calls Obj_StartDying (0x0CEE74).
 * Syncs (+0x14, +0x18, +0x1D) to parent (+0x11, +0x0B), dispatches state
 * 0..3 (0x0F489E): waits for parent state 3, aims at player via 0x0D06E0
 * clamped to [0x9B..0xBE] and computes step via 0x0D1EBC, extends and
 * retracts segments drawing via 0x001282 and 0x0CFA86 + 0x0D0DCC, then
 * sets A0 = parent and draws parent (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[d0w] * d1w (via 0x0D4F26). */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *a1_io)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
    *a1_io = a1 + 6u;
}

/* ROM 0x0D06E0 player check */
static int16_t check_player_0D06E0(const f3mem_t *m, uint32_t obj, uint32_t player,
                                   uint16_t *out_dx, uint16_t *out_dy)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return -1;
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t a3w = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, player + 0x14u)
                            + (int16_t)rd16(m, a2) - 0x17);
    int16_t d2w = (a3w < 0) ? (int16_t)(-a3w) : a3w;
    int16_t a2w = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, player + 0x18u)
                            + (int16_t)rd16(m, a2 + 2u) - 0x0F);
    int16_t d3w = (a2w < 0) ? (int16_t)(-a2w) : a2w;
    *out_dx = (uint16_t)d2w;
    *out_dy = (uint16_t)d3w;
    return (int16_t)Obj_Atan256(m, a3w, a2w);
}

/* ROM 0x0D06E0: aim at nearer live player, store in +0x42 (default 0xB4). */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, 0x00B4u);
    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t d4w = check_player_0D06E0(m, obj, 0x0041B190u, &dx1, &dy1);
    int16_t d7w = check_player_0D06E0(m, obj, 0x0041B21Au, &dx2, &dy2);

    if (d4w < 0) {
        if (d7w < 0)
            return;
        wr16(m, obj + 0x42u, (uint16_t)d7w);
        return;
    }
    if (d7w >= 0) {
        uint16_t m1 = (dx1 > dy1) ? dx1 : dy1;
        uint16_t m2 = (dx2 > dy2) ? dx2 : dy2;
        if (m2 < m1) {
            wr16(m, obj + 0x42u, (uint16_t)d7w);
            return;
        }
    }
    wr16(m, obj + 0x42u, (uint16_t)d4w);
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

int Fn_0F4858(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x0F4858 */

    if (rd8(m, a1 + 0x06u) == 0u) {                     /* 0x0F485C */
        Obj_StartDying(m, obj);                         /* 0x0F4860 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));          /* 0x0F4866 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0011u)); /* 0x0F486C */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));          /* 0x0F4872 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x000Bu)); /* 0x0F4878 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));            /* 0x0F487E */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F4884..0x0F4886 */
    switch ((uint16_t)d0) {                             /* 0x0F488A -> 0x0F489E */
    case 0u:                                            /* 0x0F48AE */
        a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x06u) == 3u) {
            wr8(m, obj + 0x23u, 0x1Eu);
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 1u: {                                          /* 0x0F48CA */
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c == 0u) {
            wr32(m, obj + 0x68u, 0x000F4A2Au);
            sub_0D06E0(m, obj);
            uint16_t ang = rd16(m, obj + 0x42u);
            if (ang > 0x00BEu)
                ang = 0x00BEu;
            else if (ang < 0x009Bu)
                ang = 0x009Bu;
            sub_0D1EBC(m, ang, 4u, &d0, &d1);
            wr32(m, obj + 0x50u, d0);
            wr32(m, obj + 0x54u, d1);
            wr8(m, obj + 0x21u, 0u);
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    }
    case 2u:                                            /* 0x0F491C */
        if (rd8(m, obj + 0x21u) == 0x14u) {
            wr8(m, obj + 0x06u, 3u);
            goto state3;
        }
        {
            uint8_t v = (uint8_t)(rd8(m, obj + 0x21u) + 1u);
            wr8(m, obj + 0x21u, v);
            wr8(m, obj + 0x22u, v);
            sub_0CFA86(m, obj, 0x000F4A16u, &a1);
            for (;;) {
                uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
                wr8(m, obj + 0x22u, c);
                if ((int8_t)c < 0)
                    break;
                draw_sprite_regs(m, obj, &d0, &d1, &a1);
                wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x50u));
                wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x54u));
            }
            sub_0CFA86(m, obj, 0x000F4A20u, &a1);
            draw_sprite_regs(m, obj, &d0, &d1, &a1);
            Coll_AddHazard(m, obj);
        }
        break;
    case 3u:                                            /* 0x0F4978 */
    state3:
        if (rd8(m, obj + 0x21u) == 1u) {
            a1 = rd32(m, obj + 0x08u);
            wr8(m, a1 + 0x06u, 4u);
            wr8(m, obj + 0x06u, 0u);
        } else {
            uint8_t v = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
            wr8(m, obj + 0x21u, v);
            wr8(m, obj + 0x22u, v);
            sub_0CFA86(m, obj, 0x000F4A16u, &a1);
            for (;;) {
                uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
                wr8(m, obj + 0x22u, c);
                if ((int8_t)c < 0)
                    break;
                draw_sprite_regs(m, obj, &d0, &d1, &a1);
                wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x50u));
                wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x54u));
            }
            sub_0CFA86(m, obj, 0x000F4A20u, &a1);
            draw_sprite_regs(m, obj, &d0, &d1, &a1);
            Coll_AddHazard(m, obj);
        }
        break;
    }

    obj = rd32(m, obj + 0x08u);                         /* 0x0F4892 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F4896 */

    r->a[0] = obj;
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
