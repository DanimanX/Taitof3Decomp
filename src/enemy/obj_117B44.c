#include "obj_117B44.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

#define DIVU_OVF 0xFFFFFFFFu
/* 68k divu.w: (remainder << 16) | quotient, or DIVU_OVF when the quotient
 * does not fit in 16 bits (V set, destination unchanged) */
static uint32_t divu_w(uint32_t n, uint16_t d)
{
    uint32_t q = n / d;
    if (q > 0xFFFFu) return DIVU_OVF;
    return ((n % d) << 16) | q;
}

/* ROM 0x117B44 */
int Obj_117B44_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x117B44 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));

    /* 0x117B54 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, F3_A5 + 0x886u, rd32(m, F3_A5 + 0x886u) + 0x466u);

    uint32_t a2 = obj + 0x46u;
    wr32(m, obj + 0x10u, a2);
    wr32(m, a2, 0x00010002u);
    a2 += 4u;

    /* 0x117B72: digits of the score in A0+0x0C, drawn as 4 tiles */
    a1 = 0x0005A834u;
    uint32_t d1 = rd32(m, obj + 0x0Cu);
    uint32_t d0 = divu_w(d1, 1000u);    /* 0x117B7E */
    if (d0 == DIVU_OVF) {
        Obj_Free(m, obj);               /* 0x117BF2 */
        return 0;
    }

    uint32_t a3;
    if ((int16_t)d0 >= 0x0A) {
        /* 0x117B8E: D2 = remainder, divu.w #100 (overflow impossible) */
        uint32_t d2 = divu_w(d0 >> 16, 100u);
        a3 = rd32(m, 0x00117C00u + (uint32_t)(int32_t)(int16_t)d2 * 4u);
    } else {
        /* 0x117BA4 (divu.w overflow leaves D0 = D1, V ignored here) */
        d0 = divu_w(d1, 100u);
        if (d0 == DIVU_OVF) d0 = d1;
        if ((int16_t)d0 >= 0x0A) {
            a3 = 0x0005A9CCu;
        } else {
            /* 0x117BBC */
            d0 = divu_w(d1, 10u);
            if (d0 == DIVU_OVF) d0 = d1;
            a3 = 0x0005A9C4u;
        }
    }

    /* 0x117BC8 */
    wr32(m, a2, rd32(m, a1 + (uint32_t)(int32_t)(int16_t)d0 * 4u));
    a2 += 4u;
    wr32(m, a2, rd32(m, a3 + 0x04u));
    a2 += 4u;
    wr16(m, obj + 0x44u, 0x003Cu);

    /* 0x117BD6 */
    wr32(m, obj + 0x00u, 0x00117BDEu);
    return 0;
}
