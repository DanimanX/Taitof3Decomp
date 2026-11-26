/* ROM 0x1035A2: parent-linked shield/wing sub-object tick (A0 = object).
 * Copies parent (+0x08) +0x3A/+0x3E/+0x1D/+0x22 to self; if +0x22 == 0 calls
 * Obj_StartDying (0x0CEE74). Otherwise syncs (+0x14 = +0x3A + 0x1F, +0x18 =
 * +0x3E) and dispatches state +0x06 (0..5 via 0x1035E6), using 0x0CFA86 for
 * sprite frames and 0x0D4A12 for eased Y-offset curve. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x0D4A12: D0 = swap32(tbl_0D4A5A[D1w] * k), D1w = k (for k in 0..50). */
static void sub_0D4A12(const f3mem_t *m, uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t idx = (uint16_t)*d1_io;
    uint32_t val = rd32(m, 0x000D4A5Au + (uint32_t)(int32_t)(int16_t)idx * 4u);
    uint16_t k = (uint16_t)*d0_io;
    *d1_io = (*d0_io & 0xFFFF0000u) | k;
    uint32_t prod = val * (uint32_t)k;
    *d0_io = (prod << 16) | (prod >> 16);
}

int Fn_1035A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x1035A2 */

    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x3Au));            /* 0x1035A6 */
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x3Eu));            /* 0x1035AC */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));              /* 0x1035B2 */
    uint8_t active = rd8(m, a1 + 0x22u);
    wr8(m, obj + 0x22u, active);                          /* 0x1035B8 */
    if (active == 0u) {                                   /* 0x1035BE */
        Obj_StartDying(m, obj);                           /* 0x0CEE74 */
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));           /* 0x1035C4 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x001Fu)); /* 0x1035CA */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));           /* 0x1035D0 */

    uint8_t st = rd8(m, obj + 0x06u);                     /* 0x1035D6..0x1035D8 */
    d0 = setw(d0, st);

    switch (st) {                                         /* 0x1035DC -> table 0x1035E6 */
    case 0:
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u)); /* 0x1035FE */
        {
            uint8_t p = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x0Fu); /* 0x103602 */
            wr8(m, obj + 0x1Eu, p);
            if (p == 0xFFu)                               /* 0x103608 */
                wr8(m, obj + 0x06u, 1u);                  /* 0x103612 */
        }
        d0 = 0x0000001Du;                                 /* 0x103618 */
        d1 = (d1 & 0xFFFF0000u) | (uint8_t)~rd8(m, obj + 0x1Eu); /* 0x10361A..0x103620 */
        sub_0D4A12(m, &d0, &d1);                          /* 0x103622 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x103628 */
        break;

    case 1:
        a1 = 0x001036E0u;                                 /* 0x10362E */
        sub_0CFA86(m, obj, &a1, &d0);                     /* 0x103632 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u)); /* 0x103638 */
        {
            uint8_t p = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x0Fu); /* 0x10363E */
            wr8(m, obj + 0x1Eu, p);
            if (p == 0u) {                                /* 0x103644 */
                wr8(m, obj + 0x23u, 6u);                  /* 0x103648 */
                wr8(m, obj + 0x06u, 2u);                  /* 0x10364E */
            }
        }
        break;

    case 2:
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u)); /* 0x103656 */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x10365C */
            wr8(m, obj + 0x23u, t);
            if (t == 0u)                                  /* 0x103660 */
                wr8(m, obj + 0x06u, 3u);                  /* 0x103664 */
        }
        break;

    case 3:
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u)); /* 0x10366C */
        {
            uint8_t p = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x0Fu); /* 0x103672 */
            wr8(m, obj + 0x1Eu, p);
            if (p == 0xFFu)                               /* 0x103678 */
                wr8(m, obj + 0x06u, 4u);                  /* 0x103682 */
        }
        break;

    case 4:
        a1 = 0x001036D6u;                                 /* 0x10368A */
        sub_0CFA86(m, obj, &a1, &d0);                     /* 0x10368E */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u)); /* 0x103694 */
        {
            uint8_t p = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x0Fu); /* 0x103698 */
            wr8(m, obj + 0x1Eu, p);
            if (p == 0u) {                                /* 0x10369E */
                wr8(m, obj + 0x23u, 6u);                  /* 0x1036A2 */
                wr8(m, obj + 0x06u, 5u);                  /* 0x1036A8 */
            }
        }
        d0 = 0x0000001Du;                                 /* 0x1036AE */
        d1 = (d1 & 0xFFFF0000u) | (uint8_t)~rd8(m, obj + 0x1Eu); /* 0x1036B0..0x1036B6 */
        sub_0D4A12(m, &d0, &d1);                          /* 0x1036B8 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x1036BE */
        break;

    case 5:
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u)); /* 0x1036C4 */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x1036C8 */
            wr8(m, obj + 0x23u, t);
            if (t == 0u)                                  /* 0x1036CC */
                wr8(m, obj + 0x06u, 0u);                  /* 0x1036D0 */
        }
        break;

    default:
        return -1;
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
