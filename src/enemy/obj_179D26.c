#include "obj_179D26.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"

/* ROM 0x179D26: entrance tick. Budget-gate against the shared A5+0x4B24
 * counters, ride on the parent's offset, seed the RAM frame list at 0x179D5E
 * and run its index-0 INIT, clear the flip/zoom words, install 0x179C26. */

/* ROM 0x179AE4: (dx.w, dy.w) at +0x0C rides on the parent's +0x50/+0x54 */
static void sub_179AE4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    int16_t d0 = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(d0 + rd16(m, a4 + 0x50u)));
    wr16(m, obj + 0x54u, (uint16_t)(d1 + rd16(m, a4 + 0x54u)));
}

/* ROM 0x179D18: drop the slot counter and free the object */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj); /* 0x0105EC */
}

int Obj_179D26_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179D26: cmpi.w #0x1E,(0x4B24,A5) ; bgt -> 0x179D18 */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        sub_179D18(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u)); /* 0x179D2E */

    sub_179AE4(m, obj); /* 0x179D32 */

    /* 0x179D36: movea.l #0x179D5E,A1 ; move.w (A1)+,D0w ; move.w D0w,(0x40,A0) ;
     * move.l A1,(0x3A,A0) -- the frame list is a RAM copy at 0x179D5E
     * ({0, 5, 0x0005A174 | ...}, eight 6-byte frames ending in 0xFFFF). */
    {
        uint32_t a1 = 0x00179D5Eu;
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, d0);
        wr32(m, obj + 0x3Au, a1);

        /* 0x179D46: movea.l (0x0,A1,D0w*8),A1 ; jsr (A1) -- INIT slot */
        if (Obj_AnimInit(m, obj, d0) < 0)
            return -1;
    }

    wr16(m, obj + 0x1Cu, 0u); /* 0x179D52 */
    wr16(m, obj + 0x1Eu, 0u); /* 0x179D56 */
    wr32(m, obj + 0x00u, 0x00179C26u); /* 0x179D5A bra.w 0x179C1E */
    return 0;
}
