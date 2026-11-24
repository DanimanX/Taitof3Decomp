#include "obj_11418E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x11418E: follow-up phase of the 0x1140F4 chain. It mirrors the parent's
 * +0x50 flag, drives the sprite frame from the +0x6 timer, animates through the
 * 0x114224 table and draws. When the +0x6 countdown runs out it spawns the
 * 0x11423E worker against the parent and jumps back into 0x1140F4. */
int Obj_11418E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);

    if ((rd8(m, parent + 0x50u) & 0x20u) != 0u) {
        Obj_Free(m, obj);                       /* 0x114216 */
        return 0;
    }

    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x06u) >> 1);
    wr16(m, obj + 0x42u, d0);

    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u)); /* 0x1141AA jsr 0xCBE12 */
    Obj_Animate(m, obj, 0x00114224u);           /* 0x1141B6 jsr 0xCBD52 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            /* 0x1141D4 jmp 0x1141E6: spawn the worker instead of counting */
            goto spawn;
        }
    }

    Obj_DrawSprite(m, obj);                     /* 0x1141DA jsr 0x001282 */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x1141E4 bgt.b -> 0x114186 */
        wr32(m, obj + 0x00u, 0x0011418Eu);
        return 0;
    }

spawn:
    /* 0x1141E6: Obj_Spawn(parent, 0x11423E, group 9, n=2, arg = parent+0x50{4:1}) */
    {
        int32_t arg = (rd8(m, parent + 0x50u) & 0x10u) ? -1 : 0; /* bfexts {4:1} */
        Obj_Spawn(m, parent, 0x0011423Eu, 9u, 2u, (uint32_t)arg);
    }
    wr32(m, obj + 0x00u, 0x00114212u);          /* 0x11420A */
    return 0;
}
