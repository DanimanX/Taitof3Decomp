/* ROM 0x10F90A: segmented worm/snake enemy initializer (A0 = object).
 * If A5+0x1D5E == 0x0F (all 4 buffer slots in use), transitions to
 * Obj_StartDying (0x0CEE74). Otherwise spawns 9 segment children (0x110078,
 * group 5, count 2) via 0x0CEFF8 (freeing already-spawned ones and dying if
 * any spawn fails), seeds params (0x0CEC4C @ 0x10FBFE), installs script
 * (0x0CF1D8 @ 0x10FC36), offsets (+0x3A, +0x3E), doubly-links the 9 segments
 * via (+0x50, +0x54), claims a 0x200-byte trail buffer (0x0CEF62), sets
 * segment (+0x10, +0x36, +0x44), fills the 256-word angle trail buffer,
 * sets flags (+0x23=1, +0x4E=0, +0x06=1, +0x34), and installs 0x10F9E0. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CEFF8: spawn D1w (9) children with fn=A1 into A5+0x2862[].
 * On failure frees all allocated slots and returns D0 < 0. */
static int sub_0CEFF8(const f3mem_t *m, f3regs_t *r, uint32_t a1, uint16_t count)
{
    uint32_t obj = r->a[0];
    uint16_t d1w = (uint16_t)(count - 1u);
    uint32_t a2 = F3_A5 + 0x2862u;
    uint32_t a3 = r->a[3];

    r->a[1] = a1;
    r->d[1] = count;

    do {
        uint32_t d0 = Obj_Spawn(m, obj, a1, 5u, 2u, 0u);  /* 0x0CEFFE..0x0CF00C */
        wr32(m, a2, d0);                                 /* 0x0CF016 */
        a2 += 4u;
        r->d[0] = d0;
        r->a[2] = a2;
        if ((int32_t)d0 < 0) {                           /* 0x0CF018 */
            a2 = F3_A5 + 0x2862u;                        /* 0x0CF030 */
            for (;;) {
                uint32_t p = rd32(m, a2);                /* 0x0CF034 */
                a2 += 4u;
                r->d[0] = p;
                r->a[2] = a2;
                if ((int32_t)p < 0)                      /* 0x0CF036 */
                    break;
                Obj_Free(m, p);                          /* 0x0CF03A */
            }
            r->d[1] = setw(r->d[1], d1w);
            r->a[3] = a3;
            return -1;
        }
        a3 = d0;                                         /* 0x0CF01C */
        wr8(m, a3 + 0x4Eu, 0u);                          /* 0x0CF01E */
    } while (d1w-- != 0u);                               /* 0x0CF022 */

    wr8(m, a3 + 0x4Eu, rd8(m, obj + 0x4Eu));             /* 0x0CF026 */
    r->d[0] &= 0xFFFF0000u;                              /* 0x0CF02C clr.w D0w */
    r->d[1] = setw(r->d[1], 0xFFFFu);
    r->a[3] = a3;
    return 0;
}

/* ROM 0x0CEF62: claim free trail buffer slot from A5+0x1D5E, fill 128 longs with -1. */
static void sub_0CEF62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx = (uint16_t)rd8(m, F3_A5 + 0x1D5Eu);    /* 0x0CEF62..0x0CEF64 */
    uint32_t a1 = rd32(m, 0x000CEF98u + (uint32_t)idx * 4u); /* 0x0CEF68 */
    uint16_t bit = rd16(m, 0x000CEFD8u + (uint32_t)idx * 2u); /* 0x0CEF70 */
    uint32_t a2 = rd32(m, obj + 0x0Cu);                  /* 0x0CEF78 */

    wr16(m, a2, 0u);                                     /* 0x0CEF7C */
    wr8(m, a2 + 2u, (uint8_t)bit);                       /* 0x0CEF7E */
    wr32(m, a2 + 4u, a1);                                /* 0x0CEF82 */
    wr8(m, F3_A5 + 0x1D5Eu, (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) | (1u << (bit & 7u)))); /* 0x0CEF86 */

    a2 = a1;                                             /* 0x0CEF8E */
    for (int i = 0x7F; i >= 0; i--) {                    /* 0x0CEF8A..0x0CEF92 */
        wr32(m, a2, 0xFFFFFFFFu);
        a2 += 4u;
    }

    r->d[0] = 0x0000FFFFu;
    r->d[1] = 0xFFFFFFFFu;
    r->a[1] = a1;
    r->a[2] = a2;
}

