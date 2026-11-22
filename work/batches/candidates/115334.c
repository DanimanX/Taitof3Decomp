#include "obj_115334.h"
#include "../enemy/obj_13E4EE.h"

/* ROM 0x1153b8: set fn pointer to OBJ_1153B8 */
static void sub_1153b8(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x001153B8u);
}

/* ROM 0x115334: parent-child init — copy sprite/collision/script data from
 * parent, build pointer chain via +0x44/+0x48, then set fn to Obj_13E4EE. */
int Obj_115334_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    /* Read parent pointer from (0xC,A0) */
    a1 = rd32(m, obj + 0x0Cu);

    /* Copy sprite top-left position word to obj+0x42 */
    wr16(m, obj + 0x42u, rd16(m, a1));
    a1 += 2u;

    /* Copy parent pointer data (long) from a1 to obj+0xC */
    wr32(m, obj + 0x0Cu, rd32(m, a1));
    a1 -= 4u;
    a1 = rd32(m, obj + 0x0Cu); /* re-read parent pointer */

    /* Reload parent pointer and copy sprite word to +0x42 */
    a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x42u, rd16(m, a1));
    a1 += 2u;

    /* Copy long from parent+(0xC+2) to obj+0xC */
    wr32(m, obj + 0x0Cu, rd32(m, a1));

    /* Set collision box: left=-8, top=-8, right=7, bottom=7 */
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)-8);
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)-8);
    wr16(m, obj + 0x2Eu, 7u);
    wr16(m, obj + 0x32u, 7u);

    /* Reload parent pointer from (0x8,A0) */
    a1 = rd32(m, obj + 0x08u);

    /* Copy field +0x22 from parent to child, then decrement parent's copy */
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) - 1u));

    /* Clear child fields +0x44 and +0x48 */
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);

    /* A2 = parent's field +0x48 (grandchild/sibling pointer) */
    uint32_t a2 = rd32(m, a1 + 0x48u);

    if ((int32_t)a2 != 0) {
        wr32(m, a2 + 0x44u, obj);
    }

    /* Link: child->+0x48 = A2, parent->+0x48 = obj */
    wr32(m, obj + 0x48u, a2);
    wr32(m, a1 + 0x48u, obj);

    /* Clear child fields */
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);

    /* Reload parent pointer from (0x8,A0) */
    a1 = rd32(m, obj + 0x08u);

    /* Copy fields from parent: +0x20, +0x14, +0x18 */
    wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    sub_1153b8(m, obj);
    return 0;
}
