#include "obj_126DEC.h"
#include "../objects/obj_pool.h"
#include "obj_11B09A.h"

/* Shared tracker helpers of the 0x11B/0x11C family. They are still static
 * copies inside the routines that need them (e.g. src/enemy/obj_12EB90.c);
 * there is no shared header yet, so the three this tick uses are reproduced
 * below verbatim from that verified copy. */

/* ROM 0x0CBDDE: A1 = A0+0x36, then follow +0x8 when +0x35 is 1 or 2 */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t s = rd8(m, obj + 0x35u);
    if (s == 1u || s == 2u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x11AE62: A0+0x24 is subtracted from the global A5+0x30DA counter;
 * when it reaches 0 the target (0x0CBDDE) gets D0 added at +0x48. D2 is
 * scaled by D1 according to the target kind at +0x22. */
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

/* ROM 0x11C32C: hit flash. A pending hit byte at +0x35 is either consumed
 * (current frame 6 becomes blank 8) or turns the sprite red (6). */
static void sub_11C32C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 6u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 8u);
        } else {
            wr8(m, obj + 0x1Du, 6u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x126DEC: one of the two 20-frame spawner "arms" of the final fight.
 * It pays its HP into the shared A5+0x30DA pool (scaled by 5), publishes
 * itself to the hazard lists, blinks, aims 35.0 degrees west (0x2300) at
 * +0x20/+0x54, integrates through 0x11B09A and, on the pool's own hit-flag
 * bits 12/13 at A5+0x30EE, spawns 0x1281C4 children into group 2 (arg 0).
 * Only the spawn pairs that are not taken by the golden set are reproduced
 * (all 60 captured ticks take the disarm path). */
int Obj_126DEC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x126DEC: movea.l (0x8,A0),A1; tst.b (0x59,A1); bne 0x126E76 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);                              /* 0x126E78 */
        return 0;
    }

    sub_11AE62(m, obj, 0x000186A0u, 5u);               /* 0x126DF8 */
    sub_11C32C(m, obj);                                /* 0x126E06 */

    /* 0x126E12: copy the parent palette and integrate the position */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du));
    Obj_11B09A_Tick(m, obj);                           /* 0x126E1C */

    /* 0x126E22 / 0x126E4A: bfclr (0x30EE,A5){12} and {13}; each spawns a
     * 0x1281C4 child (group 2, 3 groups, arg -0x7F001C / -0x86000E) */
    {
        uint32_t a = F3_A5 + 0x30EEu + 12u / 8u;
        uint8_t b = rd8(m, a);
        wr8(m, a, (uint8_t)(b & (uint8_t)~(0x80u >> (12u % 8u))));
        if ((b & (0x80u >> (12u % 8u))) != 0u)
            Obj_Spawn(m, obj, 0x001281C4u, 2u, 3u, 0xFF80001Cu);
    }
    {
        uint32_t a = F3_A5 + 0x30EEu + 13u / 8u;
        uint8_t b = rd8(m, a);
        wr8(m, a, (uint8_t)(b & (uint8_t)~(0x80u >> (13u % 8u))));
        if ((b & (0x80u >> (13u % 8u))) != 0u)
            Obj_Spawn(m, obj, 0x001281C4u, 2u, 3u, 0xFF79FFF2u);
    }

    wr32(m, obj + 0x00u, 0x00126DECu);                 /* 0x126E72 -> 0x126DE4 */
    return 0;
}
