#include "obj_118494.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static void addw(const f3mem_t *m, uint32_t a, uint16_t v)
{
    wr16(m, a, (uint16_t)(rd16(m, a) + v));
}

/* ROM 0x118494: route map: one marker per cleared stage (A5+0x32DE), stepping
 * up/down by the route bits at A5+0x32E0, then the current-branch cursor */
int Obj_118494_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x14u);
    wr16(m, obj + 0x18u, 0xA0u);
    wr32(m, obj + 0x10u, 0x0005AAECu);
    Obj_DrawSprite(m, obj);
    uint16_t d0 = 0u;
    int16_t d1 = (int16_t)rd16(m, F3_A5 + 0x32DEu);
    for (;;) {
        addw(m, obj + 0x14u, 0x22u);
        d1 = (int16_t)(d1 - 1);
        if (d1 <= 0)
            break;
        uint16_t d2 = rd16(m, obj + 0x18u);
        uint8_t byte = rd8(m, F3_A5 + 0x32E0u + (uint32_t)(d0 >> 3));
        if (!((byte >> (7u - (d0 & 7u))) & 1u)) {
            d2 = (uint16_t)(d2 - 6u);
            addw(m, obj + 0x18u, (uint16_t)-3);
        } else {
            d2 = (uint16_t)(d2 + 6u);
            addw(m, obj + 0x18u, 3u);
        }
        addw(m, obj + 0x18u, 0x10u);
        addw(m, obj + 0x14u, (uint16_t)-0x12);
        wr32(m, obj + 0x10u, 0x0005AAECu);
        wr16(m, obj + 0x18u, d2);
        Obj_DrawSprite(m, obj);
        d0++;
    }
    /* 0x118508 */
    uint16_t y = rd16(m, obj + 0x18u);
    uint8_t f = rd8(m, F3_A5 + 0x32E1u);
    addw(m, obj + 0x18u, (f & 0x01u) ? 3u : (uint16_t)-3);
    addw(m, obj + 0x18u, 0x10u);
    addw(m, obj + 0x14u, (uint16_t)-0x12);
    wr16(m, obj + 0x18u, y);
    if (!(f & 0x02u)) {
        uint32_t a = 0x0005AAF4u;
        uint32_t b = (rd8(m, F3_A5 + 0x879u) & 0x02u) ? 0x0005AAFCu : 0x0005AAECu;
        if (!(f & 0x01u)) {
            uint32_t t = a; a = b; b = t;
        }
        addw(m, obj + 0x18u, (uint16_t)-6);
        wr32(m, obj + 0x10u, a);
        Obj_DrawSprite(m, obj);
        addw(m, obj + 0x18u, 0x0Cu);
        wr32(m, obj + 0x10u, b);
        Obj_DrawSprite(m, obj);
    } else {
        wr32(m, obj + 0x10u, 0x0005AAECu);
        addw(m, obj + 0x18u, (f & 0x01u) ? 6u : (uint16_t)-6);
        Obj_DrawSprite(m, obj);
    }
    if (rd8(m, F3_A5 + 0x32E1u) & 0x80u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00118494u);
    return 0;
}
