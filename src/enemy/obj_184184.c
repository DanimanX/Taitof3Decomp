#include "obj_184184.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17BDA2: place the object at parent + polar offset from arg (+0xC) */
static void sub_17BDA2(const f3mem_t *m, uint32_t obj)
{
    int16_t d0 = (int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint16_t d2 = rd8(m, obj + 0x0Eu);
    uint16_t d3 = rd8(m, obj + 0x0Fu);
    wr16(m, obj + 0x16u, (uint16_t)d0);
    wr16(m, obj + 0x1Au, (uint16_t)d1);
    wr16(m, obj + 0x20u, 0u);
    wr8(m, obj + 0x20u, (uint8_t)d3);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + d3 * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + d3 * 2u);
    d2 = (uint16_t)(d2 << 2);
    uint32_t p0 = (uint32_t)((int32_t)c * (int32_t)(int16_t)d2);
    uint32_t p1 = (uint32_t)((int32_t)s * (int32_t)(int16_t)d2);
    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)(p0 >> 16)));
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)(p1 >> 16)));
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
}

/* cmp2.w (0x1849F8) / (0x1849FC): on-screen test of +0x50 / +0x54 */
static int in_bounds(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu))
        return 0;
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu))
        return 0;
    return 1;
}

/* ROM 0x184888: initialise the object's stats from the record at A1 */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x02u));
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x06u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Cu));
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x10u));
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x12u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);   /* divul.l #10 */
}

/* Animation state machines, table 0x179A5C (init, tick) per index (+0x40).
 * Only the indices these objects use are translated; others return -1. */

/* ROM 0x179550 (index 0 init) */
static void anim0_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
}

/* ROM 0x1798D4 (index 10 tick): pick one of 32 direction frames
 * {frame.l, colour.w} from +0x3A by the heading byte +0x20 */
static void anim10_tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)(d0 * 6u);
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
}

/* ROM 0x179614 (index 10 init) */
static void anim10_init(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    anim10_tick(m, obj);
}

static int anim_init(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  anim0_init(m, obj); return 0;
    case 10u: anim10_init(m, obj); return 0;
    default:  return -1;
    }
}

/* Load script at a1 ({index.w, data...}) and run its init (jsr via table). */
static int anim_start(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return anim_init(m, obj, d0);
}

/* ROM 0x184184: enemy spawn from the parameter list at 0x184234
 * {companion fn, stats, anim, movement script} */
int Obj_184184_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = 0x00184234u;
    sub_17BDA2(m, obj);
    if (!in_bounds(m, obj)) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_Spawn(m, obj, rd32(m, a2), 0x0Bu, 1u, 0u);
    a2 += 4u;
    wr16(m, obj + 0x06u, 0u);           /* move.w #0; beq: always taken */
    sub_184888(m, obj, rd32(m, a2)); a2 += 4u;
    if (anim_start(m, obj, rd32(m, a2)) < 0)
        return -1;
    a2 += 4u;
    uint32_t a1 = rd32(m, a2);
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x00u, 0x0017BE1Cu);
    return 0;
}
