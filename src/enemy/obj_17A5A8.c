#include "obj_17A5A8.h"

/* ROM 0x17A5A8: bfexts (A0){0:8},D0 / bfexts (A0){8:8},D1 (offset 0x0C = the
 * high word of the longword at +0x0C) seed the position words, then the state
 * falls through to 0x17A5BC, which parks the object in 0x17A5C4. */
int Obj_17A5A8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    int16_t d0w = (int16_t)(int8_t)(d0 >> 24);      /* {0:8}, sign-extended */
    int16_t d1w = (int16_t)(int8_t)(d0 >> 16);      /* {8:8}, sign-extended */
    wr16(m, obj + 0x16u, (uint16_t)d0w);
    wr16(m, obj + 0x1Au, (uint16_t)d1w);
    wr32(m, obj, 0x0017A5C4u);                      /* 0x17A5BC */
    return 0;
}
