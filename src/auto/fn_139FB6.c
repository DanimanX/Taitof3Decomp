/* ROM 0x139FB6: spawn-record initialiser (A0 = object). Resets the per-life
 * fields via Obj_11B2F6 (0x11B2F6), loads +0x50/+0x52/+0x2C..+0x32 from the
 * 8-byte record at (+0x0C) via 0x11B31C, advances +0x0C past that record,
 * sets bit 4 of +0x34.b (contact-damage flag), clears the two angle bytes
 * +0x20/+0x21 and re-arms the object with 0x139FE4. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_139FB6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_11B2F6(m, obj);                              /* 0x139FB6 -> 0x11B2F6 */

    uint32_t a1 = rd32(m, obj + 0x0Cu);              /* 0x139FBC movea.l (0xc,A0),A1 */

    /* 0x139FC0 -> 0x11B31C, kept register-level because A1/D0 fall through. */
    wr16(m, obj + 0x50u, rd16(m, a1)); a1 += 2u;     /* 0x11B31C */
    wr16(m, obj + 0x52u, rd16(m, a1)); a1 += 2u;     /* 0x11B320 */
    wr16(m, obj + 0x54u, 0u);                        /* 0x11B324 */
    wr16(m, obj + 0x56u, 0u);                        /* 0x11B328 */
    uint16_t d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
    wr16(m, obj + 0x2Cu, d0w);                       /* 0x11B32C..0x11B330 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
    wr16(m, obj + 0x30u, d0w);                       /* 0x11B334..0x11B338 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
    wr16(m, obj + 0x2Eu, d0w);                       /* 0x11B33C..0x11B340 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
    wr16(m, obj + 0x32u, d0w);                       /* 0x11B344..0x11B348 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;         /* ext.w only touches D0.w */
    r->a[1] = a1;

    wr32(m, obj + 0x0Cu, a1);                        /* 0x139FC6 move.l A1,(0xc,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x139FCA bset.b #4 */
    wr8(m, obj + 0x20u, 0u);                         /* 0x139FD0 */
    wr8(m, obj + 0x21u, 0u);                         /* 0x139FD6 */
    wr32(m, obj + 0x00u, 0x00139FE4u);               /* 0x139FDC */
    return 0;
}
