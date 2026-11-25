#include "obj_184170.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

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
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x184170: enemy spawn from the parameter list at 0x184214
 * {companion fn, stats, anim, movement script} */
int Obj_184170_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = 0x00184214u;
    Obj_PolarPlace(m, obj);
    if (!in_bounds(m, obj)) {
        Obj_Free(m, obj);
        return 0;
    }
    (void)Obj_Spawn(m, obj, rd32(m, a2), 0x0Bu, 1u, 0u);
    a2 += 4u;
    wr16(m, obj + 0x06u, 0u);
    sub_184888(m, obj, rd32(m, a2));
    a2 += 4u;

    uint32_t anim = rd32(m, a2);
    a2 += 4u;
    uint16_t d0 = rd16(m, anim);
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, anim + 2u);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;

    uint32_t a1 = rd32(m, a2);
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x00u, 0x0017BE1Cu);
    return 0;
}
