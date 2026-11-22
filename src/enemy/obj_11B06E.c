#include "obj_11B06E.h"

/* ROM 0x0011B06E: publish the object to the two hazard lists (A5+0x8FE and
 * A5+0x906) unless the pause flag A5+0x30F3 is set or +0x2C is zero, then run
 * the hit-flash update at 0x11C32C (see work/disasm/bundle_11B06E.s). */
int Obj_11B06E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11B06E: tst.b (0x30f3,A5); bne 0x11B092 */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        /* 0x11B07E: movea.l (0x8fe,A5),A1; move.l A0,(A1)+; move.l A1,(0x8fe,A5) */
        uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
        /* 0x11B088: movea.l (0x906,A5),A1; move.l A0,(A1)+; move.l A1,(0x906,A5) */
        a1 = rd32(m, F3_A5 + 0x906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x906u, a1 + 4u);
    }

    /* 0x11B092: jsr 0x11C32C -- hit flash. This is a 4-instruction leaf, so
     * it is inlined below (same form as src/enemy/obj_CD256.c). */
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 6u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 8u);
        } else {
            wr8(m, obj + 0x1Du, 6u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
    return 0;
}
