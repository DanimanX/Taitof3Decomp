#include "obj_13E4EE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x13E4EE */
int Obj_13E4EE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x4Cu, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x50u, rd16(m, a1 + 0x18u));
    wr16(m, obj + 0x1Eu, 0u);

    /* 0x13E506: speed 0..7, angle 0..255 from table 0x27E40 (sin at +0x80) */
    int16_t d1w = (int16_t)(Rng_NextMem(m) & 7u);
    uint32_t ang = Rng_NextMem(m) & 0xFFu;
    int16_t t = (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
    wr32(m, obj + 0x44u, (uint32_t)((int32_t)t * d1w));          /* muls.w */
    t = (int16_t)rd16(m, 0x00027E40u + ang * 2u);
    wr32(m, obj + 0x48u, (uint32_t)((int32_t)t * d1w));

    /* 0x13E53A: A0+0x0C = {dx.b, dy.b, kind.w} */
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    int16_t kind = (int16_t)(uint16_t)d0;                         /* bfextu {16:16} */

    if (kind >= 2 && kind <= 4) {
        /* 0x13E562: kinds 2..4 are re-rolled (0 keeps the kind) */
        int16_t r = (int16_t)(Rng_NextMem(m) & 0x0Fu);
        if (r != 0) {
            kind = 2;
            if (r > 3) {
                kind = 3;
                if (r > 9)
                    kind = 4;
            }
        }
    }

    /* 0x13E586: lea (0x9A,PC),A2 -> 0x13E622 */
    wr32(m, obj + 0x0Cu, rd32(m, 0x0013E622u + (uint32_t)((int32_t)kind * 4)));

    /* 0x13E594 */
    wr8(m, obj + 0x1Cu, (uint8_t)(Rng_NextMem(m) & 0x03u));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x0013E5B2u);
    return 0;
}
