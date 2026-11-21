#include "obj_117C28.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x117C28 */
int Obj_117C28_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x117C28 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t a2 = rd32(m, a1 + 0x10u);

    uint16_t d0 = rd16(m, a2);
    a2 += 2u;
    d0 = (uint16_t)(d0 << 3);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x18u));
    d0 = (uint16_t)(d0 - 0x10u);
    wr16(m, obj + 0x18u, d0);

    d0 = rd16(m, a2);
    a2 += 2u;
    d0 = (uint16_t)(d0 << 3);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x14u));
    d0 = (uint16_t)(d0 - 0x10u);
    wr16(m, obj + 0x14u, d0);

    /* 0x117C50 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x20u)); /* bset.b #5 */
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x2Cu, 0x0005u);
    wr16(m, obj + 0x2Eu, 0x001Au);
    wr16(m, obj + 0x30u, 0x0005u);
    wr16(m, obj + 0x32u, 0x001Au);

    /* 0x117C84: lea (0x4C6,PC),A2 */
    a2 = (uint32_t)(0x117C86u + 0x4C6u);

    /* 0x117C8A: move.l (0xC,A0),D0; andi.w #0xFF -> low byte of the long */
    d0 = rd16(m, obj + 0x0Eu);
    d0 = (uint16_t)(d0 & 0x00FFu);
    if ((int16_t)d0 > 7) {
        Obj_Free(m, obj);  /* 0x117D78 */
        return 0;
    }
    if (d0 == 7) {
        /* 0x117CB6 */
        wr32(m, obj + 0x08u, 0x0011812Cu);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
        wr16(m, obj + 0x2Cu, (uint16_t)(rd16(m, obj + 0x2Cu) - 8u));
        wr16(m, obj + 0x30u, (uint16_t)(rd16(m, obj + 0x30u) - 8u));
        wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) - 8u));
        wr16(m, obj + 0x32u, (uint16_t)(rd16(m, obj + 0x32u) - 8u));
    } else if (d0 == 0) {
        Obj_Free(m, obj);  /* 0x117C9E: subq.w #1 -> -1 -> 0x117D78 */
        return 0;
    } else {
        /* 0x117CA4 */
        wr8(m, obj + 0x1Du, rd8(m, a2 + (uint32_t)(d0 - 1u)));
        wr32(m, obj + 0x08u, 0x00118106u);
    }

    /* 0x117CD6 */
    wr16(m, obj + 0x42u, 0x0005u);
    wr16(m, obj + 0x20u, 0u);
    wr8(m, obj + 0x22u, 0x02u);
    if ((int16_t)rd16(m, obj + 0x18u) >= 0x78)
        wr8(m, obj + 0x22u, (uint8_t)(0u - rd8(m, obj + 0x22u))); /* neg.b */

    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x00117D06u);
    return 0;
}
