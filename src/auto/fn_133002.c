#include "fn_auto.h"
/* ROM 0x133002: stage-scroll camera phase tick (A0 = object). Decrements the
 * A0+0x06 timer; while it is > 0 it stays armed. When it runs out: if the
 * round counter (int16_t)A5+0x30DA <= 0x514, plays hooked sound 0x13, clears
 * A5+0x30EE bit 6, sets A0+0x06 = 0x3C and transitions to 0x133046; otherwise
 * sets A5+0x30EF bits 3 and 4, A5+0x30F2 = 6 and transitions to 0x132F60. */
#include "../enemy/obj_E8B72.h"

int Fn_133002(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x133002 subq.w #1,(6,A0) */
    wr16(m, obj + 0x06u, cnt);

    if ((int16_t)cnt > 0) {           /* 0x133006 bgt.b 0x132FFA */
        wr32(m, obj, 0x00133002u);    /* 0x132FFA */
        return 0;
    }

    /* 0x133008 bfclr (0x30EE,A5){6:1} -> bit 1 of the byte */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x02u));

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x0514) { /* 0x13300E..0x133014 ble.w 0x13301C */
        Sound_PlayHooked(m, 0x0013u);                  /* 0x13301C..0x13302C */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x02u)); /* 0x133030 {6:1} */
        wr16(m, obj + 0x06u, 0x003Cu);                 /* 0x133036 (beq 0x13304C never taken) */
        wr32(m, obj, 0x00133046u);                     /* 0x13303E */
        return 0;
    }

    /* 0x133018 bra.w 0x132F46 */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x10u)); /* 0x132F46 {0xB:1} */
    wr8(m, F3_A5 + 0x30F2u, 6u);                                        /* 0x132F4C */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x132F52 {0xC:1} */
    wr32(m, obj, 0x00132F60u);                                          /* 0x132F58 */
    return 0;
}
