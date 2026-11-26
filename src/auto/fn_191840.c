/* ROM 0x191840: boss/effect instance spawn-capped init tick (A0 = object).
 * Increments A5+0x4B92; if > 10, decrements it back and frees self (0x0105EC).
 * Otherwise clears fields via 0x17A7FA, copies +0x0C/+0x0E words to +0x50/+0x54,
 * sets sprite 0x2DFB0, flags +0x1C=0x200, zoom +0x1F=0x60 / +0x1E=0xF0, and
 * transitions to 0x191882. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_191840(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(rd16(m, F3_A5 + 0x4B92u) + 1u);        /* 0x191840 */
    wr16(m, F3_A5 + 0x4B92u, (uint16_t)cnt);

    if (cnt > 0x0A) {                                              /* 0x191844..0x19184A */
        wr16(m, F3_A5 + 0x4B92u, (uint16_t)(cnt - 1));             /* 0x1918FA */
        Obj_Free(m, obj);                                          /* 0x1918FE..0x191906 (0x0105EC) */
        return 0;
    }

    Obj_17A7FA(m, obj);                                            /* 0x19184E */
    wr16(m, obj + 0x50u, rd16(m, obj + 0x0Cu));                    /* 0x191854 */
    wr16(m, obj + 0x54u, rd16(m, obj + 0x0Eu));                    /* 0x19185A */
    wr32(m, obj + 0x10u, 0x0002DFB0u);                             /* 0x191860 */
    wr16(m, obj + 0x1Cu, 0x0200u);                                 /* 0x191868 */
    wr8(m, obj + 0x1Fu, 0x60u);                                    /* 0x19186E */
    wr8(m, obj + 0x1Eu, 0xF0u);                                    /* 0x191874 */
    wr32(m, obj, 0x00191882u);                                     /* 0x19187A */
    return 0;
}
