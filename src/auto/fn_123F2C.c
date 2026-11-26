/* ROM 0x123F2C: cycle 8-entry palette upload to 0x1784/0x17A4 every 0x5555
 * fixed-point ticks (A0 = object). Frees self and child handles at +0x44..
 * when A5-0x144B bit 3 is set; otherwise re-arms 0x123F2C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"

int Fn_123F2C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    int32_t acc = (int32_t)(rd32(m, obj + 0x3Au) - 0x00005555u); /* 0x123F2C */
    wr32(m, obj + 0x3Au, (uint32_t)acc);
    if (acc < 0)                                                 /* 0x123F34 bge */
        wr16(m, obj + 0x3Au, 7u);                                /* 0x123F38 */

    uint16_t idx = rd16(m, obj + 0x3Au);                         /* 0x123F44 */
    d0 = (d0 & 0xFFFF0000u) | idx;
    uint32_t a1 = 0x00024E40u + (uint32_t)(int32_t)(int16_t)idx * 4u; /* 0x123F3E..0x123F48 */

    Pal_QueueUpload(m, a1, 0x1784u, 8u);                         /* 0x123F4C..0x123F60 */
    Pal_QueueUpload(m, a1, 0x17A4u, 8u);                         /* 0x123F64..0x123F78 */

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x08u) == 0u) {               /* 0x123F7C */
        wr32(m, obj + 0x00u, 0x00123F2Cu);                       /* 0x123F24 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    a1 = obj + 0x44u;                                            /* 0x123F84 */
    for (;;) {
        uint32_t child = rd32(m, a1);                            /* 0x123F88 */
        a1 += 4u;
        d0 = child;
        if ((int32_t)child <= 0)                                 /* 0x123F8A ble */
            break;
        Obj_Free(m, child);                                      /* 0x123F90 */
    }
    Obj_Free(m, obj);                                            /* 0x123F9E */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
