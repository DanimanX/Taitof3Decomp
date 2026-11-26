/* ROM 0x19D1EE: boss/stage palette cycling & ramp tick (A0 = object).
 * When A5+0x4B38 == -1, sets +0x06 = 2 and switches tick to 0x17A85C.
 * Otherwise, when A5+0x879 bit 0 is clear, queues palette upload at 0x5B1C
 * (7 colours from 0x19D3F0 or 0x19D40C by A5+0x879 bit 1). Every 128 ticks
 * (A5+0x878 & 0x7F == 0), queues 1-colour ramp at 0x5B58 (0x19D428 <->
 * 0x19D42C by A5+0x878 bit 8). Every 64 ticks (A5+0x878 & 0x3F == 0),
 * queues 5- and 4-colour ramps at 0x54DC and 0x54F0 (by A5+0x878 bit 9).
 * Steps two 14-frame (x4 ticks) palette animations at +0x44 -> 0x5B5C
 * (0x19D430) and +0x46 -> 0x5B6C (0x19D520), and re-arms 0x19D1EE. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"

int Fn_19D1EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {      /* 0x19D1EE -> 0x17A84C */
        wr16(m, obj + 0x06u, 2u);                       /* beq 0x17A862 never taken */
        wr32(m, obj, 0x0017A85Cu);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x0879u) & 0x01u) == 0u) {      /* 0x19D1FA */
        a1 = ((rd8(m, F3_A5 + 0x0879u) & 0x02u) != 0u) ? 0x0019D3F0u : 0x0019D40Cu;
        Pal_QueueUpload(m, a1, 0x5B1Cu, 7u);
    }

    d0 = (d0 & 0xFFFF0000u) | (rd16(m, F3_A5 + 0x0878u) & 0x007Fu); /* 0x19D232 */
    if ((uint16_t)d0 == 0u) {
        uint32_t p1 = 0x0019D428u;
        uint32_t p2 = 0x0019D42Cu;
        if ((rd8(m, F3_A5 + 0x0879u) & 0x80u) == 0u) {  /* {8:1} */
            p1 = 0x0019D42Cu;
            p2 = 0x0019D428u;
        }
        a1 = p1;
        (void)Pal_QueueRamp(m, p1, p2, 0x5B58u, 0x7Fu, 1u);
    }

    d0 = (d0 & 0xFFFF0000u) | (rd16(m, F3_A5 + 0x0878u) & 0x003Fu); /* 0x19D280 */
    if ((uint16_t)d0 == 0u) {
        uint32_t p1 = 0x0019D3A8u;
        uint32_t p2 = 0x0019D3BCu;
        if ((rd8(m, F3_A5 + 0x0879u) & 0x40u) == 0u) {  /* {9:1} */
            p1 = 0x0019D3BCu;
            p2 = 0x0019D3A8u;
        }
        (void)Pal_QueueRamp(m, p1, p2, 0x54DCu, 0x3Fu, 5u);

        uint32_t q1 = 0x0019D3D0u;
        uint32_t q2 = 0x0019D3E0u;
        if ((rd8(m, F3_A5 + 0x0879u) & 0x40u) == 0u) {  /* {9:1} */
            q1 = 0x0019D3E0u;
            q2 = 0x0019D3D0u;
        }
        a1 = q1;
        (void)Pal_QueueRamp(m, q1, q2, 0x54F0u, 0x3Fu, 4u);
    }

    /* 0x19D310..0x19D356: anim slot 1 (+0x44) */
    {
        uint32_t v = (uint32_t)rd16(m, obj + 0x44u);
        uint16_t q = (uint16_t)(v / 4u);
        uint16_t rem = (uint16_t)(v % 4u);
        d0 = ((uint32_t)q << 16) | rem;
        if (rem == 0u) {
            if (q == 0x000Eu)
                wr16(m, obj + 0x44u, 0u);
            d0 = (uint32_t)q * 0x10u;
            a1 = 0x0019D430u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
            Pal_QueueUpload(m, a1, 0x5B5Cu, 4u);
        }
        wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + 1u));
    }

    /* 0x19D35A..0x19D3A0: anim slot 2 (+0x46) */
    {
        uint32_t v = (uint32_t)rd16(m, obj + 0x46u);
        uint16_t q = (uint16_t)(v / 4u);
        uint16_t rem = (uint16_t)(v % 4u);
        d0 = ((uint32_t)q << 16) | rem;
        if (rem == 0u) {
            if (q == 0x000Eu)
                wr16(m, obj + 0x46u, 0u);
            d0 = (uint32_t)q * 0x10u;
            a1 = 0x0019D520u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
            Pal_QueueUpload(m, a1, 0x5B6Cu, 4u);
        }
        wr16(m, obj + 0x46u, (uint16_t)(rd16(m, obj + 0x46u) + 1u));
    }

    wr32(m, obj, 0x0019D1EEu);                          /* 0x19D3A4 -> 0x19D1E6 */
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
