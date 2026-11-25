#include "obj_17E84E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17E84E: burst spawner tail. Places the object from its +0x0C record,
 * gives up (0x17C78E, free) once it leaves the 0x1849F8 region, otherwise
 * plays sound 0x2A and spawns eight 0x17EA0A children with arg 7..0. */

/* ROM 0x13F238 -> 0x0039D8: sound command word on the stack, +0x53 in the
 * alternate sound mode. */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    Sound_PlayCmd(m, cmd);
}

static int in_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return x > lo && x < hi;                          /* cmp2.w: lo < D < hi */
}

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17E84E */
int Obj_17E84E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x17E84E: bsr 0x17BDA2 */
    Obj_PolarPlace(m, obj);

    /* 0x17E852..0x17E86E: cmp2.w (0x1849F8)/(0x1849FC) ; bcs 0x17C78E */
    if (!in_box(m, 0x001849F8u, rd16(m, obj + 0x50u)) ||
        !in_box(m, 0x001849FCu, rd16(m, obj + 0x54u))) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17E872: pea (0x0).w; move.w #1,-(SP); move.w #0xB,-(SP);
     *          move.l #0x17C856,-(SP); jsr 0x1057C */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C856u, 11u, 1u, 0u);

        /* 0x17E88E: move.w #1,(0x6,A0); beq 0x17E8A4 -- Z comes from the
         * discarded spawn result, which the engine leaves in D0 (0 = full). */
        wr16(m, obj + 0x06u, 1u);
        if ((int32_t)child != 0) {
            /* 0x17E896 */
            wr32(m, obj + 0x00u, 0x0017E89Eu);
            return 0;
        }
    }

    /* 0x17E8A4: sound 0x2A, behaviour 0x17EBCE, then eight 0x17EA0A children */
    play_sound(m, 0x002Au);

    {
        uint32_t a1 = 0x0017EBCEu;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
        wr16(m, obj + 0x4Au, 0u);
    }

    /* 0x17E8CC: moveq #7,D7 ; dbf loop spawning {#2, #9, 0x17EA0A, D7} */
    for (uint16_t d7 = 7u; ; d7--) {
        Obj_Spawn(m, obj, 0x0017EA0Au, 9u, 2u, d7);
        if (d7 == 0u)
            break;
    }

    /* 0x17E992: move.l #0x17E99A,(A0) */
    wr32(m, obj + 0x00u, 0x0017E99Au);
    return 0;
}
