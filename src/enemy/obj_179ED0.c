#include "obj_179ED0.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x179ED0: particle/sparkle child init */
int Obj_179ED0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));
    uint32_t parent = rd32(m, obj + 0x08u);
    int16_t ox = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t oy = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(ox + (int16_t)rd16(m, parent + 0x50u)));
    wr16(m, obj + 0x54u, (uint16_t)(oy + (int16_t)rd16(m, parent + 0x54u)));

    uint32_t a1 = (Obj_RandRange(m, 1u) == 0u) ? 0x00179B3Au : 0x00179B8Cu;
    uint16_t anim_idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;

    uint32_t scr = 0x00179F12u;
    wr32(m, obj + 0x44u, scr);
    wr16(m, obj + 0x48u, rd16(m, scr + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x20u, (uint8_t)Obj_RandRange(m, 0x10u));
    wr32(m, obj + 0x00u, 0x00179C6Eu);
    return 0;
}
