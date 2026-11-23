#include "obj_126254.h"
#include "obj_11B262.h"

/* ROM 0x126254: step 0x00 of the boss state machine (0x1261FE family). The
 * 0x11B288/0x11B2CA/0x11B262 trio, then the +0x06 countdown; on expiry
 * A5+0x30EE{0xE} is set and the 0x126282 leg takes over with the same timer
 * armed (0x12C).
 *
 * Flag check for the `beq`: the producer at 0x126272 is
 * `move.w #0x12C,(0x6,A0)` (31 7C 01 2C 00 06 in work/maincpu.bin), which does
 * not touch CCR, and the instruction at 0x126278 is 67 20 (beq.b +0x20) -- the
 * real producer is truncated in the bundle. Per the golden pairs
 * (work/ram/obj_126254_regs.tsv, +0x06 in 0x12D..0x12E) the branch is NOT taken,
 * so 0x12627A always runs. Same for the 0x1262A6 beq (fall-through 0x1262A8),
 * which no captured leg reaches. */
int Obj_126254_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126254 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12625A jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126260 jsr 0x11B262 */

    /* 0x126266: subq.w #1 -> bgt 0x12624C tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126254u);         /* 0x12624C */
        return 0;
    }

    /* 0x12626C: bfset (0x30EE,A5){0xE:1} -> bit 6 of A5+0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Eu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Eu % 8u))));
    }
    wr16(m, obj + 0x06u, 0x012Cu);                 /* 0x126272 */
    wr32(m, obj + 0x00u, 0x00126282u);             /* 0x12627A */
    return 0;                                      /* 0x126280 */
}