int Fn_10F90A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd8(m, F3_A5 + 0x1D5Eu) == 0x0Fu) {              /* 0x10F90A..0x10F910 */
        Obj_StartDying(m, obj);
        return 0;
    }

    if (sub_0CEFF8(m, r, 0x00110078u, 9u) < 0) {         /* 0x10F916..0x10F924 */
        Obj_StartDying(m, obj);
        return 0;
    }

    (void)Obj_SeedFromParams(m, obj, 0x0010FBFEu);      /* 0x10F92A..0x10F92E */
    Obj_StartScript(m, obj, 0x0010FC36u);                /* 0x10F934..0x10F938 */
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x001Fu)); /* 0x10F93E */
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 0x0010u)); /* 0x10F944 */

    {
        uint32_t a1 = F3_A5 + 0x2862u;                   /* 0x10F94C */
        uint32_t a2 = obj;                               /* 0x10F950 */
        uint32_t a3 = r->a[3];
        for (int i = 8; i >= 0; i--) {                   /* 0x10F94A..0x10F95E */
            a3 = rd32(m, a1);
            a1 += 4u;
            wr32(m, a2 + 0x54u, a3);
            wr32(m, a3 + 0x50u, a2);
            a2 = a3;
        }
        wr32(m, a2 + 0x54u, 0xFFFFFFFFu);                /* 0x10F962 */
        r->a[3] = a3;
    }

    sub_0CEF62(m, r);                                    /* 0x10F96A */
    uint32_t buf = r->a[1];

    {
        uint16_t d6w = 0u;
        uint16_t d5w = 0u;
        uint32_t a2 = F3_A5 + 0x2862u;                   /* 0x10F97C */
        uint32_t a3 = r->a[3];
        for (int i = 8; i >= 0; i--) {                   /* 0x10F970..0x10F994 */
            d6w = (uint16_t)(d6w + 1u);
            d5w = (uint16_t)((d5w - 0x000Au) & 0x01FFu);
            a3 = rd32(m, a2);
            a2 += 4u;
            wr8(m, a3 + 0x10u, (uint8_t)d6w);
            wr16(m, a3 + 0x36u, d5w);
            wr32(m, a3 + 0x44u, buf);
        }
        r->a[2] = a2;
        r->a[3] = a3;
    }

    {
        int16_t d6w = 0x00B4;
        uint16_t d5w = 0u;
        for (int i = 0xFF; i >= 0; i--) {                /* 0x10F998..0x10F9B8 */
            wr16(m, buf + (uint32_t)d5w, (uint16_t)d6w);
            d6w = (int16_t)(d6w - 8);
            if (d6w < 0)
                d6w = 0x0164;
            d5w = (uint16_t)((d5w - 2u) & 0x01FFu);
        }
        r->d[3] = setw(r->d[3], 0x01FFu);
        r->d[4] = setw(r->d[4], 0x01FFu);
        r->d[5] = setw(r->d[5], d5w);
        r->d[6] = setw(r->d[6], (uint16_t)d6w);
        r->d[7] = setw(r->d[7], 0xFFFFu);
    }

    wr8(m, obj + 0x23u, 1u);                             /* 0x10F9BC */
    wr8(m, obj + 0x4Eu, 0u);                             /* 0x10F9C2 */
    wr8(m, obj + 0x06u, 1u);                             /* 0x10F9C6 */
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u)); /* 0x10F9CC..0x10F9D2 */
    wr32(m, obj, 0x0010F9E0u);                           /* 0x10F9D8 */
    return 0;
}
