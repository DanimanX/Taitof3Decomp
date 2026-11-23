#include "obj_DA28C.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

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
        Obj_ExplodeTick(m, obj);
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
