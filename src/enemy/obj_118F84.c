#include "obj_118F84.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x118F84: one step of the animation table selected by obj+0xC, drawn at
 * angle 0xC0; free the object when the table ends.
 *
 * Raw ROM at 0x118F84 (work/maincpu.bin):
 *   43FA 0046     lea (0x46,PC),A1      -> 0x118FCC (PC + 2 + 0x46)
 *   4E71          nop
 *   2028 000C     move.l (0xC,A0),D0
 *   2271 0000     movea.l (0x0,A1,D0w*4),A1
 *   4EB9 000CBD24 jsr 0x0CBD24
 *   4A71 0000     tst.w (0x0,A1,D0w)
 *   6F00 0012     ble.w 0x118FB2
 *   303C 00C0     move.w #0xC0,D0w
 *   4EB9 000CBE12 jsr 0x0CBE12
 *   4EB9 00001282 jsr 0x1282
 *   60D4          bra.b 0x118F7C
 * 0x118F7C: 20BC 00118F84  move.l #0x118F84,(A0) ; rts
 * 0x118FB2: 2F08 / 4EB9 000105EC / 4FEF 0004 / 4E75  -> Obj_Free, rts
 * Table 0x118FCC: 00 11 8F D8 | 00 11 8F F8 | 00 11 90 18 (key 3 is past it)
 */
int Obj_118F84_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t table = 0x00118FCCu;   /* lea (0x46,PC) at 0x118F84 */

    /* 0x118F8A: D0 = obj+0xC; A1 = table[D0w] (word index into the long list) */
    uint32_t index = rd32(m, obj + 0x0Cu);
    uint32_t a1 = rd32(m, table + 4u * (uint32_t)(uint16_t)index);

    /* 0x118F92. Obj_AnimFrameStep keeps the table offset in D0 like the ROM
     * does; a caller that needs it re-reads obj+0x3A, as here. */
    Obj_AnimFrameStep(m, obj, a1);
    {
        uint16_t off = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int16_t)off) <= 0)
            goto free_it;                              /* 0x118F98 */
    }

    /* 0x118FA0: angle 0xC0 at the speed in (0x42,A0) -- 0x0CBE12 */
    {
        int16_t spd = (int16_t)rd16(m, obj + 0x42u);
        int16_t cs = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * 0x00C0u);
        int16_t sn = (int16_t)rd16(m, 0x027E40u + 2u * 0x00C0u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)(int32_t)(spd * cs));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)(spd * sn));
    }
    Obj_DrawSprite(m, obj);                            /* 0x118FAA jsr 0x1282 */
    wr32(m, obj + 0x00u, 0x00118F84u);                 /* 0x118FB0 bra 0x118F7C */
    return 0;

free_it:
    Obj_Free(m, obj);                                  /* 0x118FB4 */
    return 0;
}
