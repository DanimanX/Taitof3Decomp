/* ROM 0x121BB0: camera/boss scroll-sweep phase tick (A0 = object).
 * While (int8_t)(A5+0x326E) < 6: advances camera scroll (A5+0x30D2/+0x30D6)
 * along (+0x20)>>8 via 0x11B262, subtracts 0x80 from +0x20.w, decrements +0x06
 * and re-arms 0x121BB0 while > 0; on 0 clears A5+0x30F2, sets +0x06 = 0x3C, and
 * transitions to 0x121BE0. Once A5+0x326E >= 6: plays hooked sound 0x13,
 * clears A5+0x30EE.l, computes +0x06 = 0x78 - 0x0CBEB0(0x3C), picks random
 * +0x3A in [0xCD..0xEC] and +0x3C in [0x40..0xBF] via Rng_NextMem, and
 * transitions to 0x121DAC. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: difficulty-scaled speed word from A5+0x087E. */
static uint32_t sub_0CBEB0(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    int16_t d1w = (int16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0..0x0CBEB6 */
    r->d[1] = setw(r->d[1], (uint16_t)d1w);
    uint32_t prod = (uint32_t)((int32_t)(int16_t)d0w * (int32_t)d1w); /* 0x0CBEBA */
    prod <<= 4;                                                        /* 0x0CBEBC asl.l #4 */
    return (prod << 16) | (prod >> 16);                                /* 0x0CBEBE swap */
}

/* ROM 0x11B262: step camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));                   /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));                   /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;                         /* 0x11B26E..0x11B272 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    Obj_MoveByAngle(m, obj, ang);                                     /* 0x11B274 -> 0x0CBE12 */
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)((int32_t)speed * (int32_t)c);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));                   /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));                   /* 0x11B280 */
}

int Fn_121BB0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6) {                       /* 0x121BB0..0x121BB6 */
        Sound_PlayHooked(m, 0x0013u);                                 /* 0x121D54..0x121D64 */
        wr32(m, F3_A5 + 0x30EEu, 0u);                                 /* 0x121D68 */

        uint32_t d0 = sub_0CBEB0(m, r, 0x003Cu);                      /* 0x121D6C..0x121D70 */
        uint16_t d0w = (uint16_t)(0u - (uint16_t)((uint16_t)d0 - 0x0078u)); /* 0x121D76..0x121D7A */
        wr16(m, obj + 0x06u, d0w);                                    /* 0x121D7C */

        d0 = Rng_NextMem(m);                                          /* 0x121D80 */
        d0w = (uint16_t)((d0 & 0x001Fu) + 0x00CDu);                   /* 0x121D86..0x121D8A */
        wr16(m, obj + 0x3Au, d0w);                                    /* 0x121D8E */

        d0 = Rng_NextMem(m);                                          /* 0x121D92 */
        d0w = (uint16_t)((d0 & 0x007Fu) + 0x0040u);                   /* 0x121D98..0x121D9C */
        wr16(m, obj + 0x3Cu, d0w);                                    /* 0x121DA0 */
        r->d[0] = setw(d0, d0w);

        wr32(m, obj, 0x00121DACu);                                    /* 0x121DA4 */
        return 0;
    }

    sub_11B262(m, r);                                                 /* 0x121BBA */
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0080u)); /* 0x121BC0 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);             /* 0x121BC6 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                           /* 0x121BCA -> 0x121BA8 */
        wr32(m, obj, 0x00121BB0u);
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 0u);                                      /* 0x121BCC */
    wr16(m, obj + 0x06u, 0x003Cu);                                    /* 0x121BD2 */
    wr32(m, obj, 0x00121BE0u);                                        /* 0x121BD8 */
    return 0;
}
