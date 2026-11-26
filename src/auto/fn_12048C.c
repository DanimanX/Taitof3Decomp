/* ROM 0x12048C: unpack sprite/offset/hitbox record at A1 into object A0.
 * Reads signed byte offsets (+0x50, +0x52), sprite ptr (+0x10), and signed
 * hitbox bytes (+0x2C, +0x30, +0x2E, +0x32), leaving A1 advanced by 10 bytes
 * and D0.w equal to the last sign-extended byte. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_12048C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = r->a[1];
    uint16_t d0w;

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x12048C..0x12048E */
    wr16(m, obj + 0x50u, d0w);                              /* 0x120490 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x120494..0x120496 */
    wr16(m, obj + 0x52u, d0w);                              /* 0x120498 */
    wr32(m, obj + 0x10u, rd32(m, a1));                      /* 0x12049C */
    a1 += 4u;
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x1204A0..0x1204A2 */
    wr16(m, obj + 0x2Cu, d0w);                              /* 0x1204A4 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x1204A8..0x1204AA */
    wr16(m, obj + 0x30u, d0w);                              /* 0x1204AC */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x1204B0..0x1204B2 */
    wr16(m, obj + 0x2Eu, d0w);                              /* 0x1204B4 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);          /* 0x1204B8..0x1204BA */
    wr16(m, obj + 0x32u, d0w);                              /* 0x1204BC */

    r->d[0] = setw(r->d[0], d0w);
    r->a[1] = a1;
    return 0;
}
