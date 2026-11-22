#include "obj_DA28C.h"
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
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
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
    d0 = sub_00649A(m, dx, dy);
    d0 = (uint16_t)(d0 << 2);
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
    d0 = sub_006422(m, dx, dy);
    d0 = (uint16_t)(d0 << 8);
    wr16(m, obj + 0x2Eu, d0);
    wr16(m, obj + 0x30u, 0x0100u);

    if (!(rd8(m, F3_A5 + 0x1D58u) & 0x02u)) {
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

    uint16_t rad_fp = rd16(m, obj + 0x2Cu);
    rad_fp = (uint16_t)(rad_fp - (uint16_t)(rad_fp >> 5));
    wr16(m, obj + 0x2Cu, rad_fp);

    uint32_t d1_l = 0x0100u * (uint32_t)rad_fp;
    int32_t d1_s = ASR32(d1_l, 8u);
    d1_s = ASR32(d1_s, 6u);
    uint16_t d1w = (uint16_t)((uint16_t)(uint32_t)d1_s + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t d2w = d1w;

    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    uint32_t x32 = (uint32_t)((int32_t)(int16_t)d1w * (int32_t)cos_v) + 0x00A00000u;
    wr32(m, obj + 0x14u, x32);

    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    uint32_t y32 = (uint32_t)((int32_t)(int16_t)d2w * (int32_t)sin_v) + 0x00740000u;
    wr32(m, obj + 0x18u, y32);

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

/* bomb active and on screen: bomb-scatter tail (jmp 0xCBB2C) */
static int bombed(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return (rd8(m, F3_A5 + 0x1D58u) & 0x02u) && x >= -0x0A && x <= 0x011C;
}


/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
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
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
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

static void addw(const f3mem_t *m, uint32_t a, uint16_t v)
{
    wr16(m, a, (uint16_t)(rd16(m, a) + v));
}

static void hitbox(const f3mem_t *m, uint32_t obj, uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    wr16(m, obj + 0x2Cu, a);
    wr16(m, obj + 0x2Eu, b);
    wr16(m, obj + 0x30u, c);
    wr16(m, obj + 0x32u, d);
}

/* |v - 9|-style distance: addi.w #k; bpl; neg.w */
static uint16_t absw(uint16_t v)
{
    return (int16_t)v < 0 ? (uint16_t)(0u - v) : v;
}

/* Nearest live player within +0x60 horizontally (P1 0x41B190, P2 0x41B21A);
 * 0 if none */
static uint32_t player_near_x(const f3mem_t *m, uint32_t obj)
{
    static const uint32_t pl[2] = { 0x0041B190u, 0x0041B21Au };
    for (int i = 0; i < 2; i++) {
        if ((int16_t)rd16(m, pl[i] + 0x4Cu) < 0)
            continue;
        uint16_t d = absw((uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl[i] + 0x14u) - 9u));
        if (d <= rd16(m, obj + 0x60u))
            return pl[i];
    }
    return 0u;
}

static int set_vel(const f3mem_t *m, uint32_t obj, uint16_t angle)
{
    uint32_t vx = 0u, vy = 0u;
    if (sub_0D2D44(m, angle, rd16(m, obj + 0x60u), &vx, &vy) < 0)
        return -1;
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);
    return 0;
}

/* jump table 0xDA37C */
static int phase(const f3mem_t *m, uint32_t obj)
{
    switch (rd8(m, obj + 0x06u)) {
    case 0: { /* 0xDA398: wait for a player below/above */
        if (rd8(m, obj + 0x23u) != 0u)
            return 0;
        uint32_t p = player_near_x(m, obj);
        if (p == 0u)
            return 0;
        if ((uint16_t)(rd16(m, obj + 0x60u) - 0x0Au) > 50u)
            return -1;
        addw(m, obj + 0x60u, (uint16_t)-0x0A);
        set_vel(m, obj, 0xB4u);
        wr8(m, obj + 0x23u, 6u);
        wr8(m, obj + 0x06u, 4u);
        if ((int16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u) - 8u) >= 0)
            wr8(m, obj + 0x06u, 1u);
        return 0;
    }
    case 1: /* 0xDA430 */
        if (rd8(m, obj + 0x23u) != 0u)
            return 0;
        sub_0D0678(m, obj, 0x000DA5A4u);
        hitbox(m, obj, 0x0Du, 0x14u, 0x04u, 0x0Bu);
        wr8(m, obj + 0x06u, 2u);
        return 0;
    case 4: /* 0xDA462 */
        if (rd8(m, obj + 0x23u) != 0u)
            return 0;
        addw(m, obj + 0x18u, (uint16_t)-0x20);
        sub_0D0678(m, obj, 0x000DA5F8u);
        hitbox(m, obj, 0x0Du, 0x14u, 0x24u, 0x2Bu);
        wr8(m, obj + 0x06u, 5u);
        return 0;
    case 2: /* 0xDA49A */
    case 5: /* 0xDA4E8 */ {
        int up = rd8(m, obj + 0x06u) == 5u;
        if (rd16(m, obj + 0x68u) != 0u)
            return 0;
        if ((uint16_t)(rd16(m, obj + 0x60u) + 0x0Du) > 50u)
            return -1;
        addw(m, obj + 0x14u, 0x0Au);
        addw(m, obj + 0x18u, up ? (uint16_t)-9 : (uint16_t)-7);
        addw(m, obj + 0x60u, 0x0Du);
        if (up)
            hitbox(m, obj, 0x04u, 0x0Bu, 0x2Du, 0x34u);
        else
            hitbox(m, obj, 0x04u, 0x0Bu, 0x0Bu, 0x12u);
        set_vel(m, obj, up ? 0x10Eu : 0x5Au);
        wr8(m, obj + 0x06u, up ? 6u : 3u);
        return 0;
    }
    case 3:
    case 6:
        return 0;
    default:
        return -1;
    }
}

/* ROM 0x0DA28C: dropped bomb: falls until level with a player, then opens
 * and fires sideways */
int Obj_DA28C_Tick(const f3mem_t *m, uint32_t obj)
{
    if (bombed(m, obj)) {
        sub_0CBB2C(m, obj);
        return 0;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u)) {
        if (rd8(m, obj + 0x06u) > 4u)
            addw(m, obj + 0x18u, 0x30u);
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x06u) > 6u)
        return -1;
    if (rd8(m, obj + 0x23u) != 0u)
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x50u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
    if (phase(m, obj) < 0)
        return -1;
    int16_t x = (int16_t)rd16(m, obj + 0x14u), y = (int16_t)rd16(m, obj + 0x18u);
    if (x < -0x40 || y < -0x40 || y > 0x128) {
        Obj_StartDying(m, obj);
        return 0;
    }
    uint32_t t = 0x000DA538u + (uint32_t)rd8(m, obj + 0x06u) * 4u;
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(x + (int16_t)rd16(m, t)),
                                   (int16_t)(y + (int16_t)rd16(m, t + 2u))) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    sub_0D0690(m, obj);
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
