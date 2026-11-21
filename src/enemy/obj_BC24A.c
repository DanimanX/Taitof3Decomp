#include "obj_BC24A.h"
#include "../objects/obj_common.h"

/* ROM 0x0016A19C: copy D0+1 longs from A0 to A1 */
static void sub_0016A19C(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0w)
{
    for (;;) {
        wr32(m, a1, rd32(m, a0));
        a0 += 4u;
        a1 += 4u;
        if (d0w == 0)
            break;
        d0w--;
    }
}

/* ROM 0x0BC24A: fill two cached note-glyph tables from ROM 0x26798 (10 longs)
 * and 0x26994 (4 longs), then continue at 0x0BC2A0 with a 0x7F frame timer.
 * (The bundle's extra bra.w jumps to 0x0BC2A8 / 0x0BC2D2, inside the previous
 * state, are not reproduced; the capture shows the plain continuation.) */
int Obj_BC24A_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0016A19C(m, 0x0026798u, 0x0040B3AEu, 0x0009u);
    sub_0016A19C(m, 0x001C660u, 0x0040B42Eu, 0x0003u);
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000BC2A0u);
    return 0;
}
