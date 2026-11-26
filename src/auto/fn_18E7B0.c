/* ROM 0x18E7B0: boss attack cycle wait/retrigger tick (A0 = object).
 * Decrements +0x06 while > 0 (re-arming 0x18E7B0). On 0, decrements loop
 * counter +0x64: if <= 0 sets +0x06 = 0x14 and arms 0x18E7CE (0x18E7C4 beq
 * folded); else sets +0x62 = 0x1E, +0x42 = 0x300 scaled by 0x184984, and
 * either sets +0x06 = 2 and arms 0x18E196 (when A5+0x4B38 == 0) or sets
 * +0x06 = 4 and arms 0x18E750. */
#include "fn_auto.h"

/* ROM 0x184984: scale +0x42.w by difficulty byte 0x1849D6[(A5+0x87E >> 4) & 0xF]. */
static void sub_184984(const f3mem_t *m, uint32_t obj)
{
    uint16_t sel = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) >> 4) & 0x0Fu);
    uint8_t scale = rd8(m, 0x001849D6u + sel);
    int16_t d1 = (int16_t)rd16(m, obj + 0x42u);
    d1 = (int16_t)(d1 / 0x64);
    d1 = (int16_t)(d1 * (int16_t)scale);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + (uint16_t)d1));
}

int Fn_18E7B0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x18E7B0 */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {                                /* 0x18E7B4 -> 0x18E7A8 */
        wr32(m, obj, 0x0018E7B0u);
        return 0;
    }

    uint16_t loops = (uint16_t)(rd16(m, obj + 0x64u) - 1u); /* 0x18E7B6 */
    wr16(m, obj + 0x64u, loops);
    if ((int16_t)loops > 0) {                            /* 0x18E7BA -> 0x18E724 */
        wr16(m, obj + 0x62u, 0x001Eu);                   /* 0x18E724 */
        wr16(m, obj + 0x42u, 0x0300u);                   /* 0x18E72A */
        sub_184984(m, obj);                              /* 0x18E730 */
        if (rd16(m, F3_A5 + 0x4B38u) == 0u) {            /* 0x18E736..0x18E73C */
            wr16(m, obj + 0x06u, 2u);                    /* 0x18E186 (0x18E18C beq folded) */
            wr32(m, obj, 0x0018E196u);                   /* 0x18E18E */
        } else {
            wr16(m, obj + 0x06u, 4u);                    /* 0x18E740 (0x18E746 beq folded) */
            wr32(m, obj, 0x0018E750u);                   /* 0x18E748 */
        }
        return 0;
    }

    wr16(m, obj + 0x06u, 0x0014u);                       /* 0x18E7BE (0x18E7C4 beq folded) */
    wr32(m, obj, 0x0018E7CEu);                           /* 0x18E7C6 */
    return 0;
}
