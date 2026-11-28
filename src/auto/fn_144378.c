#include "fn_auto.h"
#include "fn_call_frames.h"
#include "fn_helpers.h"
#include "../enemy/obj_1194C2.h"

/* ROM 0x144378: test-mode selftest row editor tick (no arguments).
 *
 * Reads the direction bits at A5-0x5DDC and runs the bounded menu adjuster
 * 0x19FFF0 over the repeat counter at 0x40CBD0 with the range [0..2] and a
 * step of 1. The result moves the cursor at A5+0x3320 by one 8-byte row: down
 * when it is above 1 and the next row is live, up when it is below 1 and not
 * already at the top row 0x40C2B4. The button latches at A5-0x5DDB /
 * A5-0x5DD7 gate the row-insert (0x144720) and row-delete (0x14477C) helpers,
 * and the two labels at ROM 0x144428 / 0x14443B are then drawn at 0x61CC88
 * and 0x61CD08 with attribute 7. D0 = end of the second string. */

/* ROM 0x144720: open a gap at the cursor by shifting the rows above it one
 * 8-byte slot up; the new row becomes {0, 0xA0, 0x80, 0, 0}. */
static uint32_t sub_144720(const f3mem_t *m)
{
    uint32_t start = rd32(m, F3_A5 + 0x3320u);
    uint32_t a0 = start;
    while ((int16_t)rd16(m, a0 + 0x06u) >= 0)          /* 0x144724 tst.w (6,A0) */
        a0 += 8u;
    a0 += 8u;                                          /* 0x144732 */
    if ((int32_t)a0 >= (int32_t)0x0040CAACu)           /* 0x144736 */
        return a0;
    for (;;) {                                         /* 0x144740 */
        wr16(m, a0, rd16(m, a0 - 8u));
        wr16(m, a0 + 2u, rd16(m, a0 - 6u));
        wr16(m, a0 + 4u, rd16(m, a0 - 4u));
        wr8(m, a0 + 6u, rd8(m, a0 - 2u));
        wr8(m, a0 + 7u, rd8(m, a0 - 1u));
        a0 -= 8u;
        if ((int32_t)a0 <= (int32_t)rd32(m, F3_A5 + 0x3320u))  /* 0x144760 bgt */
            break;
    }
    wr16(m, a0 + 2u, 0x00A0u);                         /* 0x144766 */
    wr16(m, a0 + 4u, 0x0080u);                         /* 0x14476C */
    wr8(m, a0 + 6u, 0u);                               /* 0x144772 sf */
    wr8(m, a0 + 7u, 0u);                               /* 0x144776 sf */
    return a0;
}

/* ROM 0x14477C: close the row at the cursor by shifting the rows above it
 * one 8-byte slot down. */
uint32_t sub_14477C(const f3mem_t *m)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x3320u);
    for (;;) {                                         /* 0x144780 */
        if ((int16_t)rd16(m, a0 + 0x06u) < 0)
            break;
        wr16(m, a0, rd16(m, a0 + 8u));
        wr16(m, a0 + 2u, rd16(m, a0 + 0x0Au));
        wr16(m, a0 + 4u, rd16(m, a0 + 0x0Cu));
        wr8(m, a0 + 6u, rd8(m, a0 + 0x0Eu));
        wr8(m, a0 + 7u, rd8(m, a0 + 0x0Fu));
        a0 += 8u;
    }
    return a0;
}

int Fn_144378(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = set_word(r->d[0], 0x0001u);                        /* 0x144378 */
    push_long(m, r, r->a[0]);                                    /* 0x14437C movem {A0} */
    push_word(m, r, 1u);                                         /* 0x144380 */
    push_word(m, r, 2u);                                         /* 0x144384 */
    push_word(m, r, 0u);                                         /* 0x144388 */
    push_word(m, r, (uint16_t)r->d[0]);                          /* 0x14438C D0w = 1 */
    r->d[0] = set_word(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, F3_A5 - 0x5DDCu));
    push_word(m, r, (uint16_t)r->d[0]);                          /* 0x144394 */
    push_long(m, r, 0x0014439Cu);
    Fn_19FFF0(m, r);                                             /* 0x144396 */
    r->a[7] += 10u;                                              /* 0x14439C lea 0xa(SP) */
    r->a[0] = rd32(m, r->a[7]);                                  /* 0x1443A0 movem (SP)+,{A0} */
    r->a[7] += 4u;

    uint32_t a0 = rd32(m, F3_A5 + 0x3320u);                      /* 0x1443A4 */
    int16_t d0w = (int16_t)(uint16_t)r->d[0];                    /* 0x1443A8 */
    if (d0w == 1) {
        /* 0x1443D2: cursor unchanged */
    } else if (d0w > 1) {
        if ((int16_t)rd16(m, a0 + 0x06u) >= 0)                   /* 0x1443C6 tst.w (6,A0) */
            a0 += 8u;                                            /* 0x1443CE */
    } else {
        if ((int32_t)a0 > (int32_t)0x0040C2B4u)                  /* 0x1443B4 cmpa.l */
            a0 -= 8u;                                            /* 0x1443BE */
    }
    wr32(m, F3_A5 + 0x3320u, a0);                                /* 0x1443D2 */

    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x80u))                      /* 0x1443D6 btst #7 */
        a0 = sub_144720(m);                                      /* 0x1443E0 */
    if (!(rd8(m, F3_A5 - 0x5DD7u) & 0x80u))                      /* 0x1443E4 btst #7 */
        a0 = sub_14477C(m);                                      /* 0x1443EE */

    Text_DrawString(m, 0x00144428u, 0x0061CC88u, 7u);            /* 0x1443F2 */
    r->d[0] = Text_DrawString(m, 0x0014443Bu, 0x0061CD08u, 7u);  /* 0x14440C */
    r->a[0] = a0;
    return 0;
}
