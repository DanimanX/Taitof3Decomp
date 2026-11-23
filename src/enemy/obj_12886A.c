#include "obj_12886A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x13E4D0: the argument for "spawn this child": D0 = (D0b | (w(A1)+ << 3)
 * << 8) | (w(A1)++ << 3), i.e. the two sprite-def words at (0x10,A0) scaled
 * by 8 into the spawn argument's low two bytes. */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    uint32_t d0 = 1u;
    d0 = (d0 & ~0x0000FF00u) | ((uint32_t)d1 << 8);    /* bfins D1,D0{8:8} */
    d1 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    d0 = (d0 & ~0x000000FFu) | (uint32_t)(d1 & 0xFFu); /* bfins D1,D0{0:8} */
    return d0;
}

/* ROM 0x128862: stay on this routine. */
static uint32_t sub_128862(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012886Au);
    return 0u;
}

/* ROM 0x12886A: a debris chunk. Above the screen it burns one frame on the
 * explosion animation 0x0CBB2C; below, while A5+0x1D58 bit 1 is clear and it
 * still has HP (+0x24), it integrates its own velocity (+0x44/+0x48, gravity
 * +0x3800 per frame on Y), draws, and re-lists itself through 0x0CBDA4. Once
 * the ground line (+0x18 < 0xFC) is reached it spawns one 0x13E4EE child
 * (group 0, one group, argument from 0x13E4D0) and frees itself, as it does
 * when the scroll lock A5+0x1D58 bit 2 or the global freeze A5-0x144B bit 4
 * is set, or when its HP runs out. */
int Obj_12886A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto free_1288f6;                              /* 0x128870 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (int16_t)rd16(m, obj + 0x14u) >= (int16_t)-0x0A &&
        (int16_t)rd16(m, obj + 0x14u) <= (int16_t)0x011C) {
        Obj_OrbitDebug(m, obj);                        /* 0x12888C -> 0x0CBB2C */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x04u) != 0u)
        goto free_1288f6;                              /* 0x128898 */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto spawn_1288d4;                             /* 0x1288A0 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x44u)); /* 0x1288A4 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x48u)); /* 0x1288AC */
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00003800u);         /* 0x1288B4 */

    Obj_DrawSprite(m, obj);                            /* 0x1288BC -> 0x001282 */

    /* 0x1288C2 -> 0x0CBDA4: append to A5+0x8FE then fall into 0x0CBDAE */
    {
        uint32_t d0 = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, d0, obj);
        wr32(m, F3_A5 + 0x8FEu, d0 + 4u);
        Coll_AddHazard(m, obj);
    }

    if ((int16_t)rd16(m, obj + 0x18u) >= 0x00FC)
        goto free_1288f6;                              /* 0x1288D0 */
    (void)sub_128862(m, obj);                          /* 0x1288CE */
    return 0;

spawn_1288d4:
    /* 0x1288D4: spawn 0x13E4EE (group 0, 1 group), arg = 0x13E4D0(obj) */
    (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, sub_13E4D0(m, obj));

free_1288f6:
    Obj_Free(m, obj);                                  /* 0x1288F8 -> 0x0105EC */
    return 0;
}
