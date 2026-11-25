#include "obj_126FF6.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"

/* Shared tracker helpers of the 0x11B/0x11C family (0x0CBDDE and 0x11AE62
 * are the verified copies from src/enemy/obj_126DEC.c). */

/* ROM 0x0CBDDE: the object this one's hit points are paid to */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x11AE62: fold +0x24 (scaled by D1 through the target kind at +0x22)
 * into the shared A5+0x30DA pool; when that reaches 0 the 0x0CBDDE target
 * gets D0 added at +0x48. */
static void sub_11AE62(const f3mem_t *m, uint32_t obj, uint32_t d0, unsigned d1)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)
        return;
    int32_t d2 = (int32_t)rd32(m, obj + 0x24u);
    if (d2 == 0)
        return;
    wr32(m, obj + 0x24u, 0u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u) {
        d2 >>= (d1 & 63u);
        if (k != 1u && k != 5u)
            d2 >>= 1;
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2);
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)
        return;
    a1 = sub_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x11C096: the hit spark, shared with src/enemy/obj_1270BE.c. On a
 * pending +0x35 the byte is folded into the parent, the 0x3116/0x311A/0x311C
 * debounce decides whether a record is built, and a built record lands in the
 * first free of the eight 0x40B0F6 slots. The record builder itself
 * (0x11C106..0x11C1D4: two-box midpoint or RNG point, needing 0x006422 and
 * 0x001D8A) is not translated: reaching it returns -1 with +0x35 cleared,
 * which is all the branch structure does before it. */
int Obj_126FF6_HitSpark(const f3mem_t *m, uint32_t obj)
{
    int r = 0;
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit != 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | hit));
        if ((rd8(m, obj + 0x34u) & 0x02u) == 0u) {
            uint16_t now = rd16(m, F3_A5 + 0x0878u);
            uint16_t d0w = (uint16_t)(now - rd16(m, F3_A5 + 0x311Cu));
            wr16(m, F3_A5 + 0x311Cu, now);
            if ((int16_t)d0w < 2) {
                uint32_t prev = rd32(m, F3_A5 + 0x3116u);
                wr32(m, F3_A5 + 0x3116u, obj);
                if (prev == obj) {
                    uint16_t c = (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u);
                    wr16(m, F3_A5 + 0x311Au, c);
                    if ((int16_t)c <= 3)
                        goto done;                  /* 0x11C1D8 */
                }
            }
            wr16(m, F3_A5 + 0x311Au, 0u);
            uint32_t a2 = 0x0040B0F6u;
            for (int i = 0; i < 8; i++, a2 += 4u) {
                uint8_t b = rd8(m, a2);
                wr8(m, a2, (uint8_t)(b | 0x80u));
                if ((b & 0x80u) == 0u) {
                    r = -1;                     /* 0x11C106 record not translated */
                    break;
                }
            }
        }
    }
done:
    wr8(m, obj + 0x35u, 0u);                        /* 0x11C1D8 */
    return r;
}

/* ROM 0x126FF6 */
int Obj_126FF6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x126FF6: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x127044 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);                           /* 0x127046 */
        return 0;
    }

    wr32(m, obj + 0x24u, (uint32_t)((int32_t)rd32(m, obj + 0x24u) >> 2)); /* asr.l #2 */
    sub_11AE62(m, obj, 0x000186A0u, 3u);            /* 0x127014 */
    if (Obj_126FF6_HitSpark(m, obj) < 0)            /* 0x12701A */
        return -1;

    /* 0x127020: lea (0x30,PC),A1 -> 0x127022 + 0x30 = 0x127052 */
    Obj_Animate(m, obj, 0x00127052u);               /* jsr 0x0CBD52 */
    Obj_11B06E_Tick(m, obj);                        /* 0x12702C */

    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x127036 */
    Obj_11B09A_Tick(m, obj);                        /* 0x12703C */

    wr32(m, obj + 0x00u, 0x00126FF6u);              /* 0x127042 -> 0x126FEE */
    return 0;
}
