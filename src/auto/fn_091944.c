#include "fn_auto.h"
/* ROM 0x091944: (re)start the sprite/colour debug overlay tick. Clears the
 * object's +0x1E word, invalidates the current record pointer A5-0xC58
 * (0x4073A8) and the frame counter A5-0xC54 (0x4073AC), then scans the 14
 * 0x0C-byte records at 0x41A260 for the first one whose leading word is
 * positive (recording it in A5-0xC58); a negative word stops the scan and a
 * run of blanks falls out of the DBF. Finally installs 0x091984 (0x091984
 * takes over on the next frame). */

#define REC_BASE 0x0041A260u
#define CURSOR   (F3_A5 - 0x0C58u)   /* 0x4073A8 current record */
#define COUNTER  (F3_A5 - 0x0C54u)   /* 0x4073AC frame counter */

int Fn_091944(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d0 = 0x000Du;                                   /* 0x091950 move.w #0xD (clears D0 high) */
    uint32_t a1 = REC_BASE;                                  /* 0x09194A lea */

    wr16(m, obj + 0x1Eu, 0u);                                /* 0x091944 move.w #0,(0x1E,A0) */
    wr32(m, CURSOR, 0xFFFFFFFFu);                            /* 0x091954 move.l #-1 */
    wr16(m, COUNTER, 0u);                                    /* 0x09195C move.w #0 */

    for (;;) {                                               /* 0x091962 */
        int16_t w = (int16_t)rd16(m, a1);
        if (w > 0) {                                         /* 0x091964 bgt 0x091978 */
            wr32(m, CURSOR, a1);                             /* 0x091978 */
            break;
        }
        if (w < 0)                                           /* 0x091968 blt 0x09197C */
            break;
        a1 += 0x0Cu;                                         /* 0x09196C lea (0xC,A1),A1 */
        d0 = (uint16_t)(d0 - 1u);                            /* 0x091970 dbf D0w */
        if (d0 == 0xFFFFu)
            break;                                           /* 0x091974 bra 0x09197C */
    }

    wr32(m, obj, 0x00091984u);                               /* 0x09197C move.l #0x91984,(A0) */
    r->d[0] = d0;                                            /* 0x091950 move.w #0xD clears the high half; dbf leaves D0.w */
    r->a[1] = a1;                                            /* 0x09196C/0x091978 cursor */
    return 0;                                                /* 0x091982 rts */
}
