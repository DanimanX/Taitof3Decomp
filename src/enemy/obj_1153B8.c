#include "obj_1153B8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x1153B8: the 0x11557A object's one-shot launch. A0 = obj, A1 = obj+8
 * (the 0x1159xx spawn record).
 *
 *   0x1153B8  btst #5,+0x50(A1) -> 0x11551C: Obj_Free(obj)
 *   else if +0x50 bit 4 or bit 3, or (A1+0x24) <= 0 -> 0x115412
 *   else: A1+0x35 |= obj+0x35 ; obj+0x24 -> A1+0x24 ; A1+0x24 += it
 *         when that sum <= 0: 0x0CBDDE credits (A5+0x1D50)+0x48 by 0x4E20 and
 *         A1+0x50 |= 0x40
 *   0x115412  obj+0x35 := 0 ; obj+0x1D := A1+0x1D
 *   0x11541C  tst.w obj+0x22: 0 -> obj+{0x14,0x18,0x20} := A1+same (16-bit)
 *                             else -> 0x11552A: obj.{x,y} := A1.{x,y} (32-bit)
 *                             and obj+0x20 eases toward A1+0x20 by
 *                             (A1+0x20 - obj+0x20) >> (A2+0x22), then moves
 *                             along obj+0x20 >> 8 (0x0CBE12)
 *   0x115442  angle = obj+0x20 >> 8 (neg.b when A1+0x50 bit 3, +2 on obj+0x1C);
 *             idx = angle >> 4, +3 on obj+0x1C when that has bit 3; 6-byte
 *             record at A1.obj+0xC + idx*6 {dx.b, dy.b, anim.l}: x -= dx,
 *             y -= dy, obj+0x10 := anim, y -= anim[0]*8, x -= anim[2]*8, draw
 *   0x1154D2  A1+0x50 bit 1/bit 4 -> done. Otherwise obj+0x28 := 0x10000 and:
 *             bit 3 clear -> 0x0CBDA4 (deferred-draw list, then hazard list)
 *             bit 3 set   -> append to the list at A5+0x902
 *   0x115518  bra.w 0x1153B0, whose tail draws the sprite itself.
 *
 * `obj` arrives with 0x400000 already stripped (the WRAM image offset), so the
 * pointers stored into it, and the 0x1159xx record it points at, are
 * offset-relative too. */
