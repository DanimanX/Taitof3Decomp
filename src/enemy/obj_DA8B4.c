#include "obj_DA8B4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x00649A */
static uint16_t sub_00649A(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)d0_in * (int32_t)(int16_t)d0_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)d1_in * (int32_t)(int16_t)d1_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    d0 = (d0 << 16) | (d0 >> 16);
    return (uint16_t)d0;
}

/* ROM 0x0CBB2C */
static void sub_0CBB2C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x32u, 0x00u);
    wr8(m, obj + 0x34u, 0xFFu);

    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    int16_t off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)off));

    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)off));

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = (uint16_t)(sub_00649A(m, dx, dy) << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0 < (int16_t)thresh;
    d0 = (uint16_t)(d0 - thresh);
    if (is_lt) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    d0 = (uint16_t)(d0 << 6);
    wr16(m, obj + 0x2Cu, d0);

    dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = (uint16_t)(sub_006422(m, dx, dy) << 8);
    wr16(m, obj + 0x2Eu, d0);
    wr16(m, obj + 0x30u, 0x0100u);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));

    uint8_t sh = rd8(m, obj + 0x32u);
    if (sh != 0u) {
        uint32_t tbl = rd32(m, obj + 0x0Cu);
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        unsigned cnt = (unsigned)sh & 63u;
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
        wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)((int32_t)(int16_t)d1 * 4)));
    }

    uint16_t rad = rd16(m, obj + 0x2Cu);
    rad = (uint16_t)(rad - (uint16_t)(rad >> 5));
    wr16(m, obj + 0x2Cu, rad);

    uint32_t d1_l = 0x0100u * (uint32_t)rad;
    int32_t d1_s = ASR32(d1_l, 8u);
    d1_s = ASR32(d1_s, 6u);
    uint16_t d1w = (uint16_t)((uint16_t)(uint32_t)d1_s + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t d2w = d1w;

    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    wr32(m, obj + 0x14u, (uint32_t)((int32_t)(int16_t)d1w * (int32_t)cos_v) + 0x00A00000u);

    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    wr32(m, obj + 0x18u, (uint32_t)((int32_t)(int16_t)d2w * (int32_t)sin_v) + 0x00740000u);

    int32_t z = (int32_t)(int16_t)(-0x0100) * (int32_t)(int16_t)rd16(m, obj + 0x30u);
    z = ASR32(z, 8u);
    uint16_t zw = (uint16_t)((uint16_t)(uint32_t)z + 0x0100u);
    wr8(m, obj + 0x1Fu, (uint8_t)zw);
    wr8(m, obj + 0x1Eu, (uint8_t)zw);

    a1 = rd32(m, obj + 0x10u);
    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)off));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)off));

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CBC78u);
}

/* ROM 0x0D7306 */

/* ROM 0x0D0CB0 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    /* PC-relative table at 0x0D0CBA + 0x00C8 = 0x0D0D82 */
    return rd16(m, 0x000D0D82u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D2D44 */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    /* PC-relative tables at 0x0D2D4A + 0x0182 = 0x0D2ECC and 0x0D2D52 + 0x12 = 0x0D2D64 */
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) mod 2^32 */
    if (d2w > 50u)
        return -1;

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0DA982: rise out of the launcher (follows the parent) */
static void st0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x58u)));
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x60u)));
    uint8_t t = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u);
    wr8(m, obj + 0x1Eu, t);
    uint16_t d0 = (uint16_t)(0x10u - (uint16_t)(t >> 4));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - d0));
    wr16(m, obj + 0x32u, d0);
    if (t == 0u) {
        wr16(m, obj + 0x32u, 0x000Du);
        wr8(m, obj + 0x06u, 1u);
    }
}

static int y_near(const f3mem_t *m, uint32_t obj, uint32_t pl)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, pl + 0x18u) - 7u);
    if ((int16_t)d < 0) d = (uint16_t)(0u - d);
    return d <= 9u;
}

/* ROM 0x0DA9E4: fly forward until level with a player */
static void st1(const f3mem_t *m, uint32_t obj)
{
    uint32_t pl;
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
    uint32_t x = rd32(m, obj + 0x3Au) + (rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Au, x);
    if ((int32_t)x < 0)
        return;
    pl = 0x0041B190u;
    if ((int16_t)rd16(m, pl + 0x4Cu) < 0 || !y_near(m, obj, pl)) {
        pl = 0x0041B21Au;
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0 || !y_near(m, obj, pl))
            return;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    int16_t dx = (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, pl + 0x14u) - 0x10u);
    Obj_DrawSprite(m, obj);
    wr8(m, obj + 0x06u, dx >= 0 ? 2u : 3u);
}

/* ROM 0x0DAA88 (left, D0 = 0xB4) / 0x0DAB60 (right, D0 = 0): turn animation */
static void st_turn(const f3mem_t *m, uint32_t obj, int right)
{
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
    wr32(m, obj + 0x10u, 0x0005A588u);
    Obj_DrawSprite(m, obj);
    wr8(m, obj + 0x1Fu, (uint8_t)~rd8(m, obj + 0x1Eu));
    wr8(m, obj + 0x1Eu, 0u);
    if (right) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 7u));
    } else {
        uint16_t d0 = (uint16_t)(0x10u - (uint16_t)(rd8(m, obj + 0x1Fu) >> 4) - 0x0Au);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    }
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    if (!right)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
    wr32(m, obj + 0x10u, 0x0005A5C8u);
    Obj_DrawSprite(m, obj);
    if (!right)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & ~0x02u));
    uint8_t d0b = (uint8_t)~rd8(m, obj + 0x1Fu);
    wr8(m, obj + 0x1Eu, d0b);
    wr8(m, obj + 0x1Fu, 0u);
    if (d0b != 0xF0u)
        return;
    uint32_t vx = 0, vy = 0;
    sub_0D2D44(m, right ? 0u : 0xB4u, (uint16_t)(sub_0D0CB0(m) + 0x14u), &vx, &vy);
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);
    if (!right)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + (right ? 7u : (uint16_t)-7)));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 8u));
    wr8(m, obj + 0x1Eu, 0u);
    wr8(m, obj + 0x06u, 4u);
    wr16(m, obj + 0x2Cu, 0x0002u);
    wr16(m, obj + 0x2Eu, 0x000Du);
    wr16(m, obj + 0x30u, 0x0006u);
    wr16(m, obj + 0x32u, 0x0009u);
}

/* ROM 0x0DA8B4: homing missile */
int Obj_DA8B4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            sub_0CBB2C(m, obj);
            return 0;
        }
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    uint8_t st = rd8(m, obj + 0x06u);
    if (st > 4u)
        return -1;
    if ((st == 2u || st == 3u) && (uint16_t)(sub_0D0CB0(m) + 0x14u) > 50u)
        return -1;
    switch (st) {
    case 0u: st0(m, obj); break;
    case 1u: st1(m, obj); break;
    case 2u: st_turn(m, obj, 0); break;
    case 3u: st_turn(m, obj, 1); break;
    case 4u:   /* ROM 0x0DAC14 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
        break;
    }
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x3Au) + 7u),
                                   (int16_t)(rd16(m, obj + 0x3Eu) + 8u)) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Coll_AddHazard(m, obj);
    st = rd8(m, obj + 0x06u);
    if (st != 2u && st != 3u)
        Obj_DrawSprite(m, obj);
    return 0;
}
