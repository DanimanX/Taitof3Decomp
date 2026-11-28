/* ROM 0x1A0942: the "SOUND TEST" entry of the Task_TestModeMenu table at
 * 0x1A2350, in segments (src/kernel/seg.h). It is a wrapper with no yield of
 * its own:
 *
 *   0x1A0942  jsr 0x1A006C                 load the character block
 *   0x1A0946  move.b 0x402220,D0 / andi.b #2,D0 / beq 0x1A0958
 *   0x1A0952  jsr 0x1A00E6                 bit 1 set: the editor screen
 *   0x1A0956  bra.s 0x1A095C / 0x1A095C rts
 *   0x1A0958  jsr 0x1A043C                 bit 1 clear: the other screen
 *   0x1A095C  rts
 *
 * Both callees are segment groups that yield inside, so the jsr cannot be a
 * plain call: the wrapper pushes the return PC and lets Seg_Run run the
 * callee's group. When that screen returns through its own RTS the popped PC
 * (0x1A0956 or 0x1A095C) comes back here and the wrapper's RTS returns to the
 * menu, which resumes at 0x1A1B9A. */
#include "../kernel/seg.h"
#include "../auto/fn_auto.h"        /* Fn_1A006C (ROM 0x1A006C) */

#define P_CABINET 0x00402220u       /* cabinet switch byte */

uint32_t Seg_1A0942(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A0942u:
        r->a[7] -= 4u;                      /* 0x1A0942 jsr 0x1A006C */
        wr32(m, r->a[7], 0x001A0946u);
        Fn_1A006C(m, r);                    /* pops its return PC */
        /* 0x1A0946: bit 1 of the cabinet switch byte picks the screen */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P_CABINET) & 0x02u);
        if (r->d[0] & 0xFFu) {              /* 0x1A0950 beq */
            r->a[7] -= 4u;                  /* 0x1A0952 jsr 0x1A00E6 */
            wr32(m, r->a[7], 0x001A0956u);
            return Seg_Run(m, r, 0x001A00E6u);
        }
        r->a[7] -= 4u;                      /* 0x1A0958 jsr 0x1A043C */
        wr32(m, r->a[7], 0x001A095Cu);
        return Seg_Run(m, r, 0x001A043Cu);

    case 0x1A0956u:                         /* bra.s 0x1A095C */
    case 0x1A095Cu:                         /* rts */
        return Seg_Run(m, r, seg_pop32(m, r));
    }
    return SEG_LEAVE;
}
