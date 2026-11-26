/* ROM 0x132FC6: camera/stage scroll phase tick (A0 = object).
 * Subtracts 0xC000 from long A5+0x30D2; while (int16_t)(A5+0x30D2) > 0x10
 * re-arms 0x132FC6. Once <= 0x10, if (int16_t)(A5+0x30DA) > 0x514 sets
 * A5+0x30EE bit 1 and A5+0x30EF bit 3, clears A5+0x30F2, sets +0x06=0x168 and
 * transitions to 0x133002; else plays hooked sound 0x13, clears A5+0x30EE bit 1,
 * sets +0x06=0x3C and transitions to 0x133046. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_132FC6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x0000C000u); /* 0x132FC6 */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x0010) {                 /* 0x132FCE..0x132FD4 */
        wr32(m, obj, 0x00132FC6u);                                    /* 0x132FBE */
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x0514) {                 /* 0x132FD6..0x132FDC */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x02u)); /* 0x132FE0 {6:1} */
        wr8(m, F3_A5 + 0x30F2u, 0u);                                  /* 0x132FE6 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x132FEC {0xC:1} */
        wr16(m, obj + 0x06u, 0x0168u);                                /* 0x132FF2 (beq 133008 never taken) */
        wr32(m, obj, 0x00133002u);                                    /* 0x132FFA */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                                     /* 0x13301C..0x13302C */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x02u)); /* 0x133030 {6:1} */
    wr16(m, obj + 0x06u, 0x003Cu);                                    /* 0x133036 (beq 13304C never taken) */
    wr32(m, obj, 0x00133046u);                                        /* 0x13303E */
    return 0;
}
