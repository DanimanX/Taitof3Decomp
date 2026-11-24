#include "obj_125B0C.h"

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


/* ROM 0x125B0C: bullet spawn init with a 2x2 box at 1/22 px scale and speed
 * 0x0A. The colour override +0x4F is cleared, both spawn offsets are -0x0C and
 * 0x0CCC08 places it on the parent. Lifetime +0x4C is 0xC8 and HP +0x24 is
 * 5.0. The angle +0x20 is the spawn argument's angle byte shifted up 8; the
 * sprite flip +0x1C is bit 16 of (that shifted value - 0x4000), and the low
 * byte of the same byte (+0x18 of the argument) is the delay +0x06. */
int Obj_125B0C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 0x0002u);                              /* 0x125B0C */
    wr16(m, obj + 0x2Eu, 0x0016u);                              /* 0x125B12 */
    wr16(m, obj + 0x30u, 0x0002u);                              /* 0x125B18 */
    wr16(m, obj + 0x32u, 0x0016u);                              /* 0x125B1E */
    wr16(m, obj + 0x1Eu, 0u);                                   /* 0x125B24 */
    wr16(m, obj + 0x1Cu, 0u);                                   /* 0x125B28 */
    wr8(m, obj + 0x4Fu, 0x00u);                                 /* 0x125B2C sf */
    wr16(m, obj + 0x42u, 0x000Au);                              /* 0x125B30 */
    wr16(m, obj + 0x50u, (uint16_t)-0x000C);                     /* 0x125B36 */
    wr16(m, obj + 0x52u, (uint16_t)-0x000C);                     /* 0x125B3C */
    sub_0CCC08(m, obj);                                         /* 0x125B42 */
    wr16(m, obj + 0x4Cu, 0x00C8u);                              /* 0x125B48 */

    /* 0x125B4E: move.l (0xC,A0),D0 ; bfextu D0{0x10:0x8},D1 ; lsl.w #8,D1w */
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    uint16_t d1 = (uint16_t)((uint16_t)((uint16_t)(d0 >> 8) & 0xFFu) << 8);
    wr16(m, obj + 0x20u, d1);                                   /* 0x125B58 */
    /* 0x125B5C subi.w: only the low word is subtracted, but the capture
     * shows bit 16 flipping exactly when the shifted angle (0xABCD ..) makes
     * the difference borrow -- i.e. bit 16 of the 32-bit subtraction, whose
     * upper half is 0 after lsl.w cleared it. */
    uint32_t d1_l = ((uint32_t)d1 - 0x4000u) & 0xFFFFu;
    uint16_t flip = (uint16_t)((d1_l >> 15) & 1u);              /* 0x125B60 bfextu D1{0x10:0x1} */
    wr8(m, obj + 0x1Cu, (uint8_t)flip);                         /* 0x125B64 move.b D1b */
    /* 0x125B68 bfextu D0{0x18:0x8} ; 0x125B6C move.w D1w -- the word is the
     * extraction before the swap back, i.e. the argument's low byte in +0x07
     * (the captured argument 0x0E0AFD28 leaves 0x0A in +0x06). */
    wr8(m, obj + 0x07u, (uint8_t)(d0 & 0xFFu));
    wr8(m, obj + 0x06u, 0x00u);
    wr16(m, obj + 0x3Au, 0u);                                   /* 0x125B70 */
    wr16(m, obj + 0x3Eu, 0u);                                   /* 0x125B74 */
    wr32(m, obj + 0x24u, 0x00050000u);                          /* 0x125B78 */
    wr32(m, obj + 0x00u, 0x00125B88u);                          /* 0x125B80 */
    return 0;
}
