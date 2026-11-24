#include "obj_11B5E6.h"

/* ROM 0x0CCC08: apply the spawn argument's byte offsets and angle, then take
 * the parent's position. D0 = (A0+0x0C):
 *   D0.b[0:8]  (bits 24-31) -> +0x50
 *   D0.b[8:8]  (bits 16-23) -> +0x52
 *   D0.b[16:8] (bits  8-15) -> +0x20   (bfextu adds the offset without the
 *                                            sign extension the other two take) */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u)
                                    + (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 24)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u)
                                    + (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 8) & 0xFFu));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}


/* ROM 0x11B5E6: bullet spawn init (A8 = 2, the 0x0CBFC0 aim helper's radius).
 * Sets the 1x1 box at 1/14 px scale, clears the sprite flip/zoom, speed 0 and
 * -8 into both spawn offsets, then 0x0CCC08 places it on the parent. The
 * lifetime +0x4C is 0x14, +0x24 starts at 1.0, the angle +0x20 is taken from
 * +0x21 (the spare byte) and +0x21 is cleared.
 *
 * Capture note: the difficulty word A5+0x87E reads 0, not the 0x180 the ROM
 * header/`0x87E` uses to mean "normal", so the spawn point lands at
 * (0x380, 0x600) in 68K coordinates rather than off-screen. The arithmetic is
 * reproduced exactly as the instructions do it, on whatever the image holds. */
int Obj_11B5E6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 0x0001u);                              /* 0x11B5E6 */
    wr16(m, obj + 0x30u, 0x0001u);                              /* 0x11B5EC */
    wr16(m, obj + 0x2Eu, 0x000Eu);                              /* 0x11B5F2 */
    wr16(m, obj + 0x32u, 0x000Eu);                              /* 0x11B5F8 */
    wr16(m, obj + 0x1Eu, 0u);                                   /* 0x11B5FE */
    wr16(m, obj + 0x1Cu, 0u);                                   /* 0x11B602 */
    wr16(m, obj + 0x42u, 0u);                                   /* 0x11B606 */
    wr16(m, obj + 0x50u, (uint16_t)-0x0008);                     /* 0x11B60A */
    wr16(m, obj + 0x52u, (uint16_t)-0x0008);                     /* 0x11B610 */
    sub_0CCC08(m, obj);                                         /* 0x11B616 */
    wr16(m, obj + 0x4Cu, 0x0014u);                              /* 0x11B61C */
    wr32(m, obj + 0x24u, 0x00010000u);                          /* 0x11B622 */
    wr8(m, obj + 0x20u, rd8(m, obj + 0x21u));                   /* 0x11B62A move.b (0x21,A0),(0x20,A0) */
    wr8(m, obj + 0x21u, 0x00u);                                 /* 0x11B630 sf */

    /* 0x11B634: move.w (0x87e,A5),D0w ; move.w D0w,D1w ; asr.w #1,D0w ;
     * addi.w #0x380,D0w -> Y ; asl.w #1,D1w ; addi.w #0x600,D1w -> X */
    uint16_t d0 = rd16(m, F3_A5 + 0x87Eu);
    uint16_t d1 = d0;
    d0 = (uint16_t)(d0 >> 1);
    d0 = (uint16_t)(d0 + 0x0380u);
    wr16(m, obj + 0x3Eu, d0);                                   /* 0x11B640 */
    d1 = (uint16_t)(d1 << 1);
    d1 = (uint16_t)(d1 + 0x0600u);
    wr16(m, obj + 0x3Au, d1);                                   /* 0x11B64A */

    wr32(m, obj + 0x00u, 0x0011B656u);                          /* 0x11B64E */
    return 0;
}
