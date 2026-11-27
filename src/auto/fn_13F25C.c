/* ROM 0x13F25C: boss-arena reset. Loads the level record pointer
 * 0x13F9D4[A5+0x32DC] into A5+0x3308, clears the A5+0x331A flag and
 * A5+0x330C, sets A5+0x3310 = 0x10000 / A5+0x3314 = -1, clears the A5-0x144C
 * bits 4/3 and A5+0x1D36 bits 2/7, and frees object groups 3..4. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_13F25C(const f3mem_t *m, f3regs_t *r)
{
    uint16_t idx = (uint16_t)rd16(m, F3_A5 + 0x32DCu);   /* 0x13F266 move.w */

    /* 0x13F260 lea 0x13F9D4 ; 0x13F26A move.l (0,A0,D0w*4),(0x3308,A5).
     * The extension word is 0x0400: D0.w index, scale 4, sign-extended. */
    wr32(m, F3_A5 + 0x3308u,
         rd32(m, 0x0013F9D4u + (uint32_t)(int32_t)(int16_t)idx * 4u));

    wr8(m, F3_A5 + 0x331Au, 0u);                         /* 0x13F270 sf */
    wr32(m, F3_A5 + 0x330Cu, 0u);                        /* 0x13F274 */
    wr32(m, F3_A5 + 0x3310u, 0x00010000u);               /* 0x13F278 */
    wr16(m, F3_A5 + 0x3314u, 0xFFFFu);                   /* 0x13F280 */

    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xEFu)); /* 0x13F286 bclr #4 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xF7u)); /* 0x13F28C bclr #3 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0xFBu)); /* 0x13F292 bclr #2 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0x7Fu)); /* 0x13F298 bclr #7 */

    r->a[7] -= 2u; wr16(m, r->a[7], 2u);                 /* 0x13F29E pea #2 */
    r->a[7] -= 2u; wr16(m, r->a[7], 3u);                 /* 0x13F2A2 pea #3 */
    Obj_ResetGroups(m, 3u, 2u);                          /* 0x13F2A6 -> 0x1060E */
    r->a[7] += 4u;                                       /* 0x13F2AC */

    /* 0x13F2B0 movem.l (SP)+,{D0,A0}: D0 and A0 are restored, D1 and A1 are
     * never written, so every register comes back as it went in. */
    return 0;                                            /* 0x13F2B4 rts */
}
