/* ROM 0x18339C: polar-offset boss/enemy sub-part spawn initializer (A0 = object).
 * Computes parent (+0x08) relative offset from packed (+0x0C..+0x0F) base (dx.b,
 * dy.b, dist.ub, angle.ub) via 0x17BDA2, frees self if outside [-16, 335] x
 * [-16, 247], spawns child 0x17C856 in group 11, sets +0x06 = 1, and arms
 * 0x1833EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BDA2: compute (+0x50,+0x54,+0x14,+0x18) from parent (+0x08) and packed +0x0C. */
static void sub_17BDA2(const f3mem_t *m, uint32_t obj,
                       uint32_t *out_d0, uint32_t *out_d1, uint32_t *out_d2,
                       uint32_t *out_d3, uint32_t *out_a1, uint32_t *out_a4)
{
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu); /* 0x17BDA2 */
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du); /* 0x17BDA8 */
    uint32_t d2 = (uint32_t)rd8(m, obj + 0x0Eu);            /* 0x17BDAE */
    uint32_t d3 = (uint32_t)rd8(m, obj + 0x0Fu);            /* 0x17BDB4 */

    wr16(m, obj + 0x16u, (uint16_t)d0);                     /* 0x17BDBA */
    wr16(m, obj + 0x1Au, (uint16_t)d1);                     /* 0x17BDBE */
    wr16(m, obj + 0x20u, 0u);                               /* 0x17BDC2 */
    wr8(m, obj + 0x20u, (uint8_t)d3);                       /* 0x17BDC6 */

    uint32_t a1 = 0x00027E40u;                              /* 0x17BDCA */
    d0 = setw(d0, rd16(m, a1 + 0x80u + (uint32_t)((int32_t)(int16_t)d3 * 2))); /* 0x17BDD0 */
    d1 = setw(d1, rd16(m, a1 + (uint32_t)((int32_t)(int16_t)d3 * 2))); /* 0x17BDD6 */
    d2 = setw(d2, (uint16_t)((uint16_t)d2 << 2));           /* 0x17BDDC */
    d0 = (uint32_t)((int32_t)(int16_t)d2 * (int32_t)(int16_t)d0); /* 0x17BDDE */
    d1 = (uint32_t)((int32_t)(int16_t)d2 * (int32_t)(int16_t)d1); /* 0x17BDE0 */
    d0 = (d0 >> 16) | (d0 << 16);                           /* 0x17BDE2 */
    d1 = (d1 >> 16) | (d1 << 16);                           /* 0x17BDE4 */
    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)d0)); /* 0x17BDE6 */
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)d1)); /* 0x17BDEA */

    uint32_t a4 = rd32(m, obj + 0x08u);                     /* 0x17BDEE */
    d0 = setw(d0, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u))); /* 0x17BDF2..0x17BDFA */
    d1 = setw(d1, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au))); /* 0x17BDF6..0x17BDFE */
    wr16(m, obj + 0x50u, (uint16_t)d0);                     /* 0x17BE02 */
    wr16(m, obj + 0x54u, (uint16_t)d1);                     /* 0x17BE06 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                     /* 0x17BE0A */
    wr16(m, obj + 0x18u, (uint16_t)d1);                     /* 0x17BE0E */

    *out_d0 = d0;
    *out_d1 = d1;
    *out_d2 = d2;
    *out_d3 = d3;
    *out_a1 = a1;
    *out_a4 = a4;
}

int Fn_18339C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0, d1, d2, d3, a1, a4;

    sub_17BDA2(m, obj, &d0, &d1, &d2, &d3, &a1, &a4);      /* 0x18339C */

    uint16_t px = rd16(m, obj + 0x50u);                     /* 0x1833A0 */
    d0 = setw(d0, px);
    if ((int16_t)px < (int16_t)rd16(m, 0x001849F8u) ||
        (int16_t)px > (int16_t)rd16(m, 0x001849FAu)) {     /* 0x1833A4..0x1833AC */
        Obj_Free(m, obj);                                   /* 0x17C78E */
        goto out;
    }

    uint16_t py = rd16(m, obj + 0x54u);                     /* 0x1833B0 */
    d0 = setw(d0, py);
    if ((int16_t)py < (int16_t)rd16(m, 0x001849FCu) ||
        (int16_t)py > (int16_t)rd16(m, 0x001849FEu)) {     /* 0x1833B4..0x1833BC */
        Obj_Free(m, obj);                                   /* 0x17C78E */
        goto out;
    }

    d0 = Obj_Spawn(m, obj, 0x0017C856u, 0x0Bu, 1u, 0u);     /* 0x1833C0..0x1833D2 */
    wr16(m, obj + 0x06u, 1u);                               /* 0x1833DC (beq 1833F2 never taken) */
    wr32(m, obj, 0x001833ECu);                              /* 0x1833E4 */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
