#include "obj_179C6E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../video/sprite.h"

/* ROM 0x179C6E: the scripted walker. Steps the movement script at +0x44
 * (0x178982), mirrors the parent-relative offset into the screen position,
 * ticks the animation and draws, then drops into the same end-of-life test as
 * 0x179C26. Every path leads to a translated helper, so it never returns -1. */

/* ROM 0x179D18: drop the slot counter and free the object */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj); /* 0x0105EC */
}

int Obj_179C6E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179C6E: jsr 0x178982 -- one pass of the 10-byte movement script */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x179C74: move.w (0x50,A0),(0x14,A0) ; move.w (0x54,A0),(0x18,A0) */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x179C80: move.w (0x40,A0),D0w ; movea.l (0x4,A1,D0w*8),A1 ; jsr (A1) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x179C90: centre the sprite by its frame (w, h) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    Obj_DrawSprite(m, obj); /* 0x179CA4 */

    /* 0x179CAA: tst.w (0x3E,A0) ; bge -> 0x179C66, else -> 0x179D14 */
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        wr32(m, obj + 0x00u, 0x00179C6Eu); /* 0x179C66 move.l #0x179C6E,(A0) */
        return 0;
    }
    sub_179D18(m, obj);
    return 0;
}
