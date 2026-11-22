#include "obj_11BDAA.h"

/* ROM 0x11BDAA: initialise the 0x11BE14 counter object: bump the parent's
 * stage counter (+0x22), clear the display/state words, park it off screen
 * at (-0x32,-0x32), give it 2x2 zoom, a 0x0D box, a 0x27100000 timer and
 * 0x10000, then loop to 0x11BE14 (see work/disasm/bundle_11BDAA.s). */
int Obj_11BDAA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u));   /* addq.w #1 */

    wr16(m, obj + 0x1Cu, 0u);            /* clr.w */
    wr16(m, obj + 0x1Du, 0u);            /* clr.w */
    wr16(m, obj + 0x1Eu, 0u);            /* clr.w */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);             /* sf */
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x14u, 0xFFCEu);       /* move.w #-0x32 */
    wr16(m, obj + 0x18u, 0xFFCEu);
    wr16(m, obj + 0x2Cu, 2u);
    wr16(m, obj + 0x30u, 2u);
    wr16(m, obj + 0x2Eu, 0x0Du);
    wr16(m, obj + 0x32u, 0x0Du);
    wr32(m, obj + 0x28u, 0x27100000u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr16(m, obj + 0x42u, 0x8Cu);
    wr16(m, obj + 0x06u, 0u);

    /* 0x11BE0C: move.l #0x11BE14,(A0) */
    wr32(m, obj + 0x00u, 0x0011BE14u);
    return 0;
}
