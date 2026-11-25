#include "obj_17EA0A.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "obj_17960A.h"

/* ROM 0x17EA0A: a burst child the first time it runs. Installs behaviour
 * 0x17EBF6, seeds its stats from 0x17EB36, starts animation 0 (record
 * 0x17EB4C), re-derives velocity/angle from +0x0C and hands over to
 * 0x17EA7E. */

/* ROM 0x184888: seed the stats from the record at a1; the first word pair is
 * divided by ten (divul.l). */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x17EA0A */
int Obj_17EA0A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17EA0A: behaviour record 0x17EBF6 {0, 0x17EB4A, dur = 1} */
    {
        uint32_t a1 = 0x0017EBF6u;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
        wr16(m, obj + 0x4Au, 0u);
    }

    /* 0x17EA1E: bra.w 0x17EA3A */
    sub_184888(m, obj, 0x0017EB36u);                 /* 0x17EA40 */

    /* 0x17EA46: movea.l #0x17EB4C,A1; move.w (A1)+,D0w; +0x40 = D0w;
     * +0x3A = A1; A1 = (0x179A5C)[D0w]; jsr (A1) -- the INIT half, idx = 2 */
    {
        uint32_t a1 = 0x0017EB4Cu;                  /* movea.l #0x17EB4C,A1 */
        uint16_t d0w = rd16(m, a1); a1 += 2u;       /* move.w (A1)+,D0w */
        wr16(m, obj + 0x40u, d0w);
        wr32(m, obj + 0x3Au, a1);
        /* 0x179A5C[D0w] is the INIT half: the word {2} at 0x17EB4C, whose
         * init is 0x17960A */
        if (rd32(m, 0x00179A5Cu + (uint32_t)d0w * 8u) == 0x0017960Au)
            Obj_17960A_Tick(m, obj);
        else
            return -1;
    }

    /* 0x17EA62: bsr 0x17BD68 -- parent position + sign-extended offsets */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17EA66: move.w (0xE,A0),D0w; mulu.w #0x20,D0; move.b D0b,(0x20,A0) */
    wr8(m, obj + 0x20u, (uint8_t)(uint16_t)(rd16(m, obj + 0x0Eu) * 0x20u));

    /* 0x17EA76: move.l #0x17EA7E,(A0) */
    wr32(m, obj + 0x00u, 0x0017EA7Eu);
    return 0;
}
