#include "obj_11C1FC.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24 */
static uint16_t sub_CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0;

    for (;;) {
        d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x11C1FC: draw up to 8 packed entries of the list at 0x40B0F6 */
int Obj_11C1FC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x59u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t a2 = 0x0040B0F6u;
    for (int i = 0; i < 8; i++, a2 += 4u) {
        uint32_t v = rd32(m, a2);
        if (v == 0u)
            continue;
        wr16(m, obj + 0x3Au, (uint16_t)((v >> 5) & 0x7Fu));
        wr16(m, obj + 0x3Eu, (uint16_t)((v >> 2) & 0x07u));
        uint32_t a1 = 0x0011C2AEu;
        uint16_t d0 = sub_CBD24(m, obj, a1);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
            wr32(m, a2, 0u);
            continue;
        }
        v = (v & ~(0x7Fu << 5)) | (((uint32_t)rd16(m, obj + 0x3Au) & 0x7Fu) << 5);
        v = (v & ~(0x07u << 2)) | (((uint32_t)rd16(m, obj + 0x3Eu) & 0x07u) << 2);
        wr32(m, a2, v);
        int32_t fx = (int32_t)((v >> 21) & 0x3FFu);
        if (fx & 0x200) fx -= 0x400;
        int32_t fy = (int32_t)((v >> 12) & 0x1FFu);
        if (fy & 0x100) fy -= 0x200;
        wr16(m, obj + 0x14u, (uint16_t)((uint16_t)fx + rd16(m, F3_A5 + 0x30D2u)));
        wr16(m, obj + 0x18u, (uint16_t)((uint16_t)fy + rd16(m, F3_A5 + 0x30D6u)));
        uint32_t spr = rd32(m, obj + 0x10u);
        uint32_t off = rd32(m, obj + 0x0Cu);
        uint16_t lo = (uint16_t)((uint16_t)off - (uint16_t)(rd16(m, spr) << 3));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + lo));
        uint16_t hi = (uint16_t)((uint16_t)(off >> 16) - (uint16_t)(rd16(m, spr + 2u) << 3));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + hi));
        Obj_DrawSprite(m, obj);
    }
    wr32(m, obj + 0x00u, 0x0011C1FCu);
    return 0;
}
