#include "obj_118B3C.h"
#include "../objects/obj_common.h"

/* ROM 0x118B3C: per-shot spawner hook. Clears the per-frame fields
 * (0x0CBD86), copies the parent's 32-bit x/y, adds the two 8-bit signed
 * deltas held in the header arg word at +0x0C and parks the object on
 * 0x118B74 (bit-exact in the MAME captures). */
int Obj_118B3C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x118B3C */
    Obj_ResetFields(m, obj);

    /* 0x118B42: movea.l (0x8,A0),A1 ; move.l (0x14,A1),(0x14,A0) */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    /* 0x118B52: bfexts D0{0:8} -> D1 ; add.w D1w,(0x14,A0) */
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)(int8_t)(d0 & 0xFFu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1));
    /* 0x118B5E: bfexts D0{8:8} -> D1 ; add.w D1w,(0x18,A0) */
    d1 = (int16_t)(int8_t)((d0 >> 8) & 0xFFu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));

    /* 0x118B66 */
    wr8(m, obj + 0x1Du, 0x28u);
    /* 0x118B6C: next tick */
    wr32(m, obj + 0x00u, 0x00118B74u);
    return 0;
}
