#include "obj_CEB3E.h"

/* ROM 0x0CBD86: clear the motion/animation block */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0CEB3E: spawn/init body (reached from the spawner, not a live tick) */
int Obj_CEB3E_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr8(m, obj + 0x1Du, 0x2Eu);

    /* 0x0CEB4A: no owner -> skip the trampoline link */
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    if (a1 != 0u) {
        wr32(m, obj + 0x44u, a1);
        wr32(m, a1 + 0x48u, obj);
    }

    wr16(m, obj + 0x24u, 0x0001u);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x2Cu, 0xFFFBu);
    wr16(m, obj + 0x30u, 0xFFFBu);
    wr16(m, obj + 0x2Eu, 0x0005u);
    wr16(m, obj + 0x32u, 0x0005u);
    wr8(m, obj + 0x22u, 0xFFu);
    wr16(m, obj + 0x06u, 0x0014u);
    wr32(m, obj + 0x00u, 0x000CEB94u);           /* 0x0CEB8C */
    return 0;
}
