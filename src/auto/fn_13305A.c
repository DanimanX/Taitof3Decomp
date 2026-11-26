/* ROM 0x13305A: boss vertical camera scroll / phase transition tick (A0 = object).
 * Adds 0x4000 to A5+0x30D6, decrements +0x06 and re-arms 0x13305A while > 0.
 * On expiry, if A5+0x30DA <= 0x320 plays Sound_PlayHooked 0x13, sets A5+0x30EE
 * bits 0xB and 0xC, sets A5+0x30F2 = 7 and arms 0x1331C6; otherwise sets
 * A5+0x30EE bits 2..5, sets +0x06 = 0x12C, and arms 0x13309A. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_13305A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + 0x00004000u); /* 0x13305A */
    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);               /* 0x133062 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                                    /* 0x133066 */
        wr32(m, obj, 0x0013305Au);                                    /* 0x133052 */
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x0320) {                /* 0x133068..0x13306E */
        Sound_PlayHooked(m, 0x0013u);                                 /* 0x133198..0x1331A8 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x10u)); /* 0x1331AC: bfset {0xb:1} */
        wr8(m, F3_A5 + 0x30F2u, 0x07u);                               /* 0x1331B2 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x1331B8: bfset {0xc:1} */
        wr32(m, obj, 0x001331C6u);                                    /* 0x1331BE */
        return 0;
    }

    /* 0x133072..0x133084: bfset (0x30EE,A5) bits 2,3,4,5 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x20u | 0x10u | 0x08u | 0x04u));
    wr16(m, obj + 0x06u, 0x012Cu);                                    /* 0x13308A */
    /* 0x133090: beq.b 0x1330A0 never taken (0x012C != 0) */
    wr32(m, obj, 0x0013309Au);                                        /* 0x133092 */
    return 0;
}
