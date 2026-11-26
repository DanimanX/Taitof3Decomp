#include "obj_112BEE.h"
#include "../objects/obj_pool.h"

/* ROM 0x113DA8: stage-2 entry of 0x112BEE. Places the body, spawns its
 * parts (0x118CF6 kept as the +0x08 parent link, 0x112D1A, two 0x112C1A
 * with different data, seven 0x1140BE with arg 0..6, 0x1134B2), then
 * arms 0x113ED0 with a random +0x52 in 0xA0..0x11F. */
int Obj_113DA8(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x50u, 0x00u);
    wr8(m, obj + 0x51u, 0x00u);
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0xFFD8u);
    wr16(m, obj + 0x42u, 0x0004u);
    wr32(m, obj + 0x24u, 0x00FA0000u);
    wr16(m, obj + 0x56u, 0x0384u);
    wr32(m, obj + 0x08u, Obj_Spawn(m, obj, 0x00118CF6u, 3u, 2u, 0x001143ECu));
    (void)Obj_Spawn(m, obj, 0x00112D1Au, 3u, 2u, 0x001143FAu);
    (void)Obj_Spawn(m, obj, 0x00112C1Au, 3u, 2u, 0x001144D6u);
    for (uint32_t i = 0; i < 7u; i++)                   /* 0x113E30, D1 = arg */
        (void)Obj_Spawn(m, obj, 0x001140BEu, 3u, 2u, i);
    (void)Obj_Spawn(m, obj, 0x00112C1Au, 3u, 2u, 0x001144B2u);
    (void)Obj_Spawn(m, obj, 0x001134B2u, 3u, 2u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr8(m, obj + 0x34u, 0x00u);
    wr8(m, obj + 0x35u, 0x00u);
    wr8(m, obj + 0x1Du, 0x00u);
    wr8(m, obj + 0x4Fu, 0x00u);
    wr16(m, obj + 0x20u, 0x0030u);
    wr16(m, obj + 0x06u, 0x00B4u);
    wr16(m, obj + 0x52u, (uint16_t)((Rng_NextMem(m) & 0x7Fu) + 0xA0u));
    wr16(m, obj + 0x54u, 0x0046u);
    wr32(m, obj + 0x00u, 0x00113ED0u);
    return 0;
}

/* ROM 0x112BEE: movea.l (-4,A1,D0.w*4),A1 ; jmp (A1) with D0 = A5+0x32DE */
int Obj_112BEE_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t st = (int16_t)rd16(m, F3_A5 + 0x32DEu);
    uint32_t fn = rd32(m, 0x00112BFEu + (uint32_t)((int32_t)st * 4 - 4));
    switch (fn) {
    case 0x00113DA8u: return Obj_113DA8(m, obj);
    default:          return -1;    /* 0x11376A, 0x1144E0, ... not yet */
    }
}
