#include "obj_E6906.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBE34: flash palette on hit */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x0CFA86: load palette/sprite/ext from table */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
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

/* ROM 0x0D0690: advance object animation script */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, timer);
    if (timer != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0w < 0) {
        uint16_t off = rd16(m, a1);
        if (off == 0u)
            return;
        a1 = (uint32_t)((int32_t)a1 - (int32_t)(int16_t)off);
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0w);
    }
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2: append to enemy hurt list (A5+0x8FE) and hazard list */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, p, obj);
    wr32(m, F3_A5 + 0x8FEu, p + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04: Score_OnHit */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E3A: 4-way interleaved global counter in A5+0x1D60 */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0w = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1w = (uint16_t)(d0w << 2);
    d0w = (uint16_t)(d0w + d1w + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0w);
    return (uint16_t)(d0w >> 8);
}

/* ROM 0x0E6A0A: state 1 - wait for player alignment or random fire trigger */
static void sub_0E6A0A(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < 0x28 || (uint16_t)x > 0x013Fu)
        return;

    uint16_t y = rd16(m, obj + 0x18u);
    int fire = 0;

    if ((int16_t)rd16(m, 0x0041B190u + 0x4Cu) >= 0) {
        int16_t dy = (int16_t)(y - rd16(m, 0x0041B190u + 0x18u) + 5);
        if (dy < 0)
            dy = (int16_t)(-dy);
        if ((uint16_t)dy <= 0x000Au)
            fire = 1;
    }

    if (!fire && (int16_t)rd16(m, 0x0041B21Au + 0x4Cu) >= 0) {
        int16_t dy = (int16_t)(y - rd16(m, 0x0041B21Au + 0x18u) + 5);
        if (dy < 0)
            dy = (int16_t)(-dy);
        if ((uint16_t)dy <= 0x000Au)
            fire = 1;
    }

    if (!fire) {
        uint8_t d0b = (uint8_t)sub_0D0E3A(m);
        uint8_t d1b = (uint8_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
        if (d1b < d0b)
            return;
    }

    uint8_t v22 = rd8(m, obj + 0x22u);
    wr8(m, obj + 0x22u, (uint8_t)(v22 ^ 0x01u));
    uint32_t a1 = ((v22 & 0x01u) == 0u) ? 0x000E6B5Eu : 0x000E6BCAu;
    sub_0D0678(m, obj, a1);
    wr8(m, obj + 0x06u, 2u);
}

/* ROM 0x0E6AA0: state 2 - animate firing and spawn projectiles */
static void sub_0E6AA0(const f3mem_t *m, uint32_t obj)
{
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        uint32_t a1 = (rd8(m, obj + 0x22u) == 0u) ? 0x000E6B4Au : 0x000E6B54u;
        sub_0CFA86(m, obj, a1);
        wr8(m, obj + 0x23u, 0x1Eu);
        wr8(m, obj + 0x06u, 3u);
        return;
    }

    int16_t d1w = (int16_t)rd16(m, obj + 0x68u);
    if (d1w < 0)
        return;
    if (d1w == 0)
        Obj_Spawn(m, obj, 0x000E6C36u, 9u, 2u, 0u);
    else
        Obj_Spawn(m, obj, 0x000E6E0Eu, 9u, 2u, 0u);
}

/* ROM 0x0E6B1A: state 3 - cooldown timer */
static void sub_0E6B1A(const f3mem_t *m, uint32_t obj)
{
    uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c);
    if (c == 0u)
        wr8(m, obj + 0x06u, 1u);
}

/* ROM 0x0E6906 */
int Obj_E6906_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, parent + 0x06u) == 0u)
        goto destroy_check;

    uint8_t state = rd8(m, obj + 0x06u);
    if (state == 0u) {
        Obj_DrawSprite(m, parent);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x00000190u);
        goto destroy_check;
    }

    if (state > 3u)
        return -1;

    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Eu));
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0042u));

    if (state == 1u)
        sub_0E6A0A(m, obj);
    else if (state == 2u)
        sub_0E6AA0(m, obj);
    else
        sub_0E6B1A(m, obj);

    sub_0D0DC2(m, obj);
    sub_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, parent);
    return 0;

destroy_check:
    if (rd8(m, obj + 0x06u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        if ((int16_t)rd16(m, obj + 0x14u) >= -0x46) {
            uint32_t child = Obj_Spawn(m, obj, 0x000DC358u, 3u, 2u, 0u);
            if ((int32_t)child >= 0) {
                wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0020u));
                wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
                wr16(m, child + 0x58u, 0u);
                wr16(m, child + 0x60u, 0u);
            }
        }
    }
    if (rd8(m, parent + 0x06u) == 0u)
        Obj_StartDying(m, obj);
    else
        Obj_DrawSprite(m, parent);
    return 0;
}
