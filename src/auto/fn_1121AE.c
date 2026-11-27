/* ROM 0x1121AE: seed a shot/effect object (A0) from the 0x18-byte parameter
 * block at 0x1121B8 (three longs for +0x10/+0x14/+0x18, then six words) after
 * clearing the animation/hit fields with 0x0CBD86. The word that lands in
 * +0x24 gets the byte at A5+0x87E added to it, +0x2C and +0x30 receive only
 * their HIGH word, +0x60 copies the +0x0C field, +0x4F = 0x64, and the object
 * is armed with 0x11221E. D0.w ends as that +0x24 word, A1 as 0x1121D0 (the
 * block's end). */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_1121AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x001121B8u;                    /* ROM 0x1121AE lea (0x8,PC),A1 */
    uint16_t d0w;

    Obj_ResetFields(m, obj);                      /* ROM 0x1121D4 jsr 0x000CBD86 */

    wr32(m, obj + 0x60u, rd32(m, obj + 0x0Cu));   /* ROM 0x1121DE */
    wr32(m, obj + 0x10u, rd32(m, a1));            /* ROM 0x1121E4 move.l (A1)+,(0x10,A0) */
    a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1));            /* ROM 0x1121E8 move.l (A1)+,(0x14,A0) */
    a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));            /* ROM 0x1121EC move.l (A1)+,(0x18,A0) */
    a1 += 4u;

    d0w = (uint16_t)(rd8(m, F3_A5 + 0x087Eu) + rd16(m, a1)); /* ROM 0x1121F0..0x1121F6 */
    a1 += 2u;
    wr16(m, obj + 0x24u, d0w);                    /* ROM 0x1121F8 */
    wr16(m, obj + 0x28u, rd16(m, a1));            /* ROM 0x1121FC move.w (A1)+,(0x28,A0) */
    a1 += 2u;
    wr16(m, obj + 0x2Cu, rd16(m, a1));            /* ROM 0x112200: HIGH word of +0x2C */
    a1 += 2u;
    wr16(m, obj + 0x30u, rd16(m, a1));            /* ROM 0x112204: HIGH word of +0x30 */
    a1 += 2u;
    wr16(m, obj + 0x2Eu, rd16(m, a1));            /* ROM 0x112208 move.w (A1)+,(0x2E,A0) */
    a1 += 2u;
    wr16(m, obj + 0x32u, rd16(m, a1));            /* ROM 0x11220C move.w (A1)+,(0x32,A0) */
    a1 += 2u;

    wr8(m, obj + 0x4Fu, 0x64u);                   /* ROM 0x112210 move.b #0x64,(0x4F,A0) */
    wr32(m, obj, 0x0011221Eu);                    /* ROM 0x112216 move.l #0x11221E,(A0) */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;      /* clr.w/add.w only touch D0.w */
    r->a[1] = a1;                                 /* 0x1121D0 */
    return 0;                                     /* ROM 0x11221C rts */
}
