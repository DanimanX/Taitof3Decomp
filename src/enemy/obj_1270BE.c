#include "obj_1270BE.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"
#include "obj_126FF6.h"

/* Shared tracker helpers of the 0x11B/0x11C family. Bodies lifted verbatim
 * from the verified copies: 0x0CC2BC from src/enemy/obj_12EB90.c, 0x0CBDDE
 * and 0x11AE62 from src/enemy/obj_126DEC.c. 0x11C096 is reached through
 * Obj_126FF6_HitSpark (src/enemy/obj_126FF6.c). */

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

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

/* ROM 0x1270BE */
int Obj_1270BE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1270BE: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne.w 0x12724E */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);                           /* 0x127250 */
        return 0;
    }

    wr32(m, obj + 0x24u, (uint32_t)((int32_t)rd32(m, obj + 0x24u) >> 2)); /* asr.l #2 */
    sub_11AE62(m, obj, 0x000186A0u, 3u);            /* 0x1270DC */
    /* 0x1270E2: jsr 0x11C096 -- the hit spark. The record builder at
     * 0x11C106..0x11C1D4 is not translated, so that path is reported. */
    if (Obj_126FF6_HitSpark(m, obj) < 0)
        return -1;

    /* 0x1270E8: lea (0x172,PC),A1 -> 0x1270EA + 0x172 = 0x12725C */
    Obj_Animate(m, obj, 0x0012725Cu);               /* jsr 0x0CBD52 */
    Obj_11B06E_Tick(m, obj);                        /* 0x1270F4 */

    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x1270FA */
    Obj_11B09A_Tick(m, obj);                        /* 0x127104 */

    /* 0x12710A: bftst (0x30EE,A5){0xA:1} -- 0x0CCD04 bullet, arg 0x0CCD04 */
    if (rd16(m, F3_A5 + 0x30EEu) & 0x0020u) {
        if (sub_0CC2BC(m, 12u) == 0u) {
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 2u, 9u, 0x000CCD04u);
        }
    }

    /* 0x12715A: bftst (0x30EE,A5){0xB:1} -- two 0x0CDDB0 shots, arg 0x0CDDB0 */
    if (rd16(m, F3_A5 + 0x30EEu) & 0x0010u) {
        if (sub_0CC2BC(m, 0x3Cu) == 0u) {
            wr16(m, obj + 0x3Eu, (uint16_t)-0x0014);   /* 0x127174 */
            (void)Obj_Spawn(m, obj, 0x000CDDB0u, 2u, 5u, 0x000CDDB0u);
            (void)Obj_Spawn(m, obj, 0x000CDDB0u, 2u, 5u, 0x000CDDB0u);
        }
    }

    /* 0x1271E6: bftst (0x30EE,A5){0x10:1} -- two 0x11B5E6 bullets when the
     * RNG roll lands */
    if (rd16(m, F3_A5 + 0x30EEu) & 0x0008u) {
        uint16_t r = sub_0CC2BC(m, 14u);
        if (r == 0u) {
            r = (uint16_t)((r >> 16) & 0x07u);      /* swap D0 ; andi.w #7 */
            if ((int16_t)r <= 2) {
                (void)Obj_Spawn(m, obj, 0x0011B5E6u, 2u, 5u, 0x0F28D800u);
                (void)Obj_Spawn(m, obj, 0x0011B5E6u, 2u, 5u, 0x002EE800u);
            }
        }
    }

    wr32(m, obj + 0x00u, 0x001270BEu);              /* 0x12724A -> 0x1270B6 */
    return 0;
}
