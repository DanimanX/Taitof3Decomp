/* ROM 0x14F1A6: perspective line-zoom/offset table stepper (A0 = object).
 * Increments +0x44; if > 0 frees itself (0x0105EC). Otherwise updates 232 lines
 * (0xE7..0) in A5+0x3BE0 (slope), A5+0x3DE0 (Z accum), 0x62A600 + A5-0x5D56
 * (line zoom), and 0x628600 + A5-0x5D56 (line Y-offset), then re-arms 0x14F1A6. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_14F1A6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x44u) + 1u); /* 0x14F1A6 */
    wr16(m, obj + 0x44u, cnt);

    if ((int16_t)cnt > 0) {                              /* 0x14F1AA */
        Obj_Free(m, obj);                                /* 0x14F1AE..0x14F1BA */
        return 0;
    }

    uint32_t a1 = F3_A5 + 0x3BE0u;                       /* 0x14F11C */
    uint32_t a2 = F3_A5 + 0x3DE0u;                       /* 0x14F120 */
    int32_t line_off = (int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x14F130..0x14F134 */
    uint32_t a3 = 0x0062A600u + (uint32_t)line_off;
    uint32_t a4 = 0x00628600u + (uint32_t)line_off;

    uint32_t d0 = 0u;
    uint32_t d1 = 0u;
    for (int16_t d7w = 0x00E7; d7w >= 0; d7w--) {        /* 0x14F138..0x14F196 */
        uint32_t off2 = (uint32_t)(int32_t)d7w * 2u;
        uint32_t off4 = (uint32_t)(int32_t)d7w * 4u;

        d0 = ((uint32_t)rd16(m, a1 + off2) << 3) + 0x0000F000u; /* 0x14F13C..0x14F144 */
        wr32(m, a2 + off4, rd32(m, a2 + off4) - d0);     /* 0x14F14A */
        int16_t d4w = (int16_t)rd16(m, a2 + off4);       /* 0x14F14E */

        d0 = 0xFFFF6000u;                                /* 0x14F152 -0xA000 */
        int16_t d1w = (int16_t)(0x00A0 + d4w);           /* 0x14F158..0x14F15E */
        d1 = (uint32_t)(uint16_t)d1w;
        if (d1w != 0) {                                  /* 0x14F160 divs.w D1w,D0 */
            int32_t q = (int32_t)d0 / (int32_t)d1w;
            int32_t rem = (int32_t)d0 % (int32_t)d1w;
            if (q >= -32768 && q <= 32767)
                d0 = ((uint32_t)(uint16_t)rem << 16) | (uint16_t)q;
        }
        d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x0100u)); /* 0x14F164 */

        int16_t d2w = (rd16(m, 0x004022A8u) != 0u) ? 0x003F : 0x0009; /* 0x14F168..0x14F176 */
        d2w = (int16_t)(d2w - 3);                        /* 0x14F17A */
        int32_t d2 = ((int32_t)(int16_t)(uint16_t)d0 * (int32_t)d2w) << 6; /* 0x14F17C..0x14F17E */
        {
            int32_t q = d2 / 0x32;                       /* 0x14F180 divs.w #0x32,D2 */
            int32_t rem = d2 % 0x32;
            if (q >= -32768 && q <= 32767)
                d2 = (int32_t)(((uint32_t)(uint16_t)rem << 16) | (uint16_t)q);
        }
        uint16_t outw = (uint16_t)((uint16_t)d2 ^ 0x003Fu); /* 0x14F184 */
        if (m->gfx) {
            wr16(m, a3 + off2, outw);                    /* 0x14F188 */
            wr8(m, a4 + off2, (uint8_t)d0);              /* 0x14F18C */
        }
        wr16(m, a1 + off2, (uint16_t)(rd16(m, a1 + off2) - 0x0040u)); /* 0x14F190 */
    }

    wr32(m, obj, 0x0014F1A6u);                           /* 0x14F19E */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
