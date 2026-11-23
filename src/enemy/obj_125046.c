#include "obj_125046.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"

/* ROM 0x0CC2BC: probability gate. D0w = threshold; the scaled threshold is
 * (0x180 - A5+0x87E) * D0w, shifted and byte-swapped; divu gives the counter
 * quotient in the low word and the remainder in the high word (`swap D0`).
 * Callers test the LOW word of D0, so the quotient is what they see. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint32_t d1 = (uint32_t)(uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    d1 = ((uint32_t)(uint16_t)d0w * d1) & 0xFFFFFFFFu;  /* mulu.w */
    d1 = (d1 << 8) & 0xFFFFFFFFu;                       /* lsl.l #8 */
    d1 = ((d1 << 16) | (d1 >> 16));                     /* swap */
    uint16_t thr = (uint16_t)d1;
    if (thr == 0u)
        thr = 1u;                                       /* 0x0CC2D0 */
    uint32_t num = rd16(m, F3_A5 + 0x0878u);            /* divu.w D1w,D0 */
    uint32_t quot = num / thr;
    uint32_t rem = num % thr;
    uint32_t d0 = (rem << 16) | (quot & 0xFFFFu);
    d0 = ((d0 << 16) | (d0 >> 16));                     /* swap D0 */
    return (uint16_t)(d0 & 0xFFFFu);
}

/* ROM 0x125046: a parented decorative object. While the parent's +0x59 stays
 * clear it animates from +0x0C, publishes itself at 0x11B06E, copies the
 * parent's colour byte +0x1D, integrates through 0x11B09A, and re-arms
 * 0x12503E. A set parent byte frees the object (0x1250AE -> 0x105EC).
 *
 * Capture note: the parent is 0x4128E0 and its +0x59 lands on the ROM long at
 * 0x122F86; that is static ROM, so the captured byte reads non-zero and every
 * pair takes the free path. */
int Obj_125046_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x125046: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x1250AE */
    if (rd8(m, rd32(m, obj + 0x08u) + 0x59u) != 0u) {
        Obj_Free(m, obj);                              /* 0x1250AE -> 0x0105EC */
        return 0;
    }

    /* 0x125052: movea.l (0xC,A0),A1 ; jsr 0x0CBD52 */
    Obj_Animate(m, obj, rd32(m, obj + 0x0Cu));
    (void)Obj_11B06E_Tick(m, obj);                     /* 0x12505C */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x125066 */
    (void)Obj_11B09A_Tick(m, obj);                     /* 0x12506C */

    /* 0x125072: bftst (0x30EE,A5){6:1} ; beq 0x1250AC */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) != 0u) {
        /* 0x12507C: move.w #0x22 ; jsr 0x0CC2BC ; tst.w D0w ; bne 0x1250AC */
        if (sub_0CC2BC(m, 0x0022u) == 0u) {
            /* 0x12508C: spawn 0x0CCE0E in group 9, 2 groups, arg 0x16108000 */
            (void)Obj_Spawn(m, obj, 0x000CCE0Eu, 9u, 2u, 0x16108000u);
        }
    }

    wr32(m, obj + 0x00u, 0x00125046u);                 /* 0x1250AC -> 0x12503E */
    return 0;
}
