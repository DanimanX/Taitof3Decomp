#include "obj_DDAE0.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    int16_t d0 = (int16_t)rd16(m, a1);
    a1 += 2u;
    if (d0 >= 0) {
        wr16(m, obj + 0x4Cu, (uint16_t)d0);
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    uint16_t rewind = rd16(m, a1);
    if (rewind != 0u) {
        a1 -= rewind;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
    }
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t s = rd8(m, obj + 0x35u);
    if (s == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (s == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
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

/* ROM 0x0CF7FE: arm the script when re-entering the screen */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x14F)
        return;
    f7 = (uint8_t)(f7 & ~0x80u);
    wr8(m, obj + 0x07u, f7);
    wr8(m, obj + 0x07u, (uint8_t)(f7 | 0x02u));
}

/* ROM 0x0DDAE0: ground turret that fires two shots at the end of each
 * animation frame (+0x68 >= 0) */
int Obj_DDAE0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x0000012Cu);
            wr32(m, obj + 0x10u, 0x0005C6BCu);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x19u));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    {
        int dying = 0;
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            sub_0D0E04(m, obj, 0x0000012Cu);
            dying = 1;
        } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            dying = 1;
        }
        if (dying) {
            uint32_t d0 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
            if ((int32_t)d0 >= 0) {
                wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x18u));
                wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x28u));
                wr16(m, d0 + 0x58u, 0u);
                wr16(m, d0 + 0x60u, 0u);
            }
            Obj_StartDying(m, obj);
            return 0;
        }
    }
    if (rd8(m, obj + 0x06u) > 1u)
        return -1;
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if (rd8(m, obj + 0x06u) == 0u) {
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            sub_0D0678(m, obj, 0x000DDCC8u);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x06u, 1u);
        }
    } else {
        sub_0D0690(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0D0678(m, obj, 0x000DDCC8u);
        } else {
            uint16_t d1 = rd16(m, obj + 0x68u);
            if ((int16_t)d1 >= 0) {
                uint32_t d0 = Obj_Spawn(m, obj, 0x000DE064u, 9u, 2u, 0u);
                if ((int32_t)d0 >= 0) {
                    wr16(m, d0 + 0x58u, d1);
                    d0 = Obj_Spawn(m, obj, 0x000DE140u, 9u, 2u, 0u);
                    if ((int32_t)d0 >= 0)
                        wr16(m, d0 + 0x58u, d1);
                }
            }
        }
    }
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
