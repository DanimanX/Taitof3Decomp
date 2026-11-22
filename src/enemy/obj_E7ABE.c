#include "obj_E7ABE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
/* ROM 0x0D0E3A */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
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

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0CBE34 */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x0CFA86 */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}


/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t k = rd8(m, obj + 0x35u);
    uint32_t a1;
    if (k == 1u)
        a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    else if (k == 2u)
        a1 = rd32(m, obj + 0x36u);
    else
        return;
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* |x(obj) - x(p) - 3| <= 10 (p active) */
static int near_x(const f3mem_t *m, uint32_t obj, uint32_t p)
{
    if ((int16_t)rd16(m, p + 0x4Cu) < 0)
        return 0;
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) - 3u);
    if ((int16_t)d0 < 0)
        d0 = (uint16_t)-d0;
    return d0 <= 0x0Au;
}

/* jump table 0xE7BB2 */
static void state_E7BC2(const f3mem_t *m, uint32_t obj)
{
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (y < -0x20 || y > 0xC7)
        return;
    if (!near_x(m, obj, 0x41B190u) && !near_x(m, obj, 0x41B21Au)) {
        uint8_t d0 = (uint8_t)sub_0D0E3A(m);
        uint8_t d1 = (uint8_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
        if (d1 < d0)
            return;
    }
    uint8_t b = rd8(m, obj + 0x22u);
    wr8(m, obj + 0x22u, (uint8_t)(b ^ 1u));
    sub_0D0678(m, obj, (b & 1u) ? 0xE7D82u : 0xE7D16u);
    wr8(m, obj + 0x06u, 2u);
}

static void state_E7C58(const f3mem_t *m, uint32_t obj)
{
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) != 0u) {
        int16_t d1 = (int16_t)rd16(m, obj + 0x68u);
        if (d1 < 0)
            return;
        Obj_Spawn(m, obj, d1 == 0 ? 0xE7DEEu : 0xE7FC6u, 9u, 2u, 0u);
        return;
    }
    sub_0CFA86(m, obj, rd8(m, obj + 0x22u) ? 0xE7D0Cu : 0xE7D02u);
    wr8(m, obj + 0x23u, 0x1Eu);
    wr8(m, obj + 0x06u, 3u);
}

static void state_E7CD2(const f3mem_t *m, uint32_t obj)
{
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t == 0u)
        wr8(m, obj + 0x06u, 1u);
}

/* ROM 0x0E7ABE: turret riding on parent (+8) */
int Obj_E7ABE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u && rd8(m, p + 0x06u) != 0u) {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st != 0u) {
            if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
                sub_0D0E04(m, obj, 0x190u);
                goto dead;
            }
            if (st > 3u)
                return -1;
            wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x42u));
            wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0Eu));
            if (st == 1u)
                state_E7BC2(m, obj);
            else if (st == 2u)
                state_E7C58(m, obj);
            else
                state_E7CD2(m, obj);
            sub_0D0DC2(m, obj);
            sub_0CBE34(m, obj);
            Obj_DrawSprite(m, obj);
        }
        Obj_DrawSprite(m, p);
        return 0;
    }
dead:
    if (rd8(m, obj + 0x06u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        if ((int16_t)rd16(m, obj + 0x14u) >= -0x19) {
            uint32_t c = Obj_Spawn(m, obj, 0xDC358u, 3u, 2u, 0u);
            if ((int32_t)c >= 0) {
                wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x10u));
                wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x20u));
                wr16(m, c + 0x58u, 0u);
                wr16(m, c + 0x60u, 0u);
            }
        }
    }
    p = rd32(m, obj + 0x08u);
    if (rd8(m, p + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, p);
    return 0;
}
