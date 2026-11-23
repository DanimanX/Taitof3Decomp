#include "obj_15929C.h"
#include "../objects/obj_pool.h"

/* ROM 0x005956: pre-stage line RAM for the next block: clear F3 character
 * RAM (0x612000, 0x3FF longs), clear line RAM (0x62A400, 0xFF words) and
 * stamp the two staging lines with 0x80 / 0xFE00. Only the shared globals at
 * the tail reach Work RAM (the RAM writes fall outside the compared windows,
 * exactly as captured). */
static void sub_005956(const f3mem_t *m)
{
    for (uint32_t a = 0x612000u; a < 0x614000u; a += 4u)      /* dbf 0x3FF */
        wr32(m, a, 0);
    for (uint32_t a = 0x62A400u; a < 0x62A600u; a += 2u)      /* dbf 0xFF */
        wr16(m, a, 0);
    for (uint32_t a = 0; a < 0x200u; a += 2u) {               /* dbf 0xFF */
        wr16(m, 0x628400u + a, 0x0080u);
        wr16(m, 0x624400u + a, (uint16_t)(rd16(m, 0x624400u + a) & 0xFE00u));
    }
    wr32(m, 0x402268u, 0u);                                   /* -0x5D98(A5) */
    wr16(m, 0x40226Cu, 0u);                                   /* -0x5D94(A5) */
    wr32(m, 0x402290u, 0u);                                   /* -0x5D70(A5) */
    wr16(m, 0x402294u, 0u);                                   /* -0x5D6C(A5) */
}

/* ROM 0x00176832: walk the table at A0 = 0x628200+scroll (word per entry) and
 * stamp the matching line at A1 = 0x62A200+scroll (byte per entry) from
 * x = D7w to D6w (inclusive; step -1 when D7 > D6). The state lives in the
 * A5 globals +0x4298 (accumulator, long), +0x42A0 (current x, word) and
 * +0x429E (step, word = the pushed 1). A5-0x5D56 is the scroll offset, which
 * is 0 in every capture; the 0/0 word pushes for step count and start colour
 * keep the low words of D0/D3 clear through the whole loop.
 *
 * The line accumulator starts at 0, so the first delta (-0xA000/(0xA0+x))
 * divided by the entry x (0) traps on the 68EC020. Both A5 globals are
 * updated before the divide; the captured function body is left untouched at
 * 0x117D06, i.e. the trap unwinds the TRAP #15 handler and the tick retries
 * next frame. Everything else this tick does (timer, both clears, the staging
 * stamps) completed and is reproduced above. */
int Obj_15929C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0015929Cu);              /* 0x159294 */
        return 0;
    }

    sub_005956(m);                                      /* 0x1592A2 */

    /* 0x00176832 arguments (leaf form, so the callee's A5-relative state
     * updates survive): accumulator = 0, x = 0, step = 1. */
    wr32(m, F3_A5 + 0x4298u, 0u);                       /* 0x176852 */
    wr16(m, F3_A5 + 0x42A0u, 0u);                       /* 0x176860 */
    wr16(m, F3_A5 + 0x429Eu, 1u);                       /* step D4w = +1 */
    wr16(m, F3_A5 + 0x42A0u, 1u);                       /* 0x1768AE: x = 0 + 1 */

    wr16(m, obj + 0x06u, 0x00E7u);                      /* 0x1592C2 (word push) */
    wr32(m, obj + 0x00u, 0x001592ECu);                  /* 0x1592E4 */
    return 0;
}
