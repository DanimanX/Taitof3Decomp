#include "obj_118B74.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x118B6C: re-arm to 0x118B74 (the loop head after the frame step). */
static void sub_118B6C(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00118B74u);
}

/* ROM 0x118B74: one step of the frame table at 0x118BA6 (30 entries of
 * {dur.w, sprdef.l}), then fall -0x8000 in Y and draw; when the table ends,
 * free the object.
 *
 * Raw ROM at 0x118B74:
 *   43FA 0030     lea (0x30,PC),A1      -> 0x118BAC
 *   4E71          nop
 *   4EB9 000CBD24 jsr 0x0CBD24
 *   4A71 0000     tst.w (0x0,A1,D0w)
 *   6F00 0012     ble.w 0x118B98
 *   04A8 00008000 subi.l #0x8000,(0x18,A0)
 *   4EB9 00001282 jsr 0x1282
 *   60D4          bra.b 0x118B6C
 * 0x118B6C: 20BC 00118B74  move.l #0x118B74,(A0) ; rts
 * 0x118B98: 2F08 / 4EB9 000105EC / 4FEF 0004 / 4E75  -> Obj_Free, rts
 */
int Obj_118B74_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t table = 0x00118BA6u;   /* lea (0x30,PC) at 0x118B74 */

    /* 0x118B7A. The helper returns nothing, but the ROM's `tst.w
     * (0x0,A1,D0w)` at 0x118B80 uses the D0w the step left behind, which is
     * the table offset the loop ended on -- i.e. obj+0x3A after the step
     * (the helper only rewinds +0x3A when it exhausts the list, and in that
     * case the entry it lands on is the terminating one). */
    Obj_AnimFrameStep(m, obj, table);
    {
        uint16_t off = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, table + (uint32_t)(int16_t)off) <= 0)
            goto free_it;                              /* 0x118B84 */
    }

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x00008000u);  /* 0x118B88 */
    Obj_DrawSprite(m, obj);                            /* 0x118B90 */
    sub_118B6C(m, obj);                                /* 0x118B96 bra 0x118B6C */
    return 0;

free_it:
    Obj_Free(m, obj);                                  /* 0x118B9A */
    return 0;
}
