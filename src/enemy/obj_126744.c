#include "obj_126744.h"
#include "obj_11B288.h"

/* ROM 0x126744: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader. On the
 * +0x06 countdown the tail sets bit A5+0x30EE{0x6} and a 0x190 timer, then
 * falls through to the same expiry block as 0x126772 (0x12678A): clear bit
 * A5+0x30EE{0xB} and either jump to 0x12679A (A5+0x30DA > 0x3E8) or run
 * 0x12695C and hand over to 0x126976. */
int Obj_126744_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126744 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12674A jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126750 jsr 0x11B262 */

    /* 0x126756: subq.w #1 -> bgt 0x12673C tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126744u);         /* 0x12673C */
        return 0;
    }
    /* 0x12675C: bfset (0x30EE,A5){0x6:1} -> bit 1 of +0x30EE */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x06u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x06u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x0190u);                 /* 0x126762 */
    /* 0x126768: beq 0x12678A -- the 0x190 store leaves Z clear, so fall
     * through to 0x12676A which re-arms this routine's own entry (0x126772)
     * with the 0x190 timer. */
    wr32(m, obj + 0x00u, 0x00126772u);             /* 0x12676A */
    return 0;
}
