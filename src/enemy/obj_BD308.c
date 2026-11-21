#include "obj_BD308.h"

/* ROM 0x0BD308 */
int Obj_BD308_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x000BD308u);
        return 0;
    }
    /* 0x0BD30E: jsr 0x53B2 (tilemap 0x90A64 -> 0x611400) only writes
     * playfield RAM, outside Work RAM. */
    wr16(m, obj + 0x06u, 0x0050u); /* Z=0: beq 0xBD346 never taken */
    wr32(m, obj + 0x00u, 0x000BD340u);
    return 0;
}
