/* ROM 0x183268: boss sub-part polar-placed initializer (A0 = object).
 * Computes parent-relative offset & position via 0x17BDA2, seeds stats &
 * hitboxes from 0x1832B2 via 0x184888, initializes 0x179A5C animation from
 * 0x1832C8, installs script 0x1832F4, sets +0x34.b = 2, and transitions to
 * 0x17BE1C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BDA2: compute (+0x50,+0x54,+0x14,+0x18) from parent (+0x08) and packed +0x0C. */
static void sub_17BDA2(const f3mem_t *m, uint32_t obj,
                       uint32_t *out_d0, uint32_t *out_d1, uint32_t *out_d2,
                       uint32_t *out_d3, uint32_t *out_a4)
{
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu); /* 0x17BDA2 */
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du); /* 0x17BDA8 */
    uint32_t d2 = (uint32_t)rd8(m, obj + 0x0Eu);                  /* 0x17BDAE */
    uint32_t d3 = (uint32_t)rd8(m, obj + 0x0Fu);                  /* 0x17BDB4 */

    wr16(m, obj + 0x16u, (uint16_t)d0);                           /* 0x17BDBA */
    wr16(m, obj + 0x1Au, (uint16_t)d1);                           /* 0x17BDBE */
    wr16(m, obj + 0x20u, 0u);                                     /* 0x17BDC2 */
    wr8(m, obj + 0x20u, (uint8_t)d3);                             /* 0x17BDC6 */

    uint32_t a1 = 0x00027E40u;                                    /* 0x17BDCA */
    d0 = setw(d0, rd16(m, a1 + 0x80u + (uint32_t)((int32_t)(int16_t)d3 * 2))); /* 0x17BDD0 */
    d1 = setw(d1, rd16(m, a1 + (uint32_t)((int32_t)(int16_t)d3 * 2))); /* 0x17BDD6 */
    d2 = setw(d2, (uint16_t)((uint16_t)d2 << 2));                 /* 0x17BDDC */
    d0 = (uint32_t)((int32_t)(int16_t)d2 * (int32_t)(int16_t)d0); /* 0x17BDDE */
    d1 = (uint32_t)((int32_t)(int16_t)d2 * (int32_t)(int16_t)d1); /* 0x17BDE0 */
    d0 = (d0 >> 16) | (d0 << 16);                                 /* 0x17BDE2 */
    d1 = (d1 >> 16) | (d1 << 16);                                 /* 0x17BDE4 */
    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)d0)); /* 0x17BDE6 */
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)d1)); /* 0x17BDEA */

    uint32_t a4 = rd32(m, obj + 0x08u);                           /* 0x17BDEE */
    d0 = setw(d0, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u))); /* 0x17BDF2..0x17BDFA */
    d1 = setw(d1, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au))); /* 0x17BDF6..0x17BDFE */
    wr16(m, obj + 0x50u, (uint16_t)d0);                           /* 0x17BE02 */
    wr16(m, obj + 0x54u, (uint16_t)d1);                           /* 0x17BE06 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                           /* 0x17BE0A */
    wr16(m, obj + 0x18u, (uint16_t)d1);                           /* 0x17BE0E */

    *out_d0 = d0;
    *out_d1 = d1;
    *out_d2 = d2;
    *out_d3 = d3;
    *out_a4 = a4;
}

int Fn_183268(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0, d1, d2, d3, a4;

    sub_17BDA2(m, obj, &d0, &d1, &d2, &d3, &a4);                  /* 0x183268 */
    Obj_184888(m, obj, 0x001832B2u);                              /* 0x18326C..0x183272 */
    d0 = rd32(m, obj + 0x24u);                                    /* 0x1848C8..0x1848D0 divul.l #0xa */

    uint32_t a1 = 0x001832C8u;                                    /* 0x183278 */
    uint16_t idx = rd16(m, a1); a1 += 2u;                         /* 0x18327E */
    d0 = setw(d0, idx);
    wr16(m, obj + 0x40u, idx);                                    /* 0x183280 */
    wr32(m, obj + 0x3Au, a1);                                     /* 0x183284 */

    if (Obj_AnimInit(m, obj, idx) < 0)                            /* 0x183288..0x183292 */
        return -1;

    a1 = 0x001832F4u;                                             /* 0x183294 */
    wr32(m, obj + 0x44u, a1);                                     /* 0x18329A */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                       /* 0x18329E */
    wr16(m, obj + 0x4Au, 0u);                                     /* 0x1832A4 */
    wr8(m, obj + 0x34u, 2u);                                      /* 0x1832A8 */
    wr32(m, obj, 0x0017BE1Cu);                                    /* 0x1832AE -> 0x17BE14 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
