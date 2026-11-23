#include "obj_12D244.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D244 (entered with +0x00 = 0x12D244, +0x04 = group 3, index 2)
 *
 * Bytes re-decoded from work/maincpu.bin:
 *   12D244 0c6d cc00 32a2      cmpi.w #0xcc00,(0x32a2,A5)
 *   12D24A 6d00 0006           blt.w  0x12D252
 *   12D24E 526d 32a6           addq.w #1,(0x32a6,A5)
 *   12D252 066d 0400 32a2      addi.w #0x400,(0x32a2,A5)
 *   12D258 4a6d 32a2           tst.w  (0x32a2,A5)
 *   12D25C 6dde                blt.b  0x12D23C     (back into the state before)
 *   12D25E 2f3c 0000 000a      move.l #0xA,-(SP)   arg
 *   12D264 3f3c 0002           move.w #2,-(SP)     ngroups
 *   12D268 3f3c 0003           move.w #3,-(SP)     group
 *   12D26C 2f3c 0013 02f8      move.l #0x1302F8,-(SP)
 *   12D272 4eb9 0001 057c      jsr 0x01057C        Obj_Spawn(A0, fn, 3, 2, 0xA)
 *   12D278 4fef 000c           lea (0xC,SP),SP
 *   12D27C 20bc 0012 d284      move.l #0x12D284,(A0)
 *   12D282 4e75                rts
 *
 * The captured ticks all enter the first state, where 0x32A2 = 0, so the
 * `blt` at 0x12D25C is not taken and 0x12D23C (the preceding state's tail,
 * which this routine does not own) is never reached. `cmpi.w` with 0xCC00 and
 * 0x400 added keeps 0x32A2 negative until it crosses zero, so both checks are
 * signed. The parent A0 goes to Obj_Spawn unchanged. */
int Obj_12D244_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t v = (int16_t)rd16(m, F3_A5 + 0x32A2u);          /* 0x12D244 */
    if (v >= (int16_t)0xCC00) {                             /* 0x12D24A blt */
        wr16(m, F3_A5 + 0x32A6u, (uint16_t)(rd16(m, F3_A5 + 0x32A6u) + 1u));
    }

    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) + 0x0400u));

    if ((int16_t)rd16(m, F3_A5 + 0x32A2u) < 0) {            /* 0x12D25C blt */
        /* 0x12D23C: previous state's body, not part of this routine */
        return 0;
    }

    (void)Obj_Spawn(m, obj, 0x001302F8u, 3u, 2u, 0x0000000Au);

    wr32(m, obj + 0x00u, 0x0012D284u);                      /* 0x12D27C */
    return 0;
}
