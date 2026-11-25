#include "obj_FF232.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "obj_E8B72.h"

/* ROM 0x0FF232: wait/materialise phase, then a ballistic phase.
 *
 * While +0x24 (lifetime) is positive and the pause bit (A5+0x144B bit5) is
 * clear, the object integrates the scroll velocities A5+0x4224/+0x4228 into
 * the 16.16 accumulators +0x3A/+0x3E, mirrors them to +0x14/+0x18, and dies
 * (0x0CEE74) when the 6-pixel hitbox point hits terrain or the off-screen
 * flag is set; otherwise script step, hazard list, sprite. With the lifetime
 * spent, or the pause bit set, it sinks by 2 px per axis and spawns one
 * 0x0DBF16 child (group 9, 2 objects) carrying the (dx,dy) long returned by
 * 0x0D2D44 on (+0x58, +0x60) into +0x50/+0x54, then parks at 0x0CEE74. */

/* ROM 0x0D2D44: (D0.l, D1.l) = (tbl[d0w].x, tbl[d0w].y) * d1w, tables at
 * 0x0D2D64 (dx) and 0x0D2ECC (dy), indexed by the signed word D0. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t a0 = 0x000D2D64u + (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d1 = rd32(m, a0 + 0x0168u);        /* (0x182,PC) = 0x0D2ECC */
    uint32_t d0 = rd32(m, a0);                  /* (0xD2D64,PC) = 0x0D2D64 */

    /* jsr ([0x21CC,PC],D2w*4) -> 0x0D4F26[d1w] (0x0D4FF2..0x0D538A):
     * block k computes D1 = k*D1, D0 = k*D0 (both 32-bit). */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
}

int Obj_FF232_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0FF232: btst.b #1,(0x1D58,A5) ; beq 0x0FF256
     * 0x0FF23A: cmpi.w #-0xA,(0x14,A0) ; blt 0x0FF256
     * 0x0FF242: cmpi.w #0x11C,(0x14,A0) ; bgt 0x0FF256 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            /* 0x0FF24A: move.b (0x4F,A0),(0x1D,A0) ; jmp 0x0CBB2C */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_OrbitDebug(m, obj);             /* 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0FF256: tst.l (0x24,A0) ; ble.w 0x0FF2E6
     * 0x0FF25E: btst.b #5,(-0x144B,A5) ; bne.w 0x0FF2E6 */
    if ((int32_t)rd32(m, obj + 0x24u) > 0 &&
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        /* 0x0FF268: integrate the scroll velocities */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

        /* 0x0FF278: bclr.b #1,(0x7,A0) ; beq 0x0FF288 */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            /* 0x0FF282: jsr 0x0CF1E4 (script interpreter) */
            if (ObjScript_CF1E4(m, obj) < 0) {
                wr8(m, obj + 0x07u, f7);        /* restore: path has no effect */
                return -1;
            }
        }

        /* 0x0FF288: tst.b (0x20,A0) ; beq 0x0FF29A
         * 0x0FF290: lea (0x94,PC),A1 -> 0x0FF326 ; jsr 0x0CF4B8.
         * A1 is only read by CF4B8 mode 15; for every mode CF4B8 can
         * implement the helper's A1 is unused (modes it cannot implement
         * return -1 below). */
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x000FF326u) < 0)
                return -1;
        }

        /* 0x0FF29A: jsr 0x0CF85A */
        Obj_FlagOffscreen(m, obj);

        /* 0x0FF2A0: btst.b #3,(0x7,A0) ; bne.l 0x0CEE74 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);             /* 0x0CEE74 */
            return 0;
        }

        /* 0x0FF2AC: move.l (0x3A,A0),(0x14,A0) ; move.l (0x3E,A0),(0x18,A0) */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

        /* 0x0FF2B8: D0w = (0x14,A0).w + 6 ; D1w = (0x18,A0).w + 6 ;
         * jsr 0x0C6632 ; tst.b D0b ; bne.l 0x0CEE74 */
        {
            int16_t x = (int16_t)(rd16(m, obj + 0x14u) + 6u);
            int16_t y = (int16_t)(rd16(m, obj + 0x18u) + 6u);
            int r = Terrain_TestPoint(m, x, y);
            if ((uint8_t)r != 0u) {
                Obj_StartDying(m, obj);         /* 0x0CEE74 */
                return 0;
            }
        }

        /* 0x0FF2D2: jsr 0x0D0690 ; jsr 0x0D0DC2 ; jsr 0x001282 ; rts */
        Obj_StepScript(m, obj);
        Obj_0D0DC2(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

    /* 0x0FF2E6 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 2u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u));

    /* 0x0FF2EE: jsr 0x0D2D44 ; move.l D0,(0x50,A0) ; move.l D1,(0x54,A0) */
    {
        uint32_t d0, d1;
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1);
        wr32(m, obj + 0x50u, d0);
        wr32(m, obj + 0x54u, d1);
    }

    /* 0x0FF304: pea #0 ; move.w #2,-(SP) ; move.w #9,-(SP) ;
     * move.l #0x0DBF16,-(SP) ; jsr 0x01057C */
    (void)Obj_Spawn(m, obj, 0x000DBF16u, 9u, 2u, 0u);

    /* 0x0FF320: bra.l 0x0CEE74 */
    Obj_StartDying(m, obj);
    return 0;
}
