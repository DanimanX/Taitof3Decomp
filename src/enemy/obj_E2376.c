#include "obj_E2376.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if ((flags & 0x80u) == 0u)
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    wr8(m, obj + 0x07u, (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u));
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

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t tag = rd8(m, obj + 0x35u);
    if (tag == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (tag == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0E2376: turret on a carrier (parent +8, carrier = grandparent) */
int Obj_E2376_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t gp = 0;
    if (rd8(m, obj + 0x22u) != 0u) {
        gp = rd32(m, rd32(m, obj + 0x08u) + 0x08u);
        if (rd8(m, gp + 0x06u) == 0u)
            wr8(m, obj + 0x22u, 0u);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0xC8u);
            wr32(m, obj + 0x10u, 0x5D078u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
            if (rd8(m, obj + 0x22u) != 0u)
                wr8(m, rd32(m, rd32(m, obj + 0x08u) + 0x08u) + 0x06u, 5u);
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    uint8_t st = rd8(m, obj + 0x06u);
    if (st != 0u) {
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            if (rd8(m, obj + 0x22u) != 0u && rd8(m, gp + 0x06u) == 1u)
                wr8(m, gp + 0x06u, 5u);
            sub_0D0E04(m, obj, 0xC8u);
            goto burst;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
            goto burst;
        if (st > 3u)
            return -1;
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
            goto clr6;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        if (st == 1u) {
            sub_0CF7FE(m, obj);
            if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
                wr8(m, obj + 0x06u, 2u);
            }
        } else if (st == 2u) {
            if (rd8(m, obj + 0x22u) == 0u || rd8(m, gp + 0x06u) != 1u) {
                sub_0D0678(m, obj, 0xE25B4u);
                wr8(m, obj + 0x06u, 3u);
            }
        } else {
            sub_0D0690(m, obj);
        }
        sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
    }
draw:
    if ((uint8_t)(rd8(m, obj + 0x06u) + rd8(m, obj + 0x22u)) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) != 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        uint32_t g = rd32(m, p + 0x08u);
        if (rd8(m, g + 0x06u) != 1u)
            Obj_DrawSprite(m, p);
        Obj_DrawSprite(m, g);
        if (rd8(m, obj + 0x06u) == 0u)
            return 0;
    }
    Obj_DrawSprite(m, obj);
    return 0;
burst:
    {
        uint32_t c = Obj_Spawn(m, obj, 0xDC456u, 3u, 2u, 0u);
        if ((int32_t)c >= 0) {
            wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x18u));
            wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x0Cu));
            wr16(m, c + 0x58u, 0u);
            wr16(m, c + 0x60u, 0u);
        }
    }
clr6:
    wr8(m, obj + 0x06u, 0u);
    goto draw;
}
