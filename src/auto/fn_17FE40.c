/* ROM 0x17FE40: boss/enemy sub-part spawn initializer (A0 = object).
 * Copies global target A5+0x1D28 into +0x4C (freeing self if 0), offsets position
 * from parent (+0x08) via packed signed bytes at +0x0C/+0x0D (0x17BD68), frees
 * self if outside [-16, 335] x [-16, 247], spawns child 0x17C856 in group 11,
 * sets +0x06 = 1, and arms 0x17FE9A. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_17FE40(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t target = rd32(m, F3_A5 + 0x1D28u);             /* 0x17FE40 */
    wr32(m, obj + 0x4Cu, target);
    if (target == 0u) {                                     /* 0x17FE46 */
        Obj_Free(m, obj);                                   /* 0x17C78E */
        return 0;
    }

    /* 0x17FE4A -> 0x17BD68: parent (+0x08) pos + signed byte offsets +0x0C/+0x0D */
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu);
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du);
    wr16(m, obj + 0x16u, (uint16_t)d0);
    wr16(m, obj + 0x1Au, (uint16_t)d1);
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t px = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t py = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    d0 = (d0 & 0xFFFF0000u) | px;
    d1 = (d1 & 0xFFFF0000u) | py;
    wr16(m, obj + 0x50u, px);
    wr16(m, obj + 0x54u, py);
    wr16(m, obj + 0x14u, px);
    wr16(m, obj + 0x18u, py);

    /* 0x17FE4E..0x17FE6A: bounds check [-16, 335] x [-16, 247] */
    if ((int16_t)px < (int16_t)rd16(m, 0x001849F8u) ||
        (int16_t)px > (int16_t)rd16(m, 0x001849FAu)) {
        Obj_Free(m, obj);                                   /* 0x17C78E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[4] = a4;
        return 0;
    }
    d0 = (d0 & 0xFFFF0000u) | py;
    if ((int16_t)py < (int16_t)rd16(m, 0x001849FCu) ||
        (int16_t)py > (int16_t)rd16(m, 0x001849FEu)) {
        Obj_Free(m, obj);                                   /* 0x17C78E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[4] = a4;
        return 0;
    }

    d0 = Obj_Spawn(m, obj, 0x0017C856u, 0x0Bu, 1u, 0u);     /* 0x17FE6E..0x17FE80 */
    wr16(m, obj + 0x06u, 1u);                               /* 0x17FE8A (Z=0, beq 17FEA0 never taken) */
    wr32(m, obj, 0x0017FE9Au);                              /* 0x17FE92 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[4] = a4;
    return 0;
}
