/* ROM 0x194814: boss sub-part initializer (A0 = object).
 * Clears common fields via 0x17A7FA, stores +0x0C.b in +0x22, indexes the
 * 0x12-byte table at 0x194964 by +0x0C.w to populate (+0x16, +0x1A, +0x1C),
 * initializes 0x179A5C animation, stores remaining table pointer in +0x0C,
 * and transitions to 0x194860. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_194814(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                                  /* 0x194814 */

    uint32_t d0 = rd32(m, obj + 0x0Cu);                  /* 0x19481A */
    wr8(m, obj + 0x22u, (uint8_t)d0);                    /* 0x19481E */

    uint32_t a1 = 0x00194964u;                           /* 0x194822 */
    d0 = (uint32_t)(uint16_t)d0 * 0x0012u;               /* 0x194828 mulu.w #0x12,D0 */
    uint32_t a2 = a1 + (int32_t)(int16_t)(uint16_t)d0;   /* 0x19482C */

    wr16(m, obj + 0x16u, rd16(m, a2)); a2 += 2u;         /* 0x194830 */
    wr16(m, obj + 0x1Au, rd16(m, a2)); a2 += 2u;         /* 0x194834 */
    wr16(m, obj + 0x1Cu, rd16(m, a2)); a2 += 2u;         /* 0x194838 */

    a1 = rd32(m, a2); a2 += 4u;                          /* 0x19483C */
    uint16_t idx = rd16(m, a1); a1 += 2u;                /* 0x19483E */
    d0 = (d0 & 0xFFFF0000u) | idx;
    wr16(m, obj + 0x40u, idx);                           /* 0x194840 */
    wr32(m, obj + 0x3Au, a1);                            /* 0x194844 */

    if (Obj_AnimInit(m, obj, idx) < 0)                   /* 0x194848..0x194852 */
        return -1;
    if (idx == 0u)
        a1 = rd32(m, obj + 0x3Au) + 6u;                  /* 0x179550 advances A1 by 6 */

    wr32(m, obj + 0x0Cu, a2);                            /* 0x194854 */
    wr32(m, obj, 0x00194860u);                           /* 0x194858 */

    r->d[0] = d0;
    r->a[1] = a1;
    r->a[2] = a2;
    return 0;
}
