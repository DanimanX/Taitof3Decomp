#include "obj_112C1A.h"

/* ROM 0x1133AE: clear state, read 6 signed offset bytes from the script at +0xC */
void Obj_1133AE_LoadPart(const f3mem_t *m, uint32_t obj)
{
    static const uint8_t dst[6] = { 0x52u, 0x54u, 0x2Cu, 0x30u, 0x2Eu, 0x32u };
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    for (int i = 0; i < 6; i++) {
        wr16(m, obj + dst[i], (uint16_t)(int16_t)(int8_t)rd8(m, a1));
        a1++;
    }
    wr32(m, obj + 0x0Cu, a1);
}

/* ROM 0x112C1A */
int Obj_112C1A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_1133AE_LoadPart(m, obj);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x34u, rd8(m, a1 + 0x34u));
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x4Fu));
    wr32(m, obj + 0x28u, rd32(m, a1 + 0x28u));
    wr32(m, obj + 0x00u, 0x00112C3Cu);   /* 0x112C34 */
    return 0;
}
