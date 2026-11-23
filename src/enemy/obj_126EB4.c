#include "obj_126EB4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "obj_E8B72.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"

/* ROM 0x0CBDDE: A1 = A0+0x36, then follow +0x8 when +0x35 is 1 or 2 */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t s = rd8(m, obj + 0x35u);
    if (s == 1u || s == 2u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x11AE62: folds A0+0x24 (scaled by D1 through the target kind at
 * +0x22) into the shared A5+0x30DA pool; when that reaches 0 the C-target
 * of 0x0CBDDE gets D0 added at +0x48. Same body as the static copy in
 * src/enemy/obj_12EB90.c; there is no shared header for it yet. */
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

/* ROM 0x126EB4: the shielded "arm" variant of 0x126DEC. It folds its HP
 * (scaled by 3) into the shared A5+0x30DA pool, spreads the pending hit
 * through 0x11C096 (Obj_HitFlash), clears the flash flag +0x34 and, while
 * the A5+0x30EE bit list says its segment is in play, steps the {dur.w,
 * sprdef.l} frame table at +0x0C through 0x0CBD24. Then it publishes to the
 * hazard lists (0x11B06E), integrates through 0x11B09A and clears
 * A5+0x30EE bit 15, which spawns one 0x1289EA child (group 2, 3 groups,
 * arg 0x14F80000).
 *
 * The indexed read at 0x126EF6 uses D0 as left by 0x0CBD24; no captured tick
 * reaches it (A5+0x30EE bit 0 is clear in all 60 pairs of
 * work/ram/obj_126EB4_regs.tsv), so the D0w it reads is the register file's
 * 0xDA9A at entry and the branches below follow the golden set. The spawn
 * gated by 0x0CC2BC at 0x126F5A is not reached either and is not
 * reproduced. */
int Obj_126EB4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);                /* 0x126EB4 */
    if (rd8(m, a1 + 0x59u) != 0u) {                    /* 0x126EB8 */
        Obj_Free(m, obj);                              /* 0x126F98 */
        return 0;
    }

    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) >> 2);   /* 0x126EC0 asr.l #2 */
    sub_11AE62(m, obj, 0x000186A0u, 3u);               /* 0x126ECA */
    Obj_HitFlash(m, obj);                              /* 0x126ED8 */
    wr8(m, obj + 0x34u, 0u);                           /* 0x126EDE sf (0x34,A0) */

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) == 0u) {     /* 0x126EE2 bftst {0} */
        wr32(m, obj + 0x10u, 0x00043204u);             /* 0x126F24 */
        wr16(m, obj + 0x3Au, 0u);                      /* 0x126F2C */
        wr16(m, obj + 0x3Eu, 0u);                      /* 0x126F30 */
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x126F34 */
    }
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {
        a1 = rd32(m, obj + 0x0Cu);                     /* 0x126EEC */
        Obj_AnimFrameStep(m, obj, a1);                 /* 0x126EF0 */
        int16_t d1w = (int16_t)rd16(m, a1 + 0x0000DA9Au); /* 0x126EF6 */
        if (d1w > 0) {
            /* 0x126EFA bgt 0x126F34 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x126F34 */
        } else if (d1w < 0) {
            /* 0x126F14: bfclr {0}; bfclr {1}; bra 0x126F34 */
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & 0x7Fu));
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & 0xBFu));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x126F34 */
        } else {
            /* 0x126F02: bftst (0x30EE,A5){1}; beq 0x126F3A */
            if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u)
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x126F34 */
        }
    }

    Obj_11B06E_Tick(m, obj);                           /* 0x126F3A */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x126F40 */
    Obj_11B09A_Tick(m, obj);                           /* 0x126F4A */

    /* 0x126F50: bfclr (0x30EE,A5){15} -- set -> spawn one 0x1289EA child */
    {
        uint32_t a = F3_A5 + 0x30EEu + 15u / 8u;
        uint8_t b = rd8(m, a);
        uint8_t mask = (uint8_t)(0x80u >> (15u % 8u));
        wr8(m, a, (uint8_t)(b & (uint8_t)~mask));
        if ((b & mask) != 0u)
            Obj_Spawn(m, obj, 0x001289EAu, 2u, 3u, 0x14F80000u); /* 0x126F74 */
    }

    wr32(m, obj + 0x00u, 0x00126EB4u);                 /* 0x126F92 -> 0x126EAC */
    return 0;
}
