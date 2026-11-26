/* ROM 0x0C1B0A: delayed gfx RAM tile fill (A0 = object).
 * Decrements +0x06.w; while > 0 returns at 0x0C1B0A. When <= 0, fills
 * 0x90 words at 0x627600 + rd16(A5 - 0x5D56) with 0xEEEE (0x176208) and
 * arms 0x0C1B3C. */
#include "fn_auto.h"

/* ROM 0x00176208: fill words [start..end] at base + rd16(A5-0x5D56) with val. */
static void fill_gfx_words(const f3mem_t *m, uint32_t base, uint16_t start,
                           uint16_t end, uint16_t val)
{
    uint32_t dst = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    int16_t i = (int16_t)start;
    int16_t lim = (int16_t)end;
    do {
        wr16(m, dst + (uint32_t)(int32_t)i * 2u, val);       /* 0x176224 */
        i++;
    } while (i <= lim);                                      /* 0x17622C */
}

int Fn_0C1B0A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x0C1B0A */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                           /* 0x0C1B0E */
        wr32(m, obj, 0x000C1B0Au);                           /* 0x0C1B02 */
        return 0;
    }

    fill_gfx_words(m, 0x00627600u, 0x0000u, 0x008Fu, 0xEEEEu); /* 0x0C1B26 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xEEEEu;
    wr32(m, obj, 0x000C1B3Cu);                               /* 0x0C1B34 */
    return 0;
}
