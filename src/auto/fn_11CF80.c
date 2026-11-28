/* ROM 0x11CF80: boss "appear" countdown for the 0x11CE6E tick (A0 = object).
 * +0x06 counts down one per call; while it stays > 0 the tick only re-arms
 * 0x11CF80 through the 0x11CF78 thunk (move.l #0x11CF80,(A0)). When it expires: clear A5+0x30EE{1:1} (0x40), then the
 * shared 0x11CE2A entry sets A5+0x30F2 bit 1, sets A5+0x30EE{6:1} (0x02),
 * clears A5+0x30EE{5:1} (0x04), reloads +0x06 = 0x12C, rolls two RNG values
 * (0x1D8A) into +0x3A (0xDC..0xFB) and +0x3C (0x40..0xBF) and hands the tick
 * to 0x11CE6E. */
#include "fn_auto.h"

int Fn_11CF80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x11CF80 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                                  /* 0x11CF84 bgt */
        wr32(m, obj, 0x0011CF80u);                                  /* 0x11CF78 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x40u)); /* 0x11CF86 {1:1} */

    /* 0x11CE2A: shared re-entry of the 0x11CE6E script */
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x02u)); /* 0x11CE2A bset #1 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x02u)); /* 0x11CE30 {6:1} */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x04u)); /* 0x11CE36 {5:1} */
    wr16(m, obj + 0x06u, 0x012Cu);                                        /* 0x11CE3C */

    uint32_t d0 = Rng_NextMem(m);                                   /* 0x11CE42 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)(d0 & 0x001Fu) + 0x00DCu); /* 0x11CE48/0x11CE4C */
    wr16(m, obj + 0x3Au, (uint16_t)d0);                             /* 0x11CE50 */

    d0 = Rng_NextMem(m);                                            /* 0x11CE54 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)(d0 & 0x007Fu) + 0x0040u); /* 0x11CE5A/0x11CE5E */
    wr16(m, obj + 0x3Cu, (uint16_t)d0);                             /* 0x11CE62 */

    r->d[0] = d0;
    wr32(m, obj, 0x0011CE6Eu);                                      /* 0x11CE66 */
    return 0;                                                       /* 0x11CE6C rts */
}
