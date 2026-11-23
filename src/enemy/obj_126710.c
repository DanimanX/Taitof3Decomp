#include "obj_126710.h"
#include "obj_11B288.h"

/* ROM 0x126710: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader. On the
 * +0x06 countdown the tail sets +0x54 = 4, bit A5+0x30EE{0xB} and a 0xC8
 * timer, then hands over to 0x126744. */
int Obj_126710_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126710 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126716 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12671C jsr 0x11B262 */

    /* 0x126722: subq.w #1 -> bgt 0x126708 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126710u);         /* 0x126708 */
        return 0;
    }
    /* 0x126728 */
    wr8(m, obj + 0x54u, 0x04u);
    /* 0x12672E: bfset (0x30EE,A5){0xB:1} -> bit 4 of the long word +0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Bu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Bu % 8u))));
    }
    wr16(m, obj + 0x06u, 0x00C8u);                 /* 0x126734 */
    /* 0x12673A: beq 0x12675C -- the 0xC8 store leaves Z clear, so fall
     * through to 0x12673C, which installs 0x126744. */
    wr32(m, obj + 0x00u, 0x00126744u);             /* 0x12673C */
    return 0;
}
