/* ROM 0x11FEDE: boss articulated part tick (A0 = object, A0+0x08 = parent).
 * Frees self (0x0105EC) if parent->+0x59 != 0. Otherwise drains +0x24 into
 * shared boss HP A5+0x30DA via 0x11AE62 (d0=0x186A0, d1=2), updates hit
 * spark via 0x11C096, unpacks sprite/hitbox/offsets via 0x11FE2A -> 0x12048C,
 * toggles +0x34 (if 0, sets bit 4 and calls 0x11B06E; else clears +0x1D),
 * anchors (+0x14, +0x18) to camera scroll A5+0x30D2/+0x30D6 + (+0x50, +0x52),
 * and re-arms 0x11FEDE. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static uint16_t max16(uint16_t a, uint16_t b)
{
    return (int16_t)a < (int16_t)b ? b : a;
}

static uint16_t min16(uint16_t a, uint16_t b)
{
    return (int16_t)a > (int16_t)b ? b : a;
}

/* ROM 0x11AE62: fold obj+0x24 into shared boss HP A5+0x30DA, crediting receiver on <= 0. */
static void sub_11AE62_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                            uint32_t d0, uint32_t d1)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)               /* 0x11AE62 */
        return;
    int32_t d2 = (int32_t)rd32(m, obj + 0x24u);                /* 0x11AE6A */
    if (d2 == 0)
        return;
    wr32(m, obj + 0x24u, 0u);                                  /* 0x11AE72 */
    uint32_t a1 = rd32(m, obj + 0x36u);                        /* 0x11AE76 */
    r->a[1] = a1;
    uint8_t k = rd8(m, a1 + 0x22u);                            /* 0x11AE7A..0x11AE9E */
    if (k != 0u && k != 4u) {
        d2 >>= (d1 & 63u);
        if (k != 1u && k != 5u)
            d2 >>= 1;
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2); /* 0x11AEA6 */
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)                 /* 0x11AEAA */
        return;
    a1 = Obj_0CBDDE(m, obj);                                   /* 0x11AEB2 */
    r->a[1] = a1;
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);             /* 0x11AEB8 */
}

