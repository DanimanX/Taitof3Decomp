#include "fn_18BF46.h"
#include "../objects/obj_anim.h"

/* ROM 0x18BF46: per-object state step.
 * 0x18BF46-0x18BF5E: a3 = (A0+0xC), a1 = (a3+4); A5+0x4B7C is a bit field
 * read with bftst {3:1} (bit 12 of the word, mask 0x1000) and {2:1}
 * (mask 0x2000). Bit 12 -> body 0x18BF6E, else bit 13 -> 0x18BF98, else
 * 0x18BF62: tst.w A0+0x3E ; blt 0x18BFC2 clears it, otherwise rts.
 * Body 0x18BF6E (bit 12): d0 = (a1+0x1E); if d0 != A0+0x10 nothing happens;
 * otherwise a1 = (a1+0x22), d0 = (a1)+, A0+0x40 = d0, A0+0x3A = a1 and the
 * INIT half of 0x179A5C[d0] runs with A0. Body 0x18BF98 (bit 13) is the same
 * with a1+0x26 and the TICK half (0x179A5E): both halves arrive with the same
 * A0, so they share Obj_AnimInit. */
/* 0x18BF7A / 0x18BFA4: latch the animation head and run its 0x179A5C entry. */
static int anim_head_body(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    wr16(m, obj + 0x40u, d0);                   /* move.w d0,$40(a0) */
    wr32(m, obj + 0x3Au, a1 + 2u);              /* move.l a1,$3a(a0) */
    return Obj_AnimInit(m, obj, d0);            /* (0x179A5C)[d0], jsr (a1) */
}

int Obj_18BF46_Step(const f3mem_t *m, uint32_t obj)
{
    uint16_t bits = rd16(m, F3_A5 + 0x4B7Cu);
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    uint32_t a1 = rd32(m, a3 + 0x04u);
    if (bits & 0x1000u) {                       /* bftst {3:1} -> 0x18BF6E */
        if (rd32(m, a1 + 0x1Eu) != rd32(m, obj + 0x10u))
            return 0;                           /* bne 0x18BFCA */
        return anim_head_body(m, obj, rd32(m, a1 + 0x22u));
    }
    if (bits & 0x2000u) {                       /* bftst {2:1} -> 0x18BF98 */
        if (rd32(m, a1 + 0x1Eu) != rd32(m, obj + 0x10u))
            return 0;
        return anim_head_body(m, obj, rd32(m, a1 + 0x26u));
    }
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0)      /* tst.w / blt 0x18BFC2 */
        wr16(m, obj + 0x3Eu, 0u);
    return 0;
}
