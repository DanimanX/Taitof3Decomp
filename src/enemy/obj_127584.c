#include "obj_127584.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_11B09A.h"

/* ROM 0x127584: three-attack cycle. A captured colour in +0x35 selects the
 * attack script (+0x36) and animation list (+0xC) of the 0x12, 0x13 or 0x14
 * slot; the attack routine is then called 0, 1 or 2 times while the +0xC list
 * scrolls. When the list ends the +0x34 lock is set, sound 0x1B plays and the
 * state stays until the gate clears.
 *
 * Raw ROM (work/maincpu.bin), PC-relative operands use base = insn + 2:
 *   0x127584 2268 0008      movea.l (0x8,A0),A1
 *   0x127588 4A29 0059      tst.b (0x59,A1) ; bne 0x1276B0
 *   0x127590 4A28 0034      tst.b (0x34,A0) ; bne 0x127614
 *   0x127598 4A28 0035      tst.b (0x35,A0) ; bge 0x127614
 *   0x1275A0 E8ED 0441 30EE bftst {0x12:1} ; beq 0x1275C4
 *   0x1275AA 11FC 0012 0035 / 217C 001276BE 0036 / 21FC 00128100 000C
 *   0x1275C4 E8ED 0441 30EE bftst {0x13:1} ; beq 0x1275E8
 *   0x1275CE ... +0x35=0x13, +0x36=0x127724, +0xC=0x128134
 *   0x1275E8 bftst {0x14:1} ; beq 0x127614
 *   0x1275F2 ... +0x35=0x14, +0x36=0x1277CC, +0xC=0x128168,
 *                  clr.w +0x20, clr.w +0x6
 *   0x127614 7000 / 1028 0035 moveq #0,D0 ; move.b (0x35,A0),D0b
 *   0x12761A E8ED 0441 30EE bftst {D0:1} ; bne 0x12763C
 *   0x127624 4A28 0034      tst.b (0x34,A0) ; bne 0x12763C
 *   0x12762C 50E8 0035 st   (0x35,A0) ; 21FC 00028740 0010 ; bra 0x12769C
 *   0x12763C 2268 000C      movea.l (0xC,A0),A1 ; jsr 0x0CBD24
 *   0x127646 4A71 0000      tst.w (0x0,A1,D0w) ; bgt 0x12769C / blt 0x127690
 *   0x127652 4A28 0034      tst.b (0x34,A0) ; bne 0x127672
 *   0x12765A 50E8 0034 st   (0x34,A0) ; 3F3C 001B ; jsr 0x13F238 ;
 *                          jsr 0x0039D8 ; 4FEF 0002
 *   0x127672 2268 0036      movea.l (0x36,A0),A1 ; jsr (A1)
 *   0x127678 7000 / 1028 0035 / bftst {D0:1} ; bne 0x12769C
 *   0x127688 5468 003A      addq.w #2,(0x3A,A0) ; bra 0x12769C
 *   0x127690 sf +0x34 ; clr.w +0x3A ; clr.w +0x3E
 *   0x12769C 2268 0008      movea.l (0x8,A0),A1 ; move.b (0x1D,A1),(0x1D,A0)
 *   0x1276A6 4EB9 0011B09A  jsr 0x11B09A
 *   0x1276AC 6000 FECE      bra.w 0x12757C (move.l #0x127584,(A0) ; rts)
 *   0x1276B0 2F08 / 4EB9 000105EC / 4FEF 0004 / 4E75   Obj_Free, rts
 *
 * The three attack scripts (0x1276BE, 0x127724, 0x1277CC) are not translated
 * yet, so the routine returns -1 once one of them would run.
 */
static int tail_12769C(const f3mem_t *m, uint32_t obj)
{
    /* 0x12769C: movea.l (0x8,A0),A1 ; move.b (0x1D,A1),(0x1D,A0) */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du));
    /* 0x1276A6: 0x11B09A -- integrate, draw, un-integrate */
    Obj_11B09A_Tick(m, obj);
    /* 0x1276AC: bra.w 0x12757C (move.l #0x127584,(A0) ; rts) */
    wr32(m, obj + 0x00u, 0x00127584u);
    return 0;
}

