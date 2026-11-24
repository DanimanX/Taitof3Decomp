#include "obj_11AFA6.h"

/* ROM 0x11AFA6: init — reset scroll shadows and clear bits 14..15 in 0xE8 words of GFX RAM */
int Obj_11AFA6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 - 0x5DA0u, (uint16_t)(int16_t)-0x0140);
    wr16(m, F3_A5 - 0x5D9Cu, (uint16_t)(int16_t)-0x00E8);
    wr16(m, F3_A5 + 0x420Cu, rd16(m, F3_A5 - 0x5DA0u));
    wr16(m, F3_A5 + 0x4210u, rd16(m, F3_A5 - 0x5D9Cu));

    if (!m->gfx)
        return -1;
    uint32_t a1 = 0x0062B200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (int i = 0; i <= 0xE7; i++) {
        uint16_t d1 = (uint16_t)(rd16(m, a1) & ~0xC000u);
        wr16(m, a1, d1);
        a1 += 2u;
    }

    wr32(m, obj + 0x00u, 0x0011AFE4u);
    return 0;
}
