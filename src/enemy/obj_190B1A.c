#include "obj_190B1A.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CC2BC: frame-rate scaled phase modulo */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    uint16_t div = (uint16_t)(prod >> 16);
    if (div == 0u)
        div = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % div);
}

/* ROM 0x17B92C: relay accumulated damage to parent (+0x08) */
static void sub_17B92C(const f3mem_t *m, uint32_t obj)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 0u)
        return;

    int32_t dmg = (int32_t)rd32(m, obj + 0x24u);
    if (hit == 2u) {
        dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Eu);
    } else {
        uint32_t a1 = rd32(m, obj + 0x36u);
        uint8_t k = rd8(m, a1 + 0x22u);
        if (k == 1u)
            dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Au);
        else if (k == 2u)
            dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Cu);
    }

    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, a4 + 0x24u, rd32(m, a4 + 0x24u) + (uint32_t)dmg);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
    wr16(m, a4 + 0x34u, rd16(m, obj + 0x34u));
}

/* ROM 0x17BA76: compute hit-spark position on hitbox overlap */
static int sub_17BA76(const f3mem_t *m, uint32_t obj, int16_t *out_x, int16_t *out_y)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if ((uint16_t)(rd16(m, F3_A5 + 0x0878u) % 3u) != 0u)
            return -1;
        int16_t hw = (int16_t)((int16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) >> 1);
        int16_t hh = (int16_t)((int16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)) >> 1);
        *out_y = (int16_t)((int16_t)Obj_RandRange(m, (uint16_t)hh) +
                           (int16_t)rd16(m, obj + 0x18u) +
                           (int16_t)rd16(m, obj + 0x30u) + hh);
        *out_x = (int16_t)((int16_t)Obj_RandRange(m, (uint16_t)hw) +
                           (int16_t)rd16(m, obj + 0x14u) +
                           (int16_t)rd16(m, obj + 0x2Cu) + hw);
        return 0;
    }

    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u && (rd16(m, F3_A5 + 0x0878u) & 3u) != 0u)
        return -1;

    int16_t ax0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t ay0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t bx0 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t by0 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t x0 = (ax0 >= bx0) ? ax0 : bx0;
    int16_t y0 = (ay0 >= by0) ? ay0 : by0;

    int16_t ax1 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    int16_t ay1 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    int16_t bx1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    int16_t by1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t x1 = (ax1 <= bx1) ? ax1 : bx1;
    int16_t y1 = (ay1 <= by1) ? ay1 : by1;

    *out_x = (int16_t)(x0 + (int16_t)((uint16_t)(x1 - x0) >> 1));
    *out_y = (int16_t)(y0 + (int16_t)((uint16_t)(y1 - y0) >> 1));
    return 0;
}

/* ROM 0x17BA3C: spawn hit-spark object 0x17A016 when hit */
static void sub_17BA3C(const f3mem_t *m, uint32_t obj)
{
    int16_t hx, hy;
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (sub_17BA76(m, obj, &hx, &hy) < 0)
        return;
    uint16_t dx = (uint16_t)(hx - (int16_t)rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(hy - (int16_t)rd16(m, obj + 0x54u));
    Obj_Spawn(m, obj, 0x0017A016u, 6u, 1u, ((uint32_t)dx << 16) | dy);
}

/* ROM 0x193E6C: knock back the object the parent hit (+0x36) when armed */
static void sub_193E6C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 1u)
        return;
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k == 0u || k == 4u || k == 5u || k == 6u)
        return;
    int32_t vx = (int32_t)rd32(m, a1 + 0x48u);
    if (vx >= 0)
        wr32(m, a1 + 0x48u, (uint32_t)(-vx));
    uint16_t r = Obj_RandRange(m, 4u);
    wr16(m, a1 + 0x4Cu, (uint16_t)(rd16(m, a1 + 0x4Cu) + r));
    wr8(m, a1 + 0x1Cu, (uint8_t)(rd8(m, a1 + 0x1Cu) | 0x01u));
    wr8(m, a1 + 0x1Du, 4u);
    wr32(m, a1 + 0x28u, 0u);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0F));
}

/* ROM 0x17BD1A: boss shake offset table */
static void sub_17BD1A(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u)
        return;
    uint8_t d0 = (uint8_t)(((rd16(m, F3_A5 + 0x0878u) >> 3) + rd8(m, obj + 0x22u)) & 0x07u);
    uint32_t tbl = 0x0017BD48u + (uint32_t)d0 * 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, tbl)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, tbl + 2u)));
}

