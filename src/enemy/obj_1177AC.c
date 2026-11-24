#include "obj_1177AC.h"

/* ROM 0x1177D6: per-tick motion step for the four children this routine
 * leaves behind (0x117826 and siblings).
 *   113652  clr.w (0x34,A0) / (0x1c,A0) / (0x1e,A0) / (0x3a,A0) / (0x3e,A0)
 *   113666  sf (0x4f,A0)
 *   11366A  D0.l = (0xc,A0): {b0,b1} -> signed words at +0x50/+0x52,
 *           {b2,b3} -> unsigned bytes shifted left 8 into +0x20/+0x22
 *   113690  if the +0x22 byte is non-zero, toggle bit 0 of +0x1C (eori.b)
 *   11369A  start from the parent's position and add both velocity words
 * The 0x50/0x52 offsets are one byte below the ROM's 0x51/0x53 because the
 * words overlap (byte writes), so the low byte of each word stays put. */
static void sub_00113652(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0x00u);

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    uint8_t b0 = (uint8_t)(d0 >> 24), b1 = (uint8_t)(d0 >> 16);
    uint8_t b2 = (uint8_t)(d0 >> 8), b3 = (uint8_t)d0;
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)b0);
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)b1); /* bfexts sign-extends */
    wr16(m, obj + 0x20u, (uint16_t)((uint16_t)b2 << 8));
    wr16(m, obj + 0x22u, (uint16_t)b3);
    if (b3 != 0u)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u)); /* eori.b #1 */

    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
}

/* ROM 0x1177AC: launch the 0x117826 state. Hit box 5/10/5/10, one tick of
 * motion, velocities 1.0/-0.5 fixed point, colour 0x3A, then aim: the angle
 * word (+0x20) picks a velocity pair out of the 0x1178AC table. */
int Obj_1177AC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 0x0005u);
    wr16(m, obj + 0x2Eu, 0x000Au);
    wr16(m, obj + 0x30u, 0x0005u);
    wr16(m, obj + 0x32u, 0x000Au);

    sub_00113652(m, obj);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u)); /* subq.w #8 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00050000u);
    wr16(m, obj + 0x42u, 0x000Eu);
    wr8(m, obj + 0x1Du, 0x3Au);

    /* 0x20 += 0x4000 ; neg.w ; -= 0x4000  ==  0xC000 - 0x20 */
    if (rd16(m, obj + 0x22u) != 0u) {
        uint16_t w = (uint16_t)(rd16(m, obj + 0x20u) + 0x4000u);
        w = (uint16_t)(0u - w);
        wr16(m, obj + 0x20u, (uint16_t)(w - 0x4000u));
    }
    /* bfextu D0{16:5} on the zero-extended word: bits 16..20, i.e. (w+8)>>11 */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) + 8u);
    uint32_t idx = (uint32_t)(d0 >> 11) & 0x1Fu;
    wr32(m, obj + 0x10u, rd32(m, 0x001178AAu + idx * 4u));
    wr8(m, obj + 0x1Cu, 0x00u); /* sf */
    wr32(m, obj + 0x00u, 0x00117826u);
    return 0;
}
