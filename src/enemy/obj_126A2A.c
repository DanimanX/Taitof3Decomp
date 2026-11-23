#include "obj_126A2A.h"
#include "obj_11B288.h"

/* ROM 0x126A2A: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader; this tick
 * additionally re-targets +0x50/+0x52 from the leader record A1 = (A5+0x30E2):
 * +0x50 = w(A1+0x14)+0x18, +0x52 = w(A1+0x18)+0x10. On countdown expiry it
 * clears bit A5+0x30EE{0x12}, sets +0x54 = 0 and a 0x78 timer, and jumps to
 * the 0x126A92 entry; a positive count re-arms through 0x126A22 (self). */
int Obj_126A2A_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126A2A jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126A30 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126A36 jsr 0x11B262 */

    /* 0x126A3C: movea.l (0x30E2,A5),A1 */
    uint32_t a1 = rd32(m, F3_A5 + 0x30E2u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x18u));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x10u));

    /* 0x126A58: subq.w #1 -> bgt 0x126A22 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126A2Au);         /* 0x126A22 */
        return 0;
    }
    /* 0x126A5E: bfclr (0x30EE,A5){0x12:1} -- bit 2 of the long word +0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x12u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x12u % 8u))));
    }
    wr8(m, obj + 0x54u, 0x00u);                    /* 0x126A64 */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x126A6A */
    /* 0x126A70: beq 0x126A92 -- the 0x78 store leaves Z clear, so the branch
     * is NOT taken: fall through to 0x126A72, which installs 0x126A7A. */
    wr32(m, obj + 0x00u, 0x00126A7Au);             /* 0x126A72 */
    return 0;
}