/* ROM 0x190DBC: boss part attack / projectile spawner */
static void sub_190DBC(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x22u);
    if (k == 0u)
        return;
    if (k == 0x0Au) {
        if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x80u) != 0u) {
            Obj_Spawn(m, obj, 0x00183EDAu, 9u, 2u, 0x3Cu);
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x80u));
        }
        return;
    }

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x40u) != 0u) {
        uint8_t arg = (uint8_t)(rd8(m, obj + 0x20u) - 0x10u);
        Obj_Spawn(m, obj, 0x00181AECu, 3u, 2u, (uint32_t)arg);
        if (k == 9u)
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x40u));
    }

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x04u) == 0u) {
        wr16(m, obj + 0x58u, 0u);
        return;
    }

    if (sub_0CC2BC(m, 7u) != 0u)
        return;

    int16_t phase = (int16_t)rd16(m, F3_A5 + 0x4B90u);
    uint32_t tbl = rd32(m, (uint32_t)(0x00190EE6 + (int32_t)phase * 4));
    int16_t step = (int16_t)rd16(m, obj + 0x58u);
    uint16_t mask = rd16(m, (uint32_t)(tbl + (int32_t)step * 2));
    if ((mask & (1u << (k & 0x1Fu))) != 0u) {
        uint16_t d1 = (uint8_t)(rd8(m, obj + 0x20u) - 0x40u);
        if ((rd8(m, F3_A5 + 0x4B3Au) & 0x02u) != 0u) {
            uint8_t ang = Obj_AimAtPlayer17AF9A(m, rd32(m, F3_A5 + 0x1D28u),
                                                rd16(m, obj + 0x50u),
                                                rd16(m, obj + 0x54u));
            d1 = (uint16_t)((uint16_t)ang + (uint16_t)((int16_t)(k - 5) * 4));
        }
        Obj_Spawn(m, obj, 0x0018048Au, 9u, 2u, (uint32_t)d1);
    }

    uint16_t next58 = (uint16_t)(rd16(m, obj + 0x58u) + 1u);
    wr16(m, obj + 0x58u, next58);
    if ((int16_t)next58 >= 0x0C)
        wr16(m, obj + 0x58u, 0u);
}

/* ROM 0x190B1A: boss main body tick */
int Obj_190B1A_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Free(m, obj);
        return 0;
    }

    sub_17B92C(m, obj);
    if (rd16(m, F3_A5 + 0x4B82u) != 0u)
        sub_193E6C(m, obj);
    else
        sub_17BA3C(m, obj);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u)
        Obj_0D0DC2(m, obj);

    if (rd8(m, obj + 0x22u) == 0x0Au) {
        if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
            uint32_t a1 = 0x00191928u;
            uint16_t idx = rd16(m, a1);
            wr16(m, obj + 0x40u, idx);
            wr32(m, obj + 0x3Au, a1 + 2u);
            if (Obj_AnimInit(m, obj, idx) < 0)
                return -1;
        } else if (rd8(m, obj + 0x20u) == 0u &&
                   (rd8(m, F3_A5 + 0x4B3Au) & 0x10u) != 0u) {
            wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & ~0x10u));
            uint32_t a1 = 0x00191B2Au;
            uint16_t idx = rd16(m, a1);
            wr16(m, obj + 0x40u, idx);
            wr32(m, obj + 0x3Au, a1 + 2u);
            if (Obj_AnimInit(m, obj, idx) < 0)
                return -1;
            Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x2B));
        }
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);
    uint16_t dy = (uint16_t)(rd16(m, spr) << 3);
    uint16_t dx = (uint16_t)(rd16(m, spr + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));

    sub_17BD1A(m, obj);
    wr16(m, obj + 0x5Eu, rd16(m, 0x0040CB80u));
    sub_190DBC(m, obj);

    wr8(m, F3_A5 + 0x4B36u, 0u);
    uint8_t pal_hit = rd8(m, F3_A5 + 0x4B28u);
    if (rd8(m, obj + 0x1Du) == pal_hit)
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, pal_hit);
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));

    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) == 0u) {
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (y >= 0x00BA && y <= 0x012C && Obj_RandRange(m, 8u) == 0u)
            Obj_Spawn(m, obj, 0x00191486u, 6u, 1u, 0x20000000u);
    }

    wr32(m, obj + 0x00u, 0x00190B1Au);
    return 0;
}
