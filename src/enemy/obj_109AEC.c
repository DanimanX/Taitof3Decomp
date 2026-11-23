#include "obj_109AEC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"

/* ROM 0x109AEC */
int Obj_109AEC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x109AEC */
    if ((rd8(m, 0xF3EB5u) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    /* 0x109AF8 */
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    /* 0x109B02 */
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    /* 0x109B0A */
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t != 0u)
        return 0;

    /* 0x109B12 */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x1Eu);

    /* 0x109B1C: pea 0 / move.w #2,-(SP) / move.w #5,-(SP) / move.l #0x109B7E,-(SP) */
    uint32_t spawned = Obj_Spawn(m, obj, 0x00109B7Eu, 5u, 2u, 0u);
    /* 0x109B38 */
    if ((int32_t)spawned < 0) {
        /* 0x109B78 */
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    /* 0x109B3E */
    uint32_t a1 = spawned;
    wr32(m, a1 + 0x14u, 0x01400000u);
    wr32(m, a1 + 0x18u, 0u);

    /* 0x109B4C: jsr 0x0D0E3A */
    uint16_t d0 = Obj_Next4Way(m);
    uint16_t d1 = (uint16_t)(d0 & 0x007Fu);      /* andi.w #0x7F */
    d0 = (uint16_t)(d0 & 0x003Fu);               /* andi.w #0x3F */
    d0 = (uint16_t)(d0 + d1 + 5u);               /* add.w D1w,D0w ; addq.w #5,D0w */
    wr16(m, a1 + 0x18u, d0);
    wr8(m, a1 + 0x4Eu, 0u);

    /* 0x109B68 */
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));

    return 0;
}
