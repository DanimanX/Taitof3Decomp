#include "obj_17C700.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17C700: `bra.w 0x17C662` -- the dithering fade of the 0x17C0xx
 * family. +0x1F and +0x1E grow by 0x10 a tick until either reaches 0xF0,
 * which frees the object (0x17C78E). The sprite is pushed right/down by half
 * of (fade * size*16) >> 8, where the X step uses the frame record's SECOND
 * size word and the Y step the first. */

/* bfextu byte ; mulu.w D1,D0 ; lsr.w #8,D0 ; lsr.w #1,D0: the shifts act on
 * the LOW word of the 32-bit product only. */
static uint16_t fade_step(uint8_t fade, uint16_t size)
{
    uint16_t d1 = (uint16_t)(size << 4);
    uint16_t lo = (uint16_t)((uint32_t)fade * d1);
    return (uint16_t)((lo >> 8) >> 1);
}

int Obj_17C700_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C662..0x17C67E: bfextu {0:8} of +0x1F / +0x1E ; cmpi.w #0xF0 ; bge */
    if (rd8(m, obj + 0x1Fu) >= 0xF0u || rd8(m, obj + 0x1Eu) >= 0xF0u) {
        Obj_Free(m, obj);
        return 0;
    }

    if (Obj_Script10(m, obj) < 0)                   /* 0x17C67E */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17C690: X */
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) +
         fade_step(rd8(m, obj + 0x1Fu), rd16(m, rd32(m, obj + 0x10u) + 2u))));
    /* 0x17C6B0: Y */
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) +
         fade_step(rd8(m, obj + 0x1Eu), rd16(m, rd32(m, obj + 0x10u)))));

    /* 0x17C6CE: frame tick (0x179A5C)[+0x40].tick */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17C6E0: centre on the frame's size */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    Obj_DrawSprite(m, obj);                         /* 0x17C6F2 */
    wr32(m, obj + 0x00u, 0x0017C700u);
    return 0;
}
