#include "fn_auto.h"
/* ROM 0x180670: frame-script op that starts a timed direction/pose animation
 * (A0 = obj). While +0x06 stays > 0 the tick re-arms 0x180670. When it
 * expires it plays hooked sound 0x2A, loads the 0x16-byte parameter record
 * 0x1806CA into the object through 0x184888 (which also divides +0x24 by 10),
 * takes the animation index 0x000A from 0x1806E0 into +0x40, points +0x3A at
 * the record body, runs the 0x179A5C init half for it (index 10 = 0x179614 ->
 * 0x1798D4), installs the script 0x1807A2 at +0x44 (+0x48 = its +0x04 word,
 * +0x4A = 0) and re-arms 0x17BE1C. */
#include "../enemy/obj_E8B72.h"   /* Sound_PlayHooked (0x13F238 + 0x0039D8) */

int Fn_180670(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t timer = rd16(m, obj + 0x06u);        /* 0x180670 subq.w #1,(0x6,A0) */
    wr16(m, obj + 0x06u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {                     /* 0x180674 bgt.b 0x180668 */
        wr32(m, obj, 0x00180670u);                /* 0x180668 */
        return 0;                                 /* 0x18066E rts */
    }

    Sound_PlayHooked(m, 0x002Au);                 /* 0x180676..0x180686 */

    /* 0x18068A A1 = 0x1806CA ; 0x180690 jsr 0x184888 */
    wr32(m, obj + 0x24u, 0u);                     /* 0x184888 */
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 8u);
    wr8(m, obj + 0x35u, 0u);
    wr16(m, obj + 0x64u, 0u);
    uint32_t a1 = 0x001806CAu;
    wr16(m, obj + 0x1Cu, rd16(m, a1));       a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1));       a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1));       a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1));       a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));       a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));       a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1));       a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1));       a1 += 4u;
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du)); /* 0x1848BE */
    {
        uint32_t d = rd32(m, obj + 0x24u) / 10u; /* 0x1848C4 divul.l #0xA */
        wr32(m, obj + 0x24u, d);
    }

    /* 0x180696 A1 = 0x1806E0 ; 0x18069C move.w (A1)+,D0w */
    a1 = 0x001806E0u;
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                    /* 0x18069E */
    wr32(m, obj + 0x3Au, a1);                     /* 0x1806A2 */

    /* 0x1806A6..0x1806B0: movea.l (0x179A5C,D0w*8),A1 ; jsr (A1).
     * The record at 0x1806E0 always holds index 0x000A, whose init half is
     * 0x179614 (movem-free): +0x3E = 1 then the 0x1798D4 body. */
    wr16(m, obj + 0x3Eu, 1u);                     /* 0x179614 */
    uint16_t d0h = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3); /* 0x1798D4 */
    uint16_t d0m = (uint16_t)(d0h * 6u);          /* 0x1798E4 mulu.w #6,D0 */
    uint32_t a2 = rd32(m, obj + 0x3Au) + (uint32_t)(int32_t)(int16_t)d0m; /* 0x1798E8 */
    wr32(m, obj + 0x10u, rd32(m, a2));            /* 0x1798EC move.l (A1)+,(0x10,A0) */
    a2 += 4u;
    uint16_t pal = rd16(m, a2);                   /* 0x1798F0 move.w (A1)+,D0w */
    wr8(m, obj + 0x1Cu, (uint8_t)pal);            /* 0x1798F2 move.b D0b,(0x1C,A0) */

    r->d[0] = ((uint32_t)d0m & 0xFFFF0000u) | pal; /* mulu high half + (A1)+ word */

    a1 = 0x001807A2u;                             /* 0x1806B2 movea.l #0x1807A2,A1 */
    wr32(m, obj + 0x44u, a1);                     /* 0x1806B8 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));       /* 0x1806BC */
    wr16(m, obj + 0x4Au, 0u);                     /* 0x1806C2 clr.w (0x4A,A0) */
    wr32(m, obj, 0x0017BE1Cu);                    /* 0x1806C6 bra 0x17BE14 */
    r->a[1] = a1;
    return 0;                                     /* 0x17BE1A rts */
}
