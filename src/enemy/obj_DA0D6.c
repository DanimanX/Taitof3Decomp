#include "obj_DA0D6.h"
#include "../objects/obj_shared.h"

/* ROM 0x0CEDE4 */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;

    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));   /* move.l (A1),(0x30,A0): A1 not advanced */
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0CF1D8: start object script */

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0DA0D6 */
int Obj_DA0D6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* subi.l #0x0,(0x886,A5): value unchanged */
    sub_0CEDE4(m, obj, 0x000D94B2u);       /* lea (-0xc2e,PC) */
    uint16_t ang = rd16(m, obj + 0x58u);
    if (ang >= 0x005Au && ang < 0x010Eu) {
        Obj_StartScript(m, obj, 0x000D94D2u);
        sub_0D0678(m, obj, 0x000D9562u);
    } else {
        Obj_StartScript(m, obj, 0x000D951Au);
        sub_0D0678(m, obj, 0x000D95C2u);
    }
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    wr32(m, obj + 0x00u, 0x000DA138u);
    return 0;
}
