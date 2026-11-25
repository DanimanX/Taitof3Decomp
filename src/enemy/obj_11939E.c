#include "obj_11939E.h"
#include "../video/sprite.h"

/* ROM 0x11939E: draw HUD/label string from 0x11957B via 0x119616 */
int Obj_11939E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a0 = 0x0041B4EEu;
    uint32_t a2 = F3_A5 + 0x30CAu;
    uint32_t a3 = 0x00059304u;
    wr32(m, a0 + 0x10u, a2);
    wr16(m, a0 + 0x1Cu, 0u);
    wr16(m, a0 + 0x1Eu, 0u);
    wr16(m, a0 + 0x14u, 0x0008u);
    wr16(m, a0 + 0x18u, 0x0030u);
    wr32(m, a2, 0x00010001u);
    a2 += 4u;
    uint32_t p = 0x0011957Bu;
    for (;;) {
        uint8_t ch = rd8(m, p++);
        if (ch == 0u)
            break;
        uint32_t idx = (uint8_t)(ch - 0x20u);
        wr32(m, a2, rd32(m, a3 + idx * 4u));
        Obj_DrawSprite(m, a0);
        wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) + 0x0010u));
    }
    wr32(m, obj + 0x00u, 0x0011939Eu);
    return 0;
}
