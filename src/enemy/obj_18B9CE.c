#include "obj_18B9CE.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x18BB3A: add (+0x64 * first word of the sprite def) >> 5 to +0x1A */
static void sub_18BB3A(const f3mem_t *m, uint32_t obj)
{
    uint16_t d1 = rd16(m, rd32(m, obj + 0x10u));
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x64u) * d1;
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)((uint16_t)d0 >> 5)));
}

/* ROM 0x17A870: load the vtable record at (+0xC)->+4 into the object and run
 * the INIT half of the frame list it points at. *a2_out = A2 left past the
 * record (the callers read on from there). */
static int sub_17A870(const f3mem_t *m, uint32_t obj, uint32_t *a2_out)
{
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    uint32_t a2 = rd32(m, a3 + 4u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);
    wr16(m, obj + 0x24u, rd16(m, a2));
    wr16(m, obj + 0x28u, rd16(m, a2 + 2u));
    wr32(m, obj + 0x2Cu, rd32(m, a2 + 4u));
    wr32(m, obj + 0x30u, rd32(m, a2 + 8u));
    wr16(m, obj + 0x34u, rd16(m, a2 + 0x0Cu));
    wr16(m, obj + 0x16u, rd16(m, a2 + 0x0Eu));
    wr16(m, obj + 0x1Au, rd16(m, a2 + 0x10u));
    uint32_t a1 = rd32(m, a2 + 0x12u);
    uint16_t op = rd16(m, a1);
    wr16(m, obj + 0x40u, op);
    wr32(m, obj + 0x3Au, a1 + 2u);
    *a2_out = a2 + 0x16u;
    if (Obj_AnimInit(m, obj, op) < 0)
        return -1;
    Obj_18495E(m, obj);
    return 0;
}

/* ROM 0x18BFCC: vtable init -- 0x17A870 plus the spin/lean words */
static int init_18BFCC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2;
    if (sub_17A870(m, obj, &a2) < 0)
        return -1;
    wr16(m, obj + 0x5Eu, rd16(m, a2));
    wr16(m, obj + 0x64u, rd16(m, a2 + 2u));
    wr16(m, obj + 0x66u, rd16(m, a2 + 4u));
    wr8(m, obj + 0x1Eu, rd8(m, obj + 0x64u));
    sub_18BB3A(m, obj);
    wr32(m, obj + 0x68u, rd32(m, obj + 0x24u) >> 1);
    return 0;
}

/* ROM 0x18B9CE: run the vtable's init, then switch to 0x18B9DE */
int Obj_18B9CE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t fn = rd32(m, rd32(m, obj + 0x0Cu));
    if (fn == 0x18BFCCu) {
        if (init_18BFCC(m, obj) < 0)
            return -1;
    } else if (fn == 0x17A870u) {
        uint32_t a2;
        if (sub_17A870(m, obj, &a2) < 0)
            return -1;
    } else {
        return -1;
    }
    wr32(m, obj, 0x18B9DEu);
    return 0;
}