int Obj_1153B8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 8u);

    /* 0x1153BC: btst.b #5,(0x50,A1) ; bne.w 0x11551C */
    if ((rd8(m, a1 + 0x50u) & 0x20u) != 0u) {
        /* 0x11551C: move.l A0,-(SP) ; jsr 0x105EC ; lea (4,SP),SP ; rts */
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x1153C6/0x1153D0/0x1153DA: bits 4 and 3, then tst.l (0x24,A1) */
    if ((rd8(m, a1 + 0x50u) & 0x10u) == 0u &&
        (rd8(m, a1 + 0x50u) & 0x08u) == 0u &&
        (int32_t)rd32(m, a1 + 0x24u) > 0) {
        /* 0x1153E2: move.b (0x35,A0),D0 ; or.b D0,(0x35,A1) */
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | rd8(m, obj + 0x35u)));

        /* 0x1153EA: move.l (0x24,A0),D0 ; clr.l (0x24,A0) ; add.l D0,(0x24,A1) */
        {
            uint32_t d0 = rd32(m, obj + 0x24u);
            wr32(m, obj + 0x24u, 0u);
            d0 = d0 + rd32(m, a1 + 0x24u);
            wr32(m, a1 + 0x24u, d0);

            /* 0x1153F6: bgt.w 0x115412 */
            if ((int32_t)d0 <= 0) {
                /* 0x1153FA: jsr 0x0CBDDE ; addi.l #0x4E20,(0x48,A1) */
                uint32_t a1c = rd32(m, a1 + 0x36u);
                if (rd8(m, a1 + 0x35u) == 1u)
                    a1c = rd32(m, a1c + 0x08u);
                wr32(m, a1c + 0x48u, rd32(m, a1c + 0x48u) + 0x4E20u);
                a1 = rd32(m, obj + 8u);                 /* 0x115408 */
                wr8(m, a1 + 0x50u, (uint8_t)(rd8(m, a1 + 0x50u) | 0x40u)); /* bset #6 */
            }
        }
    }

    /* 0x115412: sf.b (0x35,A0) ; move.b (0x1D,A1),(0x1D,A0) */
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));

    /* 0x11541C: tst.w (0x22,A0) ; bne.w 0x11543E */
    if (rd16(m, obj + 0x22u) == 0u) {
        /* 0x115424: movea.l (0x8,A0),A1 ; copy the three 16-bit values */
        a1 = rd32(m, obj + 8u);
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
    } else {
        /* 0x11543E: bsr.w 0x11552A */
        uint32_t a2 = rd32(m, obj + 8u);
        a1 = rd32(m, obj + 0x44u);
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));      /* 0x115532 */
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));      /* 0x115538 */
        {
            uint16_t d0 = rd16(m, obj + 0x20u);
            int16_t d1 = (int16_t)(rd16(m, a1 + 0x20u) - d0);
            uint16_t d2 = (uint16_t)(rd16(m, a2 + 0x22u) & 0x3Fu);
            d1 = (int16_t)((int32_t)d1 >> d2);          /* asr.w keeps the sign */
            d0 = (uint16_t)(d0 + (uint16_t)d1);
            wr16(m, obj + 0x20u, d0);                   /* 0x115550 */
            /* 0x115554/0x115556: lsr.w #8 ; jsr 0x0CBE12 */
            Obj_MoveByAngle(m, obj, (uint16_t)(d0 >> 8));
        }
    }

    /* 0x115442: x and y are saved on the stack over the draw */
    {
        uint16_t x = rd16(m, obj + 0x14u);
        uint16_t y = rd16(m, obj + 0x18u);
        uint32_t a1b = rd32(m, obj + 0x0Cu);
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
        uint32_t a2 = rd32(m, obj + 8u);

        wr8(m, obj + 0x1Cu, 0x03u);                     /* 0x115454 */

        /* 0x11545E: btst.b #3,(0x50,A2) ; beq 0x115470 */
        if ((rd8(m, a2 + 0x50u) & 0x08u) != 0u) {
            d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(0u - (uint8_t)d0)); /* neg.b */
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x02u));   /* eori.b #2 */
        }

        /* 0x115470: lsr.w #4 ; bclr.b #3 ; non-zero -> eori.b #3 */
        d0 = (uint16_t)(d0 >> 4);
        if ((d0 & 0x08u) != 0u)
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x03u));
        d0 = (uint16_t)(d0 & ~0x08u);

        /* 0x115480: mulu.w #6,D0 ; adda.w D0,A1 */
        a1b = a1b + (uint32_t)(uint16_t)(d0 * 6u);
        {
            int16_t dx = (int8_t)rd8(m, a1b);
            int16_t dy = (int8_t)rd8(m, a1b + 1u);
            uint32_t anim = rd32(m, a1b + 2u);

            if ((rd8(m, obj + 0x1Cu) & 0x01u) != 0u)    /* 0x11548A: bit 0 */
                dx = (int16_t)(0u - (uint16_t)dx);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));

            if ((rd8(m, obj + 0x1Cu) & 0x02u) != 0u)    /* 0x11549E: bit 1 */
                dy = (int16_t)(0u - (uint16_t)dy);
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));

            wr32(m, obj + 0x10u, anim);                 /* 0x1154AE */
            /* 0x1154B4: move.w (A1)+,D0 ; lsl.w #3 ; sub.w D0,(0x18,A0)
             * 0x1154BC: move.w (A1)+,D0 ; lsl.w #3 ; sub.w D0,(0x14,A0) */
            wr16(m, obj + 0x18u,
                 (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, anim) << 3)));
            wr16(m, obj + 0x14u,
                 (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, anim + 2u) << 3)));
        }

        Obj_DrawSprite(m, obj);                         /* 0x1154C4: jsr 0x1282 */

        /* 0x1154CA/0x1154CE: restore y, then x */
        wr16(m, obj + 0x18u, y);
        wr16(m, obj + 0x14u, x);
    }

    /* 0x1154D2 */
    a1 = rd32(m, obj + 8u);
    if ((rd8(m, a1 + 0x50u) & 0x02u) != 0u)             /* btst #1 -> 0x115518 */
        return 0;
    if ((rd8(m, a1 + 0x50u) & 0x10u) != 0u)             /* btst #4 -> 0x115518 */
        return 0;

    wr32(m, obj + 0x28u, 0x00010000u);                  /* 0x1154F4 / 0x115506 */
    if ((rd8(m, a1 + 0x50u) & 0x08u) == 0u) {
        /* 0x1154FC: jsr 0x0CBDA4 -- A5+0x8FE, then falls into A5+0x906 */
        uint32_t p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
        p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
    } else {
        /* 0x11550E: movea.l (0x902,A5),A1 ; move.l A0,(A1)+ ; move.l A1,(0x902,A5) */
        uint32_t p = rd32(m, F3_A5 + 0x0902u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0902u, p + 4u);
    }
    /* 0x115518: bra.w 0x1153B0 */
    return 0;
}
