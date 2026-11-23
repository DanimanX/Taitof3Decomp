#include "obj_115F36.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x113652: init from the spawn argument (+0xC: dx.b, dy.b, angle.b, flip.b)
 * at the parent's position (same body as in src/enemy/obj_114A10.c) */
static void sub_113652(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 24));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 16));
    wr16(m, obj + 0x20u, (uint16_t)(((d0 >> 8) & 0xFFu) << 8));
    wr16(m, obj + 0x22u, (uint16_t)(d0 & 0xFFu));
    if ((d0 & 0xFFu) != 0u)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
}

/* ROM 0x115F36: first tick of the spinning bullet -- configure the sprite
 * frame/zoom, initialise from the spawn argument, park on 0x115F96. */
int Obj_115F36_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x115F36: move.w #0x3c,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 */
    Sound_PlayHooked(m, 0x003Cu);

    /* 0x115F4A */
    wr16(m, obj + 0x2Cu, 0x0005u);
    wr16(m, obj + 0x2Eu, 0x000Au);
    wr16(m, obj + 0x30u, 0x0005u);
    wr16(m, obj + 0x32u, 0x000Au);
    wr32(m, obj + 0x10u, 0x000294E0u);
    sub_113652(m, obj);                          /* 0x115F6A */

    /* 0x115F70 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x42u, 0x0018u);

    /* 0x115F8E */
    wr32(m, obj + 0x00u, 0x00115F96u);
    return 0;
}