/* ROM 0x11C096: propagate hit flag +0x35 to parent and queue hit spark in 0x40B0F6. */
static void sub_11C096_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint8_t hit = rd8(m, obj + 0x35u);                         /* 0x11C096 */
    if (hit == 0u)
        goto done;

    {
        uint32_t a1 = rd32(m, obj + 0x08u);                    /* 0x11C09E */
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | hit)); /* 0x11C0A6 */
    }
    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u)                   /* 0x11C0AA */
        goto done;

    {
        uint16_t now = rd16(m, F3_A5 + 0x0878u);               /* 0x11C0B4 */
        uint16_t d0w = (uint16_t)(now - rd16(m, F3_A5 + 0x311Cu));
        wr16(m, F3_A5 + 0x311Cu, now);                         /* 0x11C0BC */
        if ((int16_t)d0w < 2) {                                /* 0x11C0C6 */
            uint32_t prev = rd32(m, F3_A5 + 0x3116u);          /* 0x11C0CA */
            wr32(m, F3_A5 + 0x3116u, obj);                     /* 0x11C0CE */
            if (prev == obj) {                                 /* 0x11C0D4 */
                uint16_t c = (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u);
                wr16(m, F3_A5 + 0x311Au, c);                   /* 0x11C0D8 */
                if ((int16_t)c <= 3)                           /* 0x11C0E2 */
                    goto done;
            }
        }
    }

    wr16(m, F3_A5 + 0x311Au, 0u);                              /* 0x11C0E6 */
    uint32_t a2 = 0x0040B0F6u;                                 /* 0x11C0EA */
    int found = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t b = rd8(m, a2);
        wr8(m, a2, (uint8_t)(b | 0x80u));                      /* 0x11C0F4 */
        if ((b & 0x80u) == 0u) {
            found = 1;
            break;
        }
        a2 += 4u;                                              /* 0x11C0FC */
    }
    if (!found)
        goto done;

    {
        uint32_t a1 = rd32(m, obj + 0x36u);                    /* 0x11C106 */
        uint16_t d0w, d1w;
        uint32_t d1 = r->d[1];
        if (rd8(m, a1 + 0x22u) == 6u || rd8(m, obj + 0x35u) == 2u) { /* 0x11C10A..0x11C11A */
            d1 = Rng_NextMem(m) & 0xFFFFu;                     /* 0x11C18A..0x11C192 */
            uint16_t dv = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
            if (dv != 0u)
                d1 = ((d1 / dv) << 16) | (d1 % dv);            /* 0x11C19C..0x11C19E */
            d1w = (uint16_t)((uint16_t)d1 + rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));
            d1 = setw(d1, d1w);

            uint32_t d0 = Rng_NextMem(m) & 0xFFFFu;            /* 0x11C1A8..0x11C1AE */
            uint16_t dv2 = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
            if (dv2 != 0u)
                d0 = ((d0 / dv2) << 16) | (d0 % dv2);
            d0w = (uint16_t)((uint16_t)d0 + rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));
        } else {
            uint16_t ox = rd16(m, obj + 0x14u), ax = rd16(m, a1 + 0x14u);
            uint16_t oy = rd16(m, obj + 0x18u), ay = rd16(m, a1 + 0x18u);
            d0w = max16((uint16_t)(rd16(m, obj + 0x2Cu) + ox), (uint16_t)(rd16(m, a1 + 0x2Cu) + ax));
            d1w = min16((uint16_t)(rd16(m, obj + 0x2Eu) + ox), (uint16_t)(rd16(m, a1 + 0x2Eu) + ax));
            d0w = (uint16_t)((int16_t)(uint16_t)(d0w + d1w) >> 1);
            d1w = max16((uint16_t)(rd16(m, obj + 0x30u) + oy), (uint16_t)(rd16(m, a1 + 0x30u) + ay));
            uint16_t d2w = min16((uint16_t)(rd16(m, obj + 0x32u) + oy), (uint16_t)(rd16(m, a1 + 0x32u) + ay));
            d1w = (uint16_t)((int16_t)(uint16_t)(d1w + d2w) >> 1);
        }
        d0w = (uint16_t)(d0w - rd16(m, F3_A5 + 0x30D2u));      /* 0x11C1C8 */
        d1w = (uint16_t)(d1w - rd16(m, F3_A5 + 0x30D6u));      /* 0x11C1CC */
        r->d[1] = setw(d1, d1w);
        uint32_t v = rd32(m, a2);
        v = (v & ~(0x3FFu << 21)) | (((uint32_t)d0w & 0x3FFu) << 21); /* 0x11C1D0 */
        v = (v & ~(0x1FFu << 12)) | (((uint32_t)d1w & 0x1FFu) << 12); /* 0x11C1D4 */
        wr32(m, a2, v);
    }

done:
    wr8(m, obj + 0x35u, 0u);                                   /* 0x11C1D8 */
}

