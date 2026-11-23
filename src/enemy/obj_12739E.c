#include "obj_12739E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_11B09A.h"

/* ROM 0x12739E: idle/shoot cycle of a rider. It walks the {dur.w, sprdef.l}
 * list at +0xC; a zero-duration entry (or the +0x34 flag already set) fires a
 * fan of 0x0CCD04 bullets when the 0x0CC2BC roll lands, a positive one just
 * draws, and running off the list resets the walk.
 *
 * Raw ROM (work/maincpu.bin), PC-relative operands use base = insn + 2:
 *   0x12739E 2268 0008      movea.l (0x8,A0),A1
 *   0x1273A2 4A29 0059      tst.b (0x59,A1)
 *   0x1273A6 6600 01AA      bne.w 0x127552
 *   0x1273AA E8ED 0441 30EE bftst (0x30EE,A5){0x11:1} ; bne 0x1273C4
 *   0x1273B4 4A28 0034      tst.b (0x34,A0) ; bne 0x1273C4
 *   0x1273BC 42A8 0010      clr.l (0x10,A0) ; bra 0x12751A
 *   0x1273C4 2268 000C      movea.l (0xC,A0),A1
 *   0x1273C8 4EB9 000CBD24  jsr 0x0CBD24
 *   0x1273CE 4A71 0000      tst.w (0x0,A1,D0w) ; bgt 0x12751A / blt 0x12750E
 *   ...        zero: st +0x34, 0x0CC2BC #0x10, then nine 0x0CCD04 spawns
 *   0x1274FC bftst {0x11:1} ; bne 0x12751A ; addq.w #2,+0x3A
 *   0x12750E sf +0x34 ; clr.w +0x3A ; clr.w +0x3E
 *   0x12751A 2F28 0010      move.l (0x10,A0),-(SP)
 *   0x12751E 2028 0010      move.l (0x10,A0),D0
 *   0x127522 43FA 0B74      lea (0xB74,PC),A1 -> 0x127524 + 0xB74 = 0x128098
 *   0x127528 31F1 0600 0050 move.w (0x0,A1,D0w*0x8),(0x50,A0)
 *   0x12752E 31F1 0602 0052 move.w (0x2,A1,D0w*0x8),(0x52,A0)
 *   0x127534 2171 0604 0010 move.l (0x4,A1,D0w*0x8),(0x10,A0)
 *   0x12753A 2268 0008      movea.l (0x8,A0),A1
 *   0x12753E 1169 001D 001D move.b (0x1D,A1),(0x1D,A0)
 *   0x127544 4EB9 0011B09A  jsr 0x11B09A
 *   0x12754A 209F          move.l (SP)+,(0x10,A0)
 *   0x12754E 6000 FE46      bra.w 0x127396  (move.l #0x12739E,(A0) ; rts)
 *   0x127552 2F08 / 4EB9 000105EC / 4FEF 0004 / 4E75   Obj_Free, rts
 *
 * 0x11B09A is translated; the bullet spawns are not, so the routine returns -1.
 */
int Obj_12739E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t anim = rd32(m, obj + 0x0Cu);
    int16_t dur = 0;
    int shot = 0;

    /* 0x12739E: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x127552 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x59u) != 0u) {
            Obj_Free(m, obj);                          /* 0x127554 */
            return 0;
        }
    }

    /* 0x1273AA: bftst (0x30EE,A5){0x11:1} ; bne 0x1273C4 */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0004u) == 0u &&
        rd8(m, obj + 0x34u) == 0u) {
        /* 0x1273BC: clr.l (0x10,A0) ; bra 0x12751A */
        wr32(m, obj + 0x10u, 0u);
        goto tail_12751A;
    }

    /* 0x1273C8: 0x0CBD24 with A1 = the +0xC list; then the table probe the
     * helper leaves in D0w picks the offset back from +0x3A. */
    Obj_AnimFrameStep(m, obj, anim);
    dur = (int16_t)rd16(m, anim + (uint32_t)(uint16_t)rd16(m, obj + 0x3Au));
    if (dur > 0)
        goto tail_12751A;                              /* 0x1273D2 */
    if (dur < 0) {
        /* 0x12750E: sf (0x34,A0) ; clr.w (0x3A,A0) ; clr.w (0x3E,A0) */
        wr8(m, obj + 0x34u, 0u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        goto tail_12751A;
    }

    /* 0x1273DA: st (0x34,A0) */
    wr8(m, obj + 0x34u, 0xFFu);

    /* 0x1274FC: bftst (0x30EE,A5){0x11:1} ; bne 0x12751A */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0004u) != 0u)
        goto tail_12751A;
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u)); /* 0x127506 */

    shot = 1;   /* 0x1273E2: 0x0CC2BC #0x10 gate + nine 0x0CCD04 spawns */

tail_12751A:
    /* 0x12751A..0x127534: park +0x10, index the 0x128098 {x.w, y.w, rec.l}
     * list with the current sprite pointer, then restore it after the draw.
     * lea (0xB74,PC) at 0x127522 -> 0x128098. */
    {
        uint32_t saved = rd32(m, obj + 0x10u);
        uint32_t a1 = 0x00128098u;
        uint32_t idx = (uint32_t)(uint16_t)saved;      /* D0w */
        wr16(m, obj + 0x50u, rd16(m, a1 + 8u * idx));
        wr16(m, obj + 0x52u, rd16(m, a1 + 8u * idx + 2u));
        wr32(m, obj + 0x10u, rd32(m, a1 + 8u * idx + 4u));

        /* 0x12753A: colour copy from the parent */
        wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du));

        /* 0x127544: 0x11B09A -- integrate, draw, un-integrate */
        Obj_11B09A_Tick(m, obj);

        wr32(m, obj + 0x10u, saved);                   /* 0x12754A */
    }

    /* 0x12754E: bra.w 0x127396 (move.l #0x12739E,(A0) ; rts) */
    wr32(m, obj + 0x00u, 0x0012739Eu);

    return shot ? -1 : 0;   /* the bullet fan is not translated yet */
}
