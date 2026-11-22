#include "obj_1070DA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0CEDE4: object template init (params at A1 + 0x14; (A1) is not consumed
 * by the 0x30 store, but A1 is advanced by it, as the 0x0CEC4C variant does) */
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
    wr32(m, obj + 0x30u, rd32(m, a1));
    a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x1070DA */
int Obj_1070DA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1070DA: coordinates are copied from the parent (obj+0x08) */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a4 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a4 + 0x18u));

    /* 0x1070EA: lea (0x312,PC),A1 = 0x1073FE */
    sub_0CEDE4(m, obj, 0x001073FEu);

    /* 0x1070F4: lea (0x328,PC),A1 = 0x10741E */
    sub_0D0678(m, obj, 0x0010741Eu);

    /* 0x1070FE */
    wr8(m, obj + 0x06u, 0x01u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr32(m, obj + 0x00u, 0x00107112u);
    return 0;
}
