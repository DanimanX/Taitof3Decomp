/* ROM 0x11CD8C: stage camera/event countdown tick (A0 = object).
 * Sets A5+0x30EE bit 3 (0x10) when +0x06 <= 0x5A and bit 1 (0x40) when +0x06 <= 0x6E;
 * steps camera scroll via 0x11B262 while +0x14 < 0x118; decrements +0x06 and
 * re-arms 0x11CD8C while > 0, else clears A5+0x30EE bit 3, sets +0x06=0x50, and
 * transitions to 0x11CDD8. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B262: step camera scroll (A5+0x30D2, A5+0x30D6) along (+0x20)>>8 via 0x0CBE12. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));                            /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));                            /* 0x11B268 */
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);                      /* 0x11B26E..0x11B272 */
    r->d[0] = setw(r->d[0], ang);

    int16_t spd = (int16_t)rd16(m, obj + 0x42u);                               /* 0x0CBE18 */
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
    uint32_t dx = (uint32_t)((int32_t)spd * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)spd * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
    r->d[2] = dy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));                            /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));                            /* 0x11B280 */
}

int Fn_11CD8C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, obj + 0x06u) <= 0x005A)                               /* 0x11CD8C..0x11CD92 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u));   /* 0x11CD96 {3:1} */

    if ((int16_t)rd16(m, obj + 0x06u) <= 0x006E)                               /* 0x11CD9C..0x11CDA2 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x40u));   /* 0x11CDA6 {1:1} */

    if ((int16_t)rd16(m, obj + 0x14u) < 0x0118)                                /* 0x11CDAC..0x11CDB2 */
        sub_11B262(m, r, obj);                                                 /* 0x11CDB6 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                      /* 0x11CDBC */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                    /* 0x11CDC0 */
        wr32(m, obj, 0x0011CD8Cu);                                             /* 0x11CD84 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x10u));      /* 0x11CDC2 {3:1} */
    wr16(m, obj + 0x06u, 0x0050u);                                             /* 0x11CDC8 (0x11CDCE beq folded) */
    wr32(m, obj, 0x0011CDD8u);                                                 /* 0x11CDD0 */
    return 0;
}
