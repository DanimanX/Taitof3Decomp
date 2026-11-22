#include "obj_D78C0.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CBD12: hand the object's score to the player, then 0x0CBB2C */
static void sub_0CBD12(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    uint32_t d0 = rd16(m, obj + 0x4Cu);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x0CBDA4 (+0x0CBDAE): append obj to lists A5+0x8FE and A5+0x906 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CBDDE: owner through the +0x36 link */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0CBE34: colour override / blue flash */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
        return;
    }
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
}

/* ROM 0x0D78C0 */
int Obj_D78C0_Tick(const f3mem_t *m, uint32_t obj)
{
    if (!((int32_t)rd32(m, obj + 0x24u) > 0)) {
        /* 0x0D795E: score to the owner, then the death burst */
        uint32_t a1 = sub_0CBDDE(m, obj);
        uint32_t d0 = rd16(m, obj + 0x4Cu);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
        return -1;                               /* 0x0D796E sound/spawn path */
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        return -1;                               /* 0x0D796E sound/spawn path */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0CBD12(m, obj);
            return -1;                           /* 0x0CBD20 jmp 0x0CBB2C */
        }
    }

    /* 0x0D78F6 */
    wr16(m, obj + 0x42u, (uint16_t)((int16_t)rd16(m, obj + 0x62u) >> 8));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));

    /* 0x0D790C: steer +0x20 toward -0x4000 (dead zone +-0x1800) */
    {
        uint16_t d0 = (uint16_t)(0xC000u - rd16(m, obj + 0x20u));
        if ((int16_t)d0 > 0x1800) {
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0100u));
        } else if ((int16_t)d0 < -0x1800) {
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0100u));
        }
    }

    /* 0x0D7934 */
    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u)
                                    + (uint16_t)((int16_t)rd16(m, obj + 0x60u) >> 1)));
    sub_0CBDA4(m, obj);
    sub_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);

    /* 0x0D7950: still on screen -> repeat at 0x0D78C0 */
    if ((int16_t)rd16(m, obj + 0x18u) >= -0x30) {
        wr32(m, obj + 0x00u, 0x000D78C0u);
        return 0;
    }

    /* 0x0D79B0: off the top of the screen -> free */
    Obj_Free(m, obj);
    return 0;
}
