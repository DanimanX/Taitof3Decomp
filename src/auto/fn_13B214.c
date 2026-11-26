/* ROM 0x13B214: segmented serpent/dragon boss initializer (A0 = object).
 * Increments A5+0x32DA, resets fields via 0x0CBD86, sets hitbox (-10..10) x
 * (-10..10), +0x28=1, +0x24=0x64, clears +0x4F/+0x22, copies descriptor
 * (+0x0C)->+0x08 into +0x50, and transitions to 0x13B25E. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_13B214(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr8(m, F3_A5 + 0x32DAu, (uint8_t)(rd8(m, F3_A5 + 0x32DAu) + 1u)); /* 0x13B214 */
    Obj_ResetFields(m, obj);                                           /* 0x13B218 -> 0x0CBD86 */
    wr16(m, obj + 0x2Cu, 0xFFF6u);                                     /* 0x13B21E */
    wr16(m, obj + 0x30u, 0xFFF6u);                                     /* 0x13B224 */
    wr16(m, obj + 0x2Eu, 0x000Au);                                     /* 0x13B22A */
    wr16(m, obj + 0x32u, 0x000Au);                                     /* 0x13B230 */
    wr16(m, obj + 0x28u, 0x0001u);                                     /* 0x13B236 */
    wr16(m, obj + 0x24u, 0x0064u);                                     /* 0x13B23C */
    wr8(m, obj + 0x4Fu, 0u);                                           /* 0x13B242 */

    uint32_t a1 = obj + 0x50u;                                         /* 0x13B246 */
    uint32_t a2 = rd32(m, obj + 0x0Cu);                                /* 0x13B24A */
    wr8(m, a1, rd8(m, a2 + 8u));                                       /* 0x13B24E */
    wr16(m, obj + 0x22u, 0u);                                          /* 0x13B252 */
    wr32(m, obj, 0x0013B25Eu);                                         /* 0x13B256 */

    r->a[1] = a1;
    r->a[2] = a2;
    return 0;
}
