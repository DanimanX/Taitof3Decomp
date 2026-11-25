#include "obj_179DCC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x179AE4: the child's own (dx.w, dy.w) from +0x0C rides on top of the
 * parent's +0x50/+0x54 offsets. `bfexts (0xC,A0){0:16}` reads the high half,
 * `{16:16}` the low half, both sign-extended. */
static void sub_179AE4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    int16_t d0 = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(d0 + rd16(m, a4 + 0x50u)));
    wr16(m, obj + 0x54u, (uint16_t)(d1 + rd16(m, a4 + 0x54u)));
}

/* ROM 0x179B06: pick one of two three-byte frame-script headers with
 * Obj_RandRange(1), latch the animation index into +0x40, point +0x3A at the
 * header body and run that animation's INIT half. */
static int sub_179B06(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00179B3Au; /* 0x179B06: lea (0x32,PC),A1 */
    if (Obj_RandRange(m, 1u) != 0u)
        a1 = 0x00179B8Cu;      /* 0x179B1A: lea (0x70,PC),A1 */

    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);

    /* 0x179B32: movea.l (0x179A5C,D0w*8),A1 ; jsr (A1) -- the INIT half */
    return Obj_AnimInit(m, obj, d0);
}

/* ROM 0x179DCC: the small debris/shrapnel object. Counts itself against the
 * shared A5+0x4B24 budget, keeps its offsets on top of the parent's and
 * advances its two-frame script. */
int Obj_179DCC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179DCC */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x001E) {
        Obj_Free(m, obj); /* 0x179D18 */
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));

    sub_179AE4(m, obj); /* 0x179DDA */
    if (sub_179B06(m, obj) < 0) /* 0x179DDE */
        return -1;

    wr16(m, obj + 0x1Cu, 0u); /* 0x179DE2 */
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x00u, 0x00179C26u); /* 0x179DEA bra 0x179C1E */
    return 0;
}
