/* ROM 0x17A42A: entrance tick of the 0x17A504 slaved object. Seeds the frame
 * list, runs its index-0 tick, then (after the A5+0x4B24 gate) installs the
 * 0x17A504 movement script and mirrors the parent. */
#include "obj_17A42A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x179D18 */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17A42A */
int Obj_17A42A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A42A: movea.l #0x17A4D6,A1 ; move.w (A1)+,D0 ; move.w D0,(0x40,A0) ;
     * move.l A1,(0x3A,A0). The record is {0, 1, 5, 0xA17C, ...}: index 0, then
     * the frame list 0x17A4D8 ({dur.w 1, spr.l 0x0005A17C}) at +0x3A. */
    uint32_t a1 = 0x0017A4D6u;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);

    /* 0x17A43A: movea.l (0x0,A1,D0w*8),A1 with A1 = 0x179A5C ; jsr (A1).
     * (0x179A5C)[0] = 0x179550 is the INIT half of frame list 0, and
     * 0x0E73C4-0x0E73E2 of the parent family shows this dispatch is `[idx]`,
     * not `[idx+4]`. Pair 0 of the capture confirms 0x179550 ran: +0x3E went
     * 0xFFFF -> 0 -> 1, so its 0x1796A8 `bsr 0x179550` also ran. The ROM
     * returns straight after the init (0x179550 -> 0x179880 -> rts), so the
     * tick half is NOT run on this entry. */
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;

    /* 0x17A446 bra.w 0x17A44A */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        sub_179D18(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));

    /* 0x17A458: install the movement script 0x17A504 and its timer */
    a1 = 0x0017A504u;
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    wr16(m, obj + 0x4Au, 0u);

    /* 0x17A46C: mirror the parent's offset and turn away from it */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, a4 + 0x20u) - 0x80u));
    }
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);

    /* 0x17A492: moveq #0xA,D0 ; jsr 0x1794A4 ; add.b D0b,(0x20,A0) */
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)Obj_RandRange(m, 0x0Au)));

    wr16(m, obj + 0x42u, 0x0002u);              /* 0x17A49E */
    wr32(m, obj, 0x00179C6Eu);                  /* 0x17A4A4 bra.w 0x179C66 */
    return 0;
}
