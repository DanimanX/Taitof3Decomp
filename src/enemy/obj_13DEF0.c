#include "obj_13DEF0.h"
#include "../objects/obj_common.h"

/* ROM 0x13E0EC: D0/D1/D2 words in; builds the three velocity words
 * +0x5C / +0x58 / +0x5A from the sine table at 0x27E40 (cos at +0x80). */
static void sub_13E0EC(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1, uint16_t d2)
{
    const uint32_t tbl = 0x00027E40u;
    int32_t a = (int32_t)(int16_t)d0 * (int16_t)rd16(m, tbl + 0x80u + (uint32_t)(int32_t)(int16_t)d1 * 2u);
    int32_t b = (int32_t)(int16_t)d0 * (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)d1 * 2u);
    uint16_t aw = (uint16_t)(((uint32_t)a << 2) >> 16);
    uint16_t bw = (uint16_t)(((uint32_t)b << 2) >> 16);
    wr16(m, obj + 0x5Cu, aw);
    int32_t c = (int32_t)(int16_t)bw * (int16_t)rd16(m, tbl + 0x80u + (uint32_t)(int32_t)(int16_t)d2 * 2u);
    int32_t e = (int32_t)(int16_t)bw * (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)d2 * 2u);
    wr16(m, obj + 0x58u, (uint16_t)(((uint32_t)c << 2) >> 16));
    wr16(m, obj + 0x5Au, (uint16_t)(((uint32_t)e << 2) >> 16));
}

/* ROM 0x13DEF0: init. Reads a {x.w,y.w,z.w, mask0.w,mask1.w,mask2.w, add0.w,
 * add1.b,add2.b} record at +0xC, seeds +0x4C/+0x50/+0x54 (high words of
 * longs), rolls three random angles and installs tick 0x13DF4C. */
int Obj_13DEF0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x22u, (rd8(m, F3_A5 - 0x1456u) & 1u) ? 0xFFu : 0x00u);
    Obj_ResetFields(m, obj);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x4Cu, (uint32_t)rd16(m, a1) << 16);
    wr32(m, obj + 0x50u, (uint32_t)rd16(m, a1 + 2u) << 16);
    wr32(m, obj + 0x54u, (uint32_t)rd16(m, a1 + 4u) << 16);
    a1 += 6u;
    uint16_t d2 = (uint16_t)Rng_NextMem(m);
    uint16_t d1 = (uint16_t)Rng_NextMem(m);
    uint16_t d0 = (uint16_t)Rng_NextMem(m);
    d0 &= rd16(m, a1);
    d1 &= rd16(m, a1 + 2u);
    d2 &= rd16(m, a1 + 4u);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 6u));
    d1 = (uint16_t)((d1 & 0xFF00u) | (uint8_t)((d1 & 0xFFu) + rd8(m, a1 + 8u)));
    d2 = (uint16_t)((d2 & 0xFF00u) | (uint8_t)((d2 & 0xFFu) + rd8(m, a1 + 9u)));
    sub_13E0EC(m, obj, d0, d1, d2);
    wr32(m, obj + 0x00u, 0x0013DF4Cu);                           /* ROM 0x13DF44 */
    return 0;
}
