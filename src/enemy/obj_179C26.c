#include "obj_179C26.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x179C26: the debris/shrapnel walker. Screen position equals the
 * parent-relative offset (+0x50/+0x54), the animation tick runs, and the
 * sprite is centred by the frame's own (w, h) before it is emitted. */

/* ROM 0x179D18: drop the slot counter and free the object */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj); /* 0x0105EC */
}

int Obj_179C26_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179C26: move.w (0x50,A0),(0x14,A0) ; move.w (0x54,A0),(0x18,A0) */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x179C32: move.w (0x40,A0),D0w ; lea (0x179A5C),A1 ;
     * movea.l (0x4,A1,D0w*8),A1 ; jsr (A1) -- the tick half of the entry */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x179C42: centre the sprite by its frame (w, h) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    Obj_DrawSprite(m, obj); /* 0x179C56 */

    /* 0x179C5C: tst.w (0x3E,A0) ; bge -> 0x179C1E, else -> 0x179D14 */
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        wr32(m, obj + 0x00u, 0x00179C26u); /* 0x179C1E move.l #0x179C26,(A0) */
        return 0;
    }
    sub_179D18(m, obj);
    return 0;
}
