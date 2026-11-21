#include "obj_CB054.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24: frame animator over a {dur.w, sprdef.l} table A1; the frame
 * index obj+0x3A is a byte offset, obj+0x3E the timer. Returns D0w = final
 * index (its sign survives the loop) */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0w = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int16_t)d0w) <= 0)   /* tst.w */
            return d0w;
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
        if ((int16_t)rd16(m, obj + 0x3Eu) <= (int16_t)rd16(m, a1 + (uint32_t)(int16_t)d0w)) {
            /* 0x0CBD4A */
            wr32(m, obj + 0x10u, rd32(m, a1 + 0x02u + (uint32_t)(int16_t)d0w));
            return d0w;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0w + 6u));
    }
}

/* ROM 0xCB054 */
int Obj_CB054_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CB054: frames 0..5 only run the renderer below */
    while ((int16_t)rd16(m, obj + 0x06u) >= 6) {
        /* 0x0CB05E: lea (0xCA,PC),A1 -> 0x0CB12A */
        uint32_t a1 = 0x0CB060u + 0xCAu;
        uint16_t d0w = sub_0CBD24(m, obj, a1);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int16_t)d0w) > 0)
            break;                                   /* 0x0CB06E */
        /* 0x0CB072: re-roll the flip frame */
        wr16(m, obj + 0x3Au, 0u);
        wr8(m, obj + 0x1Cu, (uint8_t)(Rng_NextMem(m) & 0x03u));
    }

    /* 0x0CB086 */
    Obj_DrawSprite(m, obj);
    {
        uint16_t d0w = rd16(m, F3_A5 + 0x1D5Cu);
        d0w = (uint16_t)((int16_t)d0w >> 2);         /* asr.w #2 */
        wr16(m, F3_A5 + 0x1D5Cu, (uint16_t)(rd16(m, F3_A5 + 0x1D5Cu) + d0w));
    }
    if ((rd8(m, obj + 0x07u) & 0x01u) == 0u) {
        /* 0x0CB0A0: Coll_AddAreaDamage(0xC000, A1 = obj+8) */
        uint32_t a0d = 0x0041A980u + (uint32_t)(int16_t)rd16(m, rd32(m, obj + 0x08u) + 0x22u) * 4u;
        wr32(m, a0d, rd32(m, a0d) + 0xC000u);
    }

    /* 0x0CB0B0: append to the A5+0x902 hazard list */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x902u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x902u, a1 + 4u);
    }

    /* 0x0CB0BA */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));
    if ((int16_t)rd16(m, obj + 0x06u) < 0x3C) {
        /* 0x0CB04C */
        wr32(m, obj, 0x000CB054u);
        return 0;
    }

    /* 0x0CB0C6 */
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) | 0x80u));  /* bset.b #7 */
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) & (uint8_t)~0x04u)); /* bclr.b #2 */
    wr32(m, obj, 0x000CB0DAu);
    return 0;
}
