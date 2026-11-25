#include "obj_17A070.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17A070: the flying half of the 0x17A016 burst. The sprite rides on
 * the parent's position plus the latched +0x16/+0x1A offset, the 0x179A5C
 * frame list and the 0x001282 emitter run, and the object frees itself once
 * the frame countdown (+0x3E) goes negative -- dropping the A5+0x4B24 slot it
 * took in 0x17A016. */

int Obj_17A070_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A070: movea.l (0x8,A0),A4 ; move.w (0x50,A4)/(0x54,A4),D0w/D1w ;
     * add.w (0x16,A0)/(0x1A,A0) ; move to +0x50/+0x54 and +0x14/+0x18 */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr16(m, obj + 0x14u, d0);
        wr16(m, obj + 0x18u, d1);
    }

    /* 0x17A094: move.w (0x40,A0),D0w ; movea.l (0x179A5E,D0w*8),A1 ; jsr (A1) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17A0A4: centre the sprite by the frame's (w, h) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    Obj_DrawSprite(m, obj); /* 0x17A0B8 */

    /* 0x17A0BE: tst.w (0x3E,A0) ; bge -> 0x17A068 (move.l #0x17A070,(A0)) */
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        wr32(m, obj + 0x00u, 0x0017A070u);
        return 0;
    }

    /* 0x17A0C4: subq.w #1,(0x4B24,A5) ; jsr 0x0105EC */
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj);
    return 0;
}
