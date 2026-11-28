#include "fn_auto.h"
/* ROM 0x0C7B48: player "continue?" banner tick (A0 = object). Steps the
 * {dur.w, sprdef.l} animation table at 0x0C928A through 0x0CBD24 (which also
 * reloads +0x10 with the frame's sprite definition), then acts on the
 * duration word the step stopped on:
 *  - negative: +0x06 = 0x1E and the object moves to 0x0C7B7C (the game-over
 *    countdown); the beq at 0x0C7B72 can never be taken (0x1E != 0), so the
 *    block at 0x0C7B82 belongs to the 0x0C7B7C entry.
 *  - otherwise: unless bit 2 of +0x65 is set, the sprite is drawn at
 *    +0x14/+0x18 offset by the screen shake A5+0x1D20/+0x1D24 (0x0C8460 ->
 *    0x001282, which leaves D0/D1 and A1 at the sprite cursor), then the
 *    position is restored.
 * Either way the tick re-arms itself through the 0x0C7B40 thunk. */
#include "batch_obj_regs.h"          /* batch_sprite_outputs (0x001282 results) */
#include "../video/sprite.h"         /* Obj_DrawSprite (0x001282) */

/* ROM 0x0CBD24 with D0/D1 register tracking: walk the {dur.w, sprdef.l}
 * records at `tbl` from the offset at +0x3A, one frame per call. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);                 /* 0x0CBD24 move.w */
        *d0_io = off;                                        /* move.w clears D0 high */
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)                                        /* 0x0CBD2C ble */
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30 addq.w */
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = cnt;                                        /* 0x0CBD34 move.w clears D1 high */
        if ((int16_t)cnt <= dur) {                           /* 0x0CBD3C ble 0x0CBD4A */
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                            /* 0x0CBD40 clr.w */
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));          /* 0x0CBD44 addq.w #6 */
    }
}

int Fn_0C7B48(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t tbl = 0x000C928Au;                              /* 0x0C7B48 lea (0x1740,PC),A1 */
    r->a[1] = tbl;

    sub_0CBD24(m, obj, tbl, &r->d[0], &r->d[1]);             /* 0x0C7B4E jsr 0x0CBD24 */

    int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]);
    if (dur < 0) {                                           /* 0x0C7B58 blt 0x0C7B6C */
        wr16(m, obj + 0x06u, 0x001Eu);                       /* 0x0C7B6C */
        wr32(m, obj, 0x000C7B7Cu);                           /* 0x0C7B74 */
        return 0;                                            /* 0x0C7B7A rts */
    }

    if ((rd8(m, obj + 0x65u) & 0x04u) == 0u) {               /* 0x0C7B5C btst #2 ; bne */
        uint16_t sx = rd16(m, obj + 0x14u);                  /* 0x0C8460 push */
        uint16_t sy = rd16(m, obj + 0x18u);
        wr16(m, obj + 0x14u, (uint16_t)(sx + rd16(m, F3_A5 + 0x1D20u))); /* 0x0C8468 */
        wr16(m, obj + 0x18u, (uint16_t)(sy + rd16(m, F3_A5 + 0x1D24u))); /* 0x0C8470 */
        Obj_DrawSprite(m, obj);                              /* 0x0C8478 jsr 0x001282 */
        batch_sprite_outputs(m, r);                          /* D0/D1/A1 as 0x001282 */
        wr16(m, obj + 0x18u, sy);                            /* 0x0C847E */
        wr16(m, obj + 0x14u, sx);                            /* 0x0C8482 */
    }

    wr32(m, obj, 0x000C7B48u);                               /* 0x0C7B6A -> 0x0C7B40 */
    return 0;                                                /* 0x0C7B46 rts */
}
