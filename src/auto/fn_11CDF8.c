/* ROM 0x11CDF8: boss "target camera" phase change (A0 = object).
 * Counts +0x06 down while it stays positive, re-arming 0x11CDF8. On expiry it
 * plays sound 0x13, sets bit 27 of the mode long A5+0x30EE (offset 4), copies
 * the camera position A5+0x30D2 / A5+0x30D6 into +0x14/+0x18, sets speed +0x42
 * = 4, sets A5+0x30F2 bit 1, sets mode bit 25 and clears bit 26, arms the
 * 0x12C-frame timer, picks the two 16.8 wobble rates +0x3A = 0xDC + rand(0x1F)
 * and +0x3C = 0x40 + rand(0x7F) from 0x001D8A, and parks the tick at 0x11CE6E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"      /* Sound_PlayHooked */

#define MODE_L (F3_A5 + 0x30EEu)
#define MODE_B (F3_A5 + 0x30F2u)
#define CAM_X  (F3_A5 + 0x30D2u)
#define CAM_Y  (F3_A5 + 0x30D6u)

int Fn_11CDF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x11CDF8 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                        /* 0x11CDFC bgt */
        wr32(m, obj + 0x00u, 0x0011CDF8u);              /* 0x11CDF0 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                       /* 0x11CDFE..0x11CE0E */
    /* 0x11CE12 bfset (A5+0x30EE){0x4:0x1}: bit 27 of the long. */
    wr32(m, MODE_L, rd32(m, MODE_L) | 0x08000000u);
    wr32(m, obj + 0x14u, rd32(m, CAM_X));               /* 0x11CE18 */
    wr32(m, obj + 0x18u, rd32(m, CAM_Y));               /* 0x11CE1E */
    wr16(m, obj + 0x42u, 0x0004u);                      /* 0x11CE24 */

    wr8(m, MODE_B, (uint8_t)(rd8(m, MODE_B) | 0x02u));  /* 0x11CE2A bset #1 */
    /* 0x11CE30 bfset {0x6:0x1} = bit 25, 0x11CE36 bfclr {0x5:0x1} = bit 26. */
    wr32(m, MODE_L, (rd32(m, MODE_L) | 0x02000000u) & ~0x04000000u);
    wr16(m, obj + 0x06u, 0x012Cu);                      /* 0x11CE3C */

    uint32_t d0 = Rng_NextMem(m);                       /* 0x11CE42 jsr 0x001D8A */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)(d0 & 0x001Fu) + 0x00DCu);
    wr16(m, obj + 0x3Au, (uint16_t)d0);                 /* 0x11CE50 */
    d0 = Rng_NextMem(m);                                /* 0x11CE54 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)(d0 & 0x007Fu) + 0x0040u);
    wr16(m, obj + 0x3Cu, (uint16_t)d0);                 /* 0x11CE62 */

    wr32(m, obj + 0x00u, 0x0011CE6Eu);                  /* 0x11CE66 */
    r->d[0] = d0;
    return 0;
}
