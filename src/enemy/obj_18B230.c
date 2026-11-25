/* ROM 0x18B230: initialiser of the 0x18B2CA family. Damps the sprite scale, seeds
 * the heading (+0x42) and the two effect timers (+0x50 / +0x54) from the
 * A5+0x4B3C LCG, points the frame list (+0x44) at 0x18B3C2, allocates the
 * 0x18B3CC flash tick on group 3 and arms 0x18B2CA. */
#include "obj_18B230.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1794A4 / 0x1794F0: a value in [-n+1..n-1] from the LCG at A5+0x4B3C.
 * 0x1794A4 follows with `neg.w D0w; move.l D1,(A5+0x4B3C)` and 0x1794F0 ends
 * with `addq.l #1,D1`. Both instructions are dead here: 0x1794F0's tail is the
 * A5 store that Obj_RandRange already performs, and the `neg.w D0w` only
 * matters for the odd draw whose low byte has bit 0 set (bfffo low-byte == 0x10
 * breaks to the NEG path in 0x1794CA and then negates D1, which is also
 * invisible: `move.w D1w,D0w` has already copied the result). */

/* ROM 0x18B230 */
int Obj_18B230_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B230: move.w #-0x8000,(A0+0x20) */
    wr16(m, obj + 0x20u, 0x8000u);

    /* 0x18B236: move.l #0x200,D0 ; jsr 0x1794F0 ; addi.w #0x200,D0w ;
     *           move.w D0w,(A0+0x42) */
    uint16_t d0 = Obj_RandRange(m, 0x0200u);
    wr16(m, obj + 0x42u, (uint16_t)(d0 + 0x0200u));

    /* 0x18B24A: move.w #0x1F4,(A0+0x50) ; moveq #0x20,D0 ; jsr 0x1794A4 ;
     *           add.w D0w,(A0+0x50) */
    wr16(m, obj + 0x50u, 0x01F4u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + Obj_RandRange(m, 0x0020u)));

    /* 0x18B25C: move.l (A0+0xC),D0 ; mulu.w #0x1C,D0 ; move.w D0w,(A0+0x54) */
    uint32_t d0l = rd32(m, obj + 0x0Cu) * 0x001Cu;
    wr16(m, obj + 0x54u, (uint16_t)d0l);

    /* 0x18B268: moveq #0x19,D0 ; jsr 0x1794A4 ; add.w D0w,(A0+0x54) */
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + Obj_RandRange(m, 0x0019u)));

    /* 0x18B274: movea.l #0x18B3C2,A1 ; move.l A1,(A0+0x44) ;
     *           move.w (0x4,A1),(A0+0x48) ; clr.w (A0+0x4A) */
    const uint32_t kFrameList = 0x0018B3C2u;
    wr32(m, obj + 0x44u, kFrameList);
    wr16(m, obj + 0x48u, rd16(m, kFrameList + 4u));
    wr16(m, obj + 0x4Au, 0u);

    /* 0x18B288: pea (0x0).w ; move.w #2 ; move.w #3 ;
     *           move.l #0x18B3CC ; jsr 0x1057C (D0 unused) */
    (void)Obj_Spawn(m, obj, 0x0018B3CCu, 3u, 2u, 0x00000000u);

    /* 0x18B2A4: clr.w (A0+0x22) ; clr.b (A0+0x35) ; move.b #8,(A0+0x34) ;
     *           move.l #0xA0000,(A0+0x24) ; move.l #0x10000,(A0+0x28) */
    wr16(m, obj + 0x22u, 0u);
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr32(m, obj + 0x24u, 0x000A0000u);
    wr32(m, obj + 0x28u, 0x00010000u);

    /* 0x18B2C2: move.l #0x18B2CA,(A0) */
    wr32(m, obj + 0x00u, 0x0018B2CAu);
    return 0;
}
