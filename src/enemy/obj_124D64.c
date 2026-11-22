#include "obj_124D64.h"

/* ROM 0x124D64: one-shot state switch (no side effects observed this tick) */
int Obj_124D64_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t w22 = rd16(m, obj + 0x22u);
    wr8(m, F3_A5 + 0x30F2u, 0u);
    wr8(m, obj + 0x22u, 0u);
    wr8(m, obj + 0x23u, (uint8_t)(w22 & 0xFFu)); /* sf.b leaves +0x23 intact */
    wr16(m, obj + 0x06u, 0x0000u);
    wr32(m, obj + 0x00u, 0x00124D78u);
    return 0;
}
