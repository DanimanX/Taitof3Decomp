/* ROM 0x110DCC: multi-part boss/serpent head init (A0 = object, A0+0x08 = parent).
 * Copies parent (+0x14, +0x18) + (table[+0x4E], 0xA3), dies if A5+0x1D5E == 0x0F
 * or 12-child allocation (0x0CEFF8) fails, seeds params (0x0CEC4C) and script
 * (0x0CF1D8), links the 12 children into a doubly-linked chain via +0x54/+0x50,
 * reserves a ring buffer (0x0CEF62), queues palette 0x17D00 (0x001660), and
 * arms 0x110EC6. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CEFF8: spawn D1w (12) child objects at 0x111128, storing ptrs at A5+0x2862. */
static int sub_0CEFF8(const f3mem_t *m, f3regs_t *r, uint32_t a1, int16_t count)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = F3_A5 + 0x2862u;                  /* 0x0CEFFA */
    uint32_t a3 = 0u;
    int16_t d1 = (int16_t)(count - 1);              /* 0x0CEFF8 */

    for (; d1 >= 0; d1--) {                         /* 0x0CEFFE..0x0CF022 */
        uint32_t d0 = Obj_Spawn(m, obj, a1, 5u, 2u, 0u);
        wr32(m, a2, d0);
        a2 += 4u;
        if ((int32_t)d0 < 0) {                      /* 0x0CF018 -> 0x0CF030 */
            a2 = F3_A5 + 0x2862u;
            for (;;) {
                d0 = rd32(m, a2);
                a2 += 4u;
                if ((int32_t)d0 < 0)
                    break;
                Obj_Free(m, d0);
            }
            r->d[0] = d0;
            r->d[1] = setw(r->d[1], (uint16_t)d1);
            return -1;
        }
        a3 = d0;
        wr8(m, a3 + 0x4Eu, 0u);                     /* 0x0CF01E */
    }
    wr8(m, a3 + 0x4Eu, rd8(m, obj + 0x4Eu));        /* 0x0CF026 */
    r->d[0] &= 0xFFFF0000u;                         /* 0x0CF02C */
    r->d[1] = setw(r->d[1], 0xFFFFu);
    return 0;
}

/* ROM 0x0CEF62: claim ring buffer slot from A5+0x1D5E bitmask, init +0x0C, fill 0xFFFFFFFF. */
static uint32_t sub_0CEF62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx = rd8(m, F3_A5 + 0x1D5Eu);         /* 0x0CEF62..0x0CEF64 */
    uint32_t a1 = rd32(m, 0x000CEF98u + (uint32_t)idx * 4u); /* 0x0CEF68 */
    uint16_t bit = rd16(m, 0x000CEFD8u + (uint32_t)idx * 2u); /* 0x0CEF70 */
    uint32_t a2 = rd32(m, obj + 0x0Cu);             /* 0x0CEF78 */

    wr16(m, a2, 0u);                                /* 0x0CEF7C */
    wr8(m, a2 + 0x02u, (uint8_t)bit);               /* 0x0CEF7E */
    wr32(m, a2 + 0x04u, a1);                        /* 0x0CEF82 */
    wr8(m, F3_A5 + 0x1D5Eu,
        (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) | (uint8_t)(1u << (bit & 7u)))); /* 0x0CEF86 */

    for (uint32_t i = 0; i < 0x80u; i++)            /* 0x0CEF8A..0x0CEF92 */
        wr32(m, a1 + i * 4u, 0xFFFFFFFFu);

    r->d[0] = 0x0000FFFFu;
    r->d[1] = 0xFFFFFFFFu;
    r->a[1] = a1;
    return a1;
}

int Fn_110DCC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x110DCC */
    r->a[1] = a1;
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));      /* 0x110DD0 */

    uint8_t idx = rd8(m, obj + 0x4Eu);              /* 0x110DD6..0x110DD8 */
    uint16_t d0w = rd16(m, 0x0011103Eu + (uint32_t)idx * 2u); /* 0x110DDC */
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w)); /* 0x110DE4 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));      /* 0x110DE8 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x00A3u)); /* 0x110DEE */
    wr16(m, obj + 0x4Cu, d0w);                      /* 0x110DF4 */

    if (rd8(m, F3_A5 + 0x1D5Eu) == 0x0Fu) {         /* 0x110DF8..0x110DFE */
        Obj_StartDying(m, obj);
        return 0;
    }

    r->a[1] = 0x00111128u;                          /* 0x110E04 */
    r->d[1] = 0x0000000Cu;                          /* 0x110E08 */
    if (sub_0CEFF8(m, r, 0x00111128u, 12) < 0) {    /* 0x110E0A..0x110E12 */
        Obj_StartDying(m, obj);
        return 0;
    }

    Obj_SeedFromParams(m, obj, 0x0010C8E2u);        /* 0x110E18..0x110E1C */
    idx = rd8(m, obj + 0x4Eu);                      /* 0x110E22..0x110E24 */
    a1 = rd32(m, 0x00111044u + (uint32_t)idx * 4u); /* 0x110E28 */
    r->a[1] = a1;
    Obj_StartScript(m, obj, a1);                    /* 0x110E30 */

    wr8(m, obj + 0x23u, 0x32u);                     /* 0x110E36 */
    wr32(m, obj + 0x48u, 0u);                       /* 0x110E3C */
    wr8(m, obj + 0x06u, 1u);                        /* 0x110E40 */

    /* 0x110E46..0x110E64: link 12 children via +0x54/+0x50 */
    uint32_t a2 = obj;
    for (int i = 0; i < 12; i++) {
        uint32_t a3 = rd32(m, F3_A5 + 0x2862u + (uint32_t)i * 4u);
        wr32(m, a2 + 0x54u, a3);
        wr32(m, a3 + 0x50u, a2);
        wr8(m, a3 + 0x06u, 1u);
        a2 = a3;
    }
    wr32(m, a2 + 0x54u, 0xFFFFFFFFu);

    uint32_t ring = sub_0CEF62(m, r);               /* 0x110E6C */

    /* 0x110E72..0x110E9E: init child positions and ring ptrs */
    uint16_t d6w = 0u;
    uint32_t px = rd32(m, obj + 0x14u);
    uint32_t py = rd32(m, obj + 0x18u);
    for (int i = 0; i < 12; i++) {
        uint32_t a3 = rd32(m, F3_A5 + 0x2862u + (uint32_t)i * 4u);
        wr32(m, a3 + 0x14u, px);
        wr32(m, a3 + 0x18u, py);
        wr16(m, a3 + 0x10u, d6w);
        wr32(m, a3 + 0x36u, ring);
        d6w = (uint16_t)((d6w - 0x000Eu) & 0x01FFu);
    }

    Pal_QueueUpload(m, 0x00017D00u, 0x5900u, 0x0020u); /* 0x110EA2..0x110EB4 */
    wr32(m, obj, 0x00110EC6u);                      /* 0x110EBE */
    return 0;                                       /* 0x110EC4 */
}
