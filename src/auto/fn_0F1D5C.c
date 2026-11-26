/* ROM 0x0F1D5C: carried/layered turret-gun enemy tick (A0 = object,
 * A6 = its carrier). While HP (+0x24) > 0 and the bomb flag (A5-0x144B bit 5)
 * is clear: subtract the vertical scroll delta (A5+0x4228) from +0x3E, run the
 * script (0x0CF1E4) when bit 1 of +0x07 was set, 0x0CF4B8 with the aim record
 * 0x0F3AF0 when +0x20 != 0; when +0x3A < -0x20 it dies (0x0CEE74), else it
 * copies (+0x3A,+0x3E) to (+0x14,+0x18), steps the animation (0x0D0690),
 * registers in the collision lists (0x0D0DC2), blinks the hit colour
 * (0x0CBE34) and draws its sprite (0x001282). HP <= 0 or the bomb flag: score
 * 0xBB8 (0x0D0E04), spawn one child 0x0DC486 (group 3, count 2, arg 0) offset
 * by (+0x10, +0x10) with +0x58 = 0xB4 and +0x60 inherited, then the bonus
 * death tail (0x0CEE48). On screen (bit 1 of A5+0x1D58, X in [-10,0x11C]):
 * +0x1D = +0x4F, score 0xBB8 (0x0D0E30) and the debug death tail (0x0CEE9A).
 * The carrier is then released (0x0F1F5C): 0x0CEC4C seeds the object from the
 * gun record 0x0F0E22, and the burst state 0x0F1E84 is installed as the tick
 * (a next-frame routine here, not a callee). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

/* Parameter records / jump table of the ROM (dumped from work/maincpu.bin). */
#define REC_BURST_PARAMS 0x000F3AF0u  /* lea (0x1D3C,PC),A1 at 0x0F1DB2 */

/* ROM 0x0F1F5C: drop the carrier and switch to the burst state.
 *   43FA EABE  lea     (0xEABE,PC),A1        ; 0x0F0E22, the carrier's seed
 *   4EB9 000CEC4C jsr  0x0CEC4C              ; Obj_SeedFromParams(A1)
 *   4EB9 000D0C7A jsr  0x0D0C7A              ; D0.w = difficulty fire chance
 *   5240       addq.w #1,D0
 *   5240       addq.w #1,D0
 *   5240       addq.w #1,D0
 *   5240       addq.w #1,D0                  ; 4 shots over 0xD0C7A's value
 *   1140 0021  move.b D0,(0x21,A0)           ; shots left
 *   1140 0022  move.b D0,(0x22,A0)           ; failures allowed
 *   117C 0001 0023 move.b #1,(0x23,A0)       ; one idle frame before shot 1
 *   217C 000F1E84 0000 move.l #0x0F1E84,(A0)
 *   4E75       rts
 * 0x0D0C7A only reads A5 (the difficulty byte A5+0x87F); A1 is the record. */
static void sub_0F1F5C(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t d0 = 4u + (uint32_t)rd16(m, 0x000D0CC2u +
                             (uint32_t)(rd8(m, F3_A5 + 0x087Fu) >> 3) * 2u);
    *a1_io = 0x000F0E22u;                                   /* 0x0F1E5C lea */
    (void)Obj_SeedFromParams(m, obj, *a1_io);               /* 0x0F1E60 */
    wr8(m, obj + 0x21u, (uint8_t)d0);                       /* 0x0F1E6E */
    wr8(m, obj + 0x22u, (uint8_t)d0);                       /* 0x0F1E72 */
    wr8(m, obj + 0x23u, 0x01u);                             /* 0x0F1E76 */
    wr32(m, obj, 0x000F1E84u);                              /* 0x0F1E7C */
}

int Fn_0F1D5C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {            /* 0x0F1D5C */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                      /* 0x0F1D64..0x0F1D72 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));         /* 0x0F1D74 */
            Score_AddCurrent(m, 0x00000BB8u);                 /* 0x0F1E4A..0x0F1E50 */
            Obj_DieWithBonusDbgChk(m, obj, 0u);               /* 0x0F1E56 -> 0x0CEE9A */
            r->d[0] = 0x00000BB8u;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                 /* 0x0F1D80..0x0F1D84 */
        Score_OnHit(m, obj, 0x00000BB8u);                     /* 0x0F1DEE..0x0F1DF4 */
        r->d[0] = 0x00000BB8u;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {     /* 0x0F1D88..0x0F1D8E */
        uint32_t d0 = rd32(m, F3_A5 + 0x4228u);               /* 0x0F1D92 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);      /* 0x0F1D96 */
        r->d[0] = d0;

        {
            uint8_t f7 = rd8(m, obj + 0x07u);                 /* 0x0F1D9A bclr #1 */
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if ((f7 & 0x02u) != 0u) {
                int rc = ObjScript_CF1E4(m, obj);             /* 0x0F1DA4 */
                if (rc < 0)
                    return -1;
            }
        }

        if (rd8(m, obj + 0x20u) != 0u) {                      /* 0x0F1DAA */
            a1 = REC_BURST_PARAMS;                            /* 0x0F1DB2 lea (0x1D3C,PC) */
            if (ObjScript_CF4B8(m, obj, a1) < 0)              /* 0x0F1DB6 */
                return -1;
        }

        if ((int16_t)rd16(m, obj + 0x3Au) < -0x0020) {        /* 0x0F1DBC..0x0F1DC2 */
            Obj_StartDying(m, obj);                           /* 0x0CEE74 */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));           /* 0x0F1DC8 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));           /* 0x0F1DCE */
        Obj_StepScript(m, obj);                               /* 0x0F1DD4 jsr 0x0D0690 */
        Coll_AddEnemyHazard(m, obj);                          /* 0x0F1DDA jsr 0x0D0DC2 */
        Obj_HitFlash(m, obj);                                 /* 0x0F1DE0 jsr 0x0CBE34 */
        Obj_DrawSprite(m, obj);                               /* 0x0F1DE6 jsr 0x001282 */
        r->a[1] = rd32(m, F3_A5 - 0x0CA0u);                   /* A1 after 0x001282 */
        r->d[0] = 0x0000FFFFu;                                /* D0 after 0x001282 */
        r->d[1] = 0x0001FFFFu;                                /* D1 after 0x001282 */
        return 0;
    }

    /* 0x0F1DFA: off-screen / bomb-clear death. */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u); /* 0x0F1DFA..0x0F1E12 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {                            /* 0x0F1E16..0x0F1E18 */
            r->a[1] = child;                                  /* 0x0F1E1E movea.l D0,A1 */
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));     /* 0x0F1E20 */
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u)); /* 0x0F1E26 */
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));     /* 0x0F1E2C */
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u)); /* 0x0F1E32 */
            wr16(m, child + 0x58u, 0x00B4u);                  /* 0x0F1E38 */
            wr16(m, child + 0x60u, rd16(m, obj + 0x60u));     /* 0x0F1E3E */
        }
        /* 0x0F1E44 bra.l 0x0CEE48 */
        Obj_DieWithBonusChk(m, obj, child);                   /* 0x0CEE48 */
    }

    /* 0x0F1F5C: the gun releases its carrier and arms the burst state. */
    sub_0F1F5C(m, obj, &a1);
    r->a[1] = a1;
    return 0;
}
