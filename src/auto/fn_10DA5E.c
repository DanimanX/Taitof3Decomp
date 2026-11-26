/* ROM 0x10DA5E: parent-attached child sub-object initializer (A0 = object).
 * Seeds HP/hitboxes from 0x10DB26 via 0x0CEDE4, clears +0x34 bit 1, copies
 * +0x0C/+0x14/+0x18 from parent (+0x08), computes (+0x50, +0x54) velocity
 * from parent +0x68 angle * 0x32 via 0x0D2D44, subtracts scroll (A5+0x4224,
 * A5+0x4228) from (+0x14, +0x18), and arms 0x10DAB2. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

int Fn_10DA5E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x0010DB26u);                     /* 0x10DA5E..0x10DA62 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x10DA68 */

    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x10DA6E */
    r->a[1] = a1;
    wr32(m, obj + 0x0Cu, rd32(m, a1 + 0x08u));           /* 0x10DA72 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));           /* 0x10DA78 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));           /* 0x10DA7E */

    uint32_t d0, d1;
    sub_0D2D44(m, rd16(m, a1 + 0x68u), 0x0032u, &d0, &d1); /* 0x10DA84..0x10DA8C */
    wr32(m, obj + 0x50u, d0);                            /* 0x10DA92 */
    wr32(m, obj + 0x54u, d1);                            /* 0x10DA96 */
    r->d[1] = d1;

    d0 = rd32(m, F3_A5 + 0x4224u);                       /* 0x10DA9A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);     /* 0x10DA9E */
    d0 = rd32(m, F3_A5 + 0x4228u);                       /* 0x10DAA2 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);     /* 0x10DAA6 */
    r->d[0] = d0;

    wr32(m, obj + 0x00u, 0x0010DAB2u);                   /* 0x10DAAA */
    return 0;
}
