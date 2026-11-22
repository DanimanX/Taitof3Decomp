#include "obj_11B09A.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0011B09A: integrate the velocity offsets into the position, draw the
 * sprite while its bounding box is on screen, then flip the offsets back off
 * the position (see work/disasm/bundle_11B09A.s). */
int Obj_11B09A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0;

    /* 0x11B09A: move.w (0x50,A0),D0w; add.w (0x54,A0),D0w; add.w (0x30d2,A5),D0w */
    d0 = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, d0);
    d0 = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, d0);

    /* 0x11B0BA: cmpi.w #0xe8,(0x18,A0); bge out. 0x11B0C4: cmpi.w #0x140,(0x14,A0); bge out */
    if (!((int16_t)rd16(m, obj + 0x18u) >= 0xE8 ||
          (int16_t)rd16(m, obj + 0x14u) >= 0x140)) {
        /* 0x11B0CE: movea.l (0x10,A0),A1 ; (A1)+, (A1)+ top-left then size */
        uint32_t a1 = rd32(m, obj + 0x10u);
        d0 = (uint16_t)(rd16(m, a1) << 4);
        d0 = (uint16_t)(d0 + rd16(m, obj + 0x18u));
        if ((int16_t)d0 > 0) {
            d0 = (uint16_t)(rd16(m, a1 + 2u) << 4);
            d0 = (uint16_t)(d0 + rd16(m, obj + 0x14u));
            if ((int16_t)d0 > 0)
                Obj_DrawSprite(m, obj); /* ROM 0x001282 */
        }
    }

    /* 0x11B0F0: sub.w (0x54,A0),(0x14,A0); sub.w (0x56,A0),(0x18,A0) */
    d0 = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    d0 = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    return 0;
}