int Obj_127584_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t anim;
    uint16_t sel;

    /* 0x127584: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x1276B0 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x59u) != 0u) {
            Obj_Free(m, obj);                          /* 0x1276B2 */
            return 0;
        }
    }

    /* 0x127590: tst.b (0x34,A0) ; bne 0x127614 */
    if (rd8(m, obj + 0x34u) == 0u) {
        /* 0x127598: tst.b (0x35,A0) ; bge 0x127614 (only a negative byte
         * picks an attack) */
        if ((int8_t)rd8(m, obj + 0x35u) < 0) {
            /* 0x1275A0 / 0x1275C4 / 0x1275E8: the 0x12/0x13/0x14 gates */
            if ((rd16(m, F3_A5 + 0x30EEu) & 0x0020u) != 0u) {
                wr8(m, obj + 0x35u, 0x12u);            /* 0x1275AA */
                wr32(m, obj + 0x36u, 0x001276BEu);     /* 0x1275B0 */
                wr32(m, obj + 0x0Cu, 0x00128100u);     /* 0x1275B8 */
            } else if ((rd16(m, F3_A5 + 0x30EEu) & 0x0010u) != 0u) {
                wr8(m, obj + 0x35u, 0x13u);            /* 0x1275CE */
                wr32(m, obj + 0x36u, 0x00127724u);     /* 0x1275D4 */
                wr32(m, obj + 0x0Cu, 0x00128134u);     /* 0x1275DC */
            } else if ((rd16(m, F3_A5 + 0x30EEu) & 0x0008u) != 0u) {
                wr8(m, obj + 0x35u, 0x14u);            /* 0x1275F2 */
                wr32(m, obj + 0x36u, 0x001277CCu);     /* 0x1275F8 */
                wr32(m, obj + 0x0Cu, 0x00128168u);     /* 0x127600 */
                wr16(m, obj + 0x20u, 0u);              /* 0x127608 */
                wr16(m, obj + 0x06u, 0u);              /* 0x12760C */
            }
        }
    }

    /* 0x127614: moveq #0,D0 ; move.b (0x35,A0),D0b */
    sel = (uint16_t)rd8(m, obj + 0x35u);

    /* 0x12761A: bftst (0x30EE,A5){D0:1} ; bne 0x12763C */
    if (((rd16(m, F3_A5 + 0x30EEu) >> (15u - (sel & 15u))) & 1u) == 0u &&
        rd8(m, obj + 0x34u) == 0u) {
        /* 0x12762C: st (0x35,A0) ; move.l #0x28740,(0x10,A0) ; bra 0x12769C */
        wr8(m, obj + 0x35u, 0xFFu);
        wr32(m, obj + 0x10u, 0x00028740u);
        return tail_12769C(m, obj);
    }

    /* 0x12763C: 0x0CBD24 with A1 = the +0xC list, then the entry probe */
    anim = rd32(m, obj + 0x0Cu);
    Obj_AnimFrameStep(m, obj, anim);
    {
        int16_t dur = (int16_t)rd16(m, anim + (uint32_t)(uint16_t)rd16(m, obj + 0x3Au));
        if (dur > 0)
            return tail_12769C(m, obj);                /* 0x12764A */
        if (dur < 0) {
            /* 0x127690: sf (0x34,A0) ; clr.w (0x3A,A0) ; clr.w (0x3E,A0) */
            wr8(m, obj + 0x34u, 0u);
            wr16(m, obj + 0x3Au, 0u);
            wr16(m, obj + 0x3Eu, 0u);
            return tail_12769C(m, obj);
        }
    }

    /* 0x127652: tst.b (0x34,A0) ; bne 0x127672 */
    if (rd8(m, obj + 0x34u) == 0u) {
        wr8(m, obj + 0x34u, 0xFFu);                    /* 0x12765A */
    }

    /* 0x127672: movea.l (0x36,A0),A1 ; jsr (A1) -- the attack routine. The
     * sound 0x1B it opens with (0x13F238 -> 0x0039D8) is not replayed here
     * because the attack itself is not translated. None of the three scripts
     * (0x1276BE, 0x127724, 0x1277CC) are translated yet. */
    return -1;
}
