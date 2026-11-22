#include "obj_1289A6.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x1289A6: ballistic mover — pos += vel (+0x44/+0x48), gravity 0x3800 per
 * frame on the y velocity, draw, and free once the y word falls below -4
 * (cmpi.w #0xFC,(0x18,A0): blt when y < -4). */
int Obj_1289A6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1289A6 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x48u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00003800u);

    Obj_DrawSprite(m, obj);                             /* 0x1289BE jsr 0x1282 */

    if ((int16_t)rd16(m, obj + 0x18u) < -4) {           /* 0x1289C4 cmpi.w/blt */
        /* 0x12899E: nothing to do, then free */
        Obj_Free(m, obj);                               /* 0x1289CE */
        return 0;
    }

    /* 0x12899E */
    wr32(m, obj + 0x00u, 0x001289A6u);
    return 0;
}