/* ROM 0x11FE2A -> 0x12048C: select pose/sprite/hitbox record. */
static void sub_11FE2A_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                        /* 0x11FE2A */
    uint32_t d0 = rd8(m, F3_A5 + 0x3262u);                     /* 0x11FE2E..0x11FE30 */
    if (d0 != 0u) {                                            /* 0x11FE34 */
        wr8(m, obj + 0x34u, 0xFFu);                            /* 0x11FE38 */
        a1 = rd32(m, a1 - 4u + d0 * 4u);                       /* 0x11FE3C */
    } else {
        a1 += 0x10u;                                           /* 0x11FE44 */
        wr32(m, obj + 0x10u, rd32(m, a1 + 2u));                /* 0x11FE48 */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) != 0u) {         /* 0x11FE4E */
            wr8(m, obj + 0x34u, 0xFFu);                        /* 0x11FE98 */
            a1 = rd32(m, obj + 0x10u);                         /* 0x11FE9C */
        } else if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {  /* 0x11FE58 */
            Obj_AnimFrameStep(m, obj, a1);                     /* 0x11FE62 */
            uint16_t off = rd16(m, obj + 0x3Au);
            d0 = setw(d0, off);
            int16_t dur = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)off); /* 0x11FE68 */
            if (dur <= 0) {
                if (dur < 0) {                                 /* 0x11FE70 */
                    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0xC0u)); /* 0x11FE86..0x11FE8C */
                } else if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u) { /* 0x11FE74 */
                    wr16(m, obj + 0x3Au, (uint16_t)(off + 2u)); /* 0x11FE7E */
                }
                wr32(m, obj + 0x10u, rd32(m, a1 - 4u + (uint32_t)(int32_t)(int16_t)off)); /* 0x11FE92 */
            }
            wr8(m, obj + 0x34u, 0xFFu);                        /* 0x11FE98 */
            a1 = rd32(m, obj + 0x10u);                         /* 0x11FE9C */
        } else {
            wr8(m, obj + 0x34u, 0u);                           /* 0x11FEA4 */
            a1 = rd32(m, a1 + 2u);                             /* 0x11FEA8 */
            wr16(m, obj + 0x3Au, 0u);                          /* 0x11FEAC */
            wr16(m, obj + 0x3Eu, 0u);                          /* 0x11FEB0 */
        }
    }

    /* 0x11FEB4 -> 0x12048C */
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    int16_t last_b = (int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x32u, (uint16_t)last_b);
    r->d[0] = setw(d0, (uint16_t)last_b);
    r->a[1] = a1;
}

/* ROM 0x11B06E with A1 register tracking. */
static void sub_11B06E_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) { /* 0x11B06E..0x11B07A */
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);                /* 0x11B07E */
        wr32(m, a1, obj);                                      /* 0x11B082 */
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);                     /* 0x11B084 */
        a1 = rd32(m, F3_A5 + 0x0906u);                         /* 0x11B088 */
        wr32(m, a1, obj);                                      /* 0x11B08C */
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);                          /* 0x11B08E */
        r->a[1] = a1;
    }
    Obj_11C32C(m, obj);                                        /* 0x11B092 */
}

int Fn_11FEDE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                    /* 0x11FEDE */
    r->a[1] = parent;

    if (rd8(m, parent + 0x59u) != 0u) {                        /* 0x11FEE2..0x11FEE6 */
        Obj_Free(m, obj);                                      /* 0x11FF3E */
        return 0;
    }

    r->d[0] = 0x000186A0u;                                     /* 0x11FEEA */
    r->d[1] = 2u;                                              /* 0x11FEF0 */
    sub_11AE62_regs(m, r, obj, 0x000186A0u, 2u);               /* 0x11FEF2 */
    sub_11C096_regs(m, r, obj);                                /* 0x11FEF8 */
    sub_11FE2A_regs(m, r, obj);                                /* 0x11FEFE */

    uint8_t f34 = (uint8_t)(~rd8(m, obj + 0x34u));             /* 0x11FF02 */
    wr8(m, obj + 0x34u, f34);
    if (f34 == 0u) {                                           /* 0x11FF06..0x11FF0A */
        wr8(m, obj + 0x34u, (uint8_t)(f34 | 0x10u));           /* 0x11FF0E */
        sub_11B06E_regs(m, r, obj);                            /* 0x11FF14 */
    } else {
        wr8(m, obj + 0x1Du, 0u);                               /* 0x11FF1E */
    }

    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, F3_A5 + 0x30D2u)); /* 0x11FF22..0x11FF26 */
    wr16(m, obj + 0x14u, x);                                   /* 0x11FF2A */
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, F3_A5 + 0x30D6u)); /* 0x11FF2E..0x11FF32 */
    wr16(m, obj + 0x18u, y);                                   /* 0x11FF36 */
    r->d[0] = setw(r->d[0], y);
    wr32(m, obj + 0x00u, 0x0011FEDEu);                         /* 0x11FF3A -> 0x11FED6 */
    return 0;
}
