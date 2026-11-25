#include "obj_11AFE4.h"
#include "../video/sprite.h"

/* ROM 0x11AFE4: draw compressed tilemap (when gfx RAM is bound), arm 0x11B022 */
int Obj_11AFE4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint32_t src = rd32(m, a1); a1 += 4u;
    if (m->gfx)
        Gfx_DrawCompressedTilemap(m, src, 0x00611000u, 0x02000000u);
    wr16(m, obj + 0x2Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x30u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    wr32(m, obj + 0x24u, rd32(m, F3_A5 + 0x30DAu));
    wr8(m, obj + 0x35u, 0u);
    wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x00u, 0x0011B022u);
    return 0;
}
