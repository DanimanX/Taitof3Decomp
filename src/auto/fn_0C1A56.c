/* ROM 0x0C1A56: (recipe capture fnk_, work/zcap/recipes15.tsv): zoom/scale-in-out sprite tick with a
 * looping animation table (A0 = object). Sibling of 0x0C1998 (Fn_0C1998).
 * Writes +0x60 into both zoom bytes (+0x1E, +0x1F), places (+0x14,+0x18) at
 * (+0x44,+0x48) + (+0x60 * 0x38 << 8), steps the looping animation table at
 * +0x4C through 0x0CBD52, draws the sprite (0x001282), integrates
 * (+0x44,+0x48) += (+0x50,+0x54), accelerates (+0x50,+0x54) by (0x180, 0x200)
 * and decrements +0x60 while it stays >= 0 (re-arming 0x0C1A56 through the
 * 0x0C1A4E thunk); otherwise it clears A5+0x8E2 and frees the object. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x001282 as a call: Obj_DrawSprite plus the register trace of its
 * block-control loop (batch_sprite_outputs in batch_obj_regs.h, which
 * follows the single-sprite path at 0x13CE and the tile grid at 0x1330). */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    Obj_DrawSprite(m, r->a[0]);
    batch_sprite_outputs(m, r);
}

/* ROM 0x0CBD52: looping {dur.w, sprdef.l} animation step; a negative duration
 * word rewinds the index to 0. */
static void sub_0CBD52(const f3mem_t *m, uint32_t obj, uint32_t tbl)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

int Fn_0C1A56(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t sc = rd16(m, obj + 0x60u);                   /* 0x0C1A56 move.w (0x60,A0),D0w */
    uint32_t d0;

    wr8(m, obj + 0x1Fu, (uint8_t)sc);                     /* 0x0C1A5A move.b D0b,(0x1F,A0) */
    wr8(m, obj + 0x1Eu, (uint8_t)sc);                     /* 0x0C1A5E move.b D0b,(0x1E,A0) */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));           /* 0x0C1A62 */
    d0 = ((uint32_t)rd16(m, obj + 0x60u) * 0x38u) << 8;   /* 0x0C1A68..0x0C1A70 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);      /* 0x0C1A72 */

    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));           /* 0x0C1A76 */
    d0 = ((uint32_t)rd16(m, obj + 0x60u) * 0x38u) << 8;   /* 0x0C1A7C..0x0C1A84 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);      /* 0x0C1A86 */

    r->a[1] = rd32(m, obj + 0x4Cu);                       /* 0x0C1A8A movea.l (0x4C,A0),A1 */
    sub_0CBD52(m, obj, r->a[1]);                          /* 0x0C1A8E jsr 0x0CBD52 */
    draw_sprite_regs(m, r);                               /* 0x0C1A94 jsr 0x001282 */

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + rd32(m, obj + 0x50u)); /* 0x0C1A9A */
    d0 = rd32(m, obj + 0x54u);                            /* 0x0C1AA2 move.l (0x54,A0),D0 */
    r->d[0] = d0;
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + d0);      /* 0x0C1AA6 */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + 0x00000180u); /* 0x0C1AAA */
    wr32(m, obj + 0x54u, d0 + 0x00000200u);               /* 0x0C1AB2 */

    {
        int16_t rem = (int16_t)(rd16(m, obj + 0x60u) - 1u); /* 0x0C1ABA subq.w #1 */
        wr16(m, obj + 0x60u, (uint16_t)rem);
        if (rem >= 0) {                                   /* 0x0C1ABE bge */
            wr32(m, obj, 0x000C1A56u);                    /* 0x0C1A4E re-arm thunk */
            return 0;
        }
    }
    wr32(m, F3_A5 + 0x08E2u, 0u);                         /* 0x0C1AC0 clr.l */
    Obj_Free(m, obj);                                     /* 0x0C1AC6 jsr 0x0105EC */
    return 0;                                             /* 0x0C1AD0 rts */
}
