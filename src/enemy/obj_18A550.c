#include "obj_18A550.h"
#include "../objects/obj_pool.h"

/* ROM 0x18A5A4: line-RAM fade mask. With a = 4 and b = min(+0x64.w & 0xF,
 * 8), fill the 0xE8 words at 0x626200 (+ the A5-0x5D56 bank offset) with
 * ~((((a << 8) + 8 - a) << 4) | ((b << 8) + 8 - b)). */
void Obj_18A5A4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a0 = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint16_t d0 = 4u;
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x64u) & 0x000Fu);
    if (d1 > 8u)
        d1 = 8u;
    d0 = (uint16_t)(((uint16_t)(d0 << 8) + (uint16_t)(8u - d0)) << 4);
    d1 = (uint16_t)((uint16_t)(d1 << 8) + (uint16_t)(8u - d1));
    uint16_t v = (uint16_t)~(d0 | d1);
    for (int32_t i = 0xE7; i >= 0; i--)                 /* dbra from 0xE7 */
        wr16(m, a0 + (uint32_t)i * 2u, v);
}

/* ROM 0x18A550: fade in. Mask, +0x64 += 0x3333; re-arm (0x18A548) while its
 * high word is <= 8, else release the object. */
int Obj_18A550_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_18A5A4(m, obj);
    wr32(m, obj + 0x64u, rd32(m, obj + 0x64u) + 0x00003333u);
    if ((int16_t)rd16(m, obj + 0x64u) <= 0x0008) {
        wr32(m, obj + 0x00u, 0x0018A550u);              /* 0x18A548 */
        return 0;
    }
    Obj_Free(m, obj);                                   /* 0x18A566 */
    return 0;
}
