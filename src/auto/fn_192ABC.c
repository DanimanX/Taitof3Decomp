/* ROM 0x192ABC: init object from 0x192B66 table indexed by +0x0C (A0 = object).
 * Clears common fields via 0x17A7FA, sets +0x1D = 0x7A, (+0x50, +0x54) =
 * (0, 0xB0), +0x22.b = +0x0C, looks up 10-byte entry in 0x192B66[+0x0C], runs
 * 0x179A5C anim init, adds entry dx/dy/dt to +0x50/+0x54/+0x3E, and arms
 * 0x192B1C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_192ABC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                                 /* 0x192ABC */
    wr8(m, obj + 0x1Du, 0x7Au);                         /* 0x192AC2 */
    wr16(m, obj + 0x50u, 0u);                           /* 0x192AC8 */
    wr16(m, obj + 0x54u, 0x00B0u);                      /* 0x192ACE */

    uint32_t d0 = rd32(m, obj + 0x0Cu);                 /* 0x192AD4 */
    wr8(m, obj + 0x22u, (uint8_t)d0);                   /* 0x192AD8 */

    d0 = (uint32_t)(uint16_t)d0 * 0x0Au;                /* 0x192AE2 */
    uint32_t a2 = 0x00192B66u + (uint32_t)(int32_t)(int16_t)d0; /* 0x192ADC..0x192AE6 */
    uint32_t a1 = rd32(m, a2);                          /* 0x192AEA */
    a2 += 4u;

    uint16_t idx = rd16(m, a1);                         /* 0x192AEC */
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                          /* 0x192AEE */
    wr32(m, obj + 0x3Au, a1);                           /* 0x192AF2 */
    if (Obj_AnimInit(m, obj, idx) < 0)                  /* 0x192AF6..0x192B00 */
        return -1;
    if (idx == 0u)
        a1 = rd32(m, obj + 0x3Au) + 6u;                 /* 0x179550 leaves A1 = +0x3A + 6 */

    uint16_t dx = rd16(m, a2); a2 += 2u;                /* 0x192B02 */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + dx)); /* 0x192B04 */
    uint16_t dy = rd16(m, a2); a2 += 2u;                /* 0x192B08 */
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + dy)); /* 0x192B0A */
    uint16_t dt = rd16(m, a2); a2 += 2u;                /* 0x192B0E */
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + dt)); /* 0x192B10 */

    wr32(m, obj, 0x00192B1Cu);                          /* 0x192B14 */
    r->d[0] = (d0 & 0xFFFF0000u) | dt;
    r->a[1] = a1;
    r->a[2] = a2;
    return 0;
}
