#include "obj_11912C.h"

/* ROM 0x11912C: init of the stage-intro text typer. Line record: text
 * pointer 0x11932F at (11, 16) with colour 7, banked into the +0x44 slot
 * table (8 bytes per line: x.w, y.w, ptr.l whose top byte is then
 * overwritten by the +0x1D colour). Waits 3 frames in 0x119176. */
int Obj_11912C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x22u, 0x0000u);
    wr16(m, obj + 0x3Au, 0x0000u);
    wr32(m, obj + 0x10u, 0x0011932Fu);
    wr16(m, obj + 0x14u, 0x000Bu);
    wr16(m, obj + 0x18u, 0x0010u);
    wr16(m, obj + 0x1Cu, 0x0007u);

    /* 0x11914E: lea (0x44,A0,D0.w*8),A1 ; (A1)+ = x, y ; (A1) = ptr ;
     * move.b (0x1D,A0),(A1) -- same address, no post-increment */
    uint32_t a1 = obj + 0x44u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au) * 8u;
    wr16(m, a1, rd16(m, obj + 0x14u));
    wr16(m, a1 + 2u, rd16(m, obj + 0x18u));
    wr32(m, a1 + 4u, rd32(m, obj + 0x10u));
    wr8(m, a1 + 4u, rd8(m, obj + 0x1Du));

    /* 0x119166: move.w #3,(0x6,A0) ; beq never taken ; arm 0x119176 */
    wr16(m, obj + 0x06u, 0x0003u);
    wr32(m, obj + 0x00u, 0x00119176u);
    return 0;
}
