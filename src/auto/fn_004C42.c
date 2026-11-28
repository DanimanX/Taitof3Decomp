/* ROM 0x004C42: UNVERIFIED (no usable capture): restore the default high-score
 * table and write the settings block back to the serial device.
 * Copies the five 12-byte default records at 0x406CD4.. to 0x406C74.., then
 * (0x004DEE) recomputes the 0x20-byte settings checksum at 0x406C54 and the
 * 0x60-byte score-table checksum at 0x406C74 through 0x004BD2 and writes the
 * whole 0x80-byte settings block back through 0x0046C0. A non-zero result from
 * the write means the device did not keep the data: the ROM then clears all
 * layers, prints "EEP-ROM ERROR" (0x1A1C36 -> 0x61C81A) and enters the reset
 * loop at 0x01014E, which never returns - that path ends with -1 here.
 * The capture (30 pairs) shows the success path only: D0 = 0, D1 = 0xFFFF,
 * A0 = 0, A1 = 0x406C74, scratch/SR word 0x406C50 = 0x2010. The 0x46C0
 * read-back verify needs the serial device's replies, which the harness cannot
 * provide (it reads zeroed unmapped I/O), so the routine always takes the
 * write-failed path there; the test line is therefore not registered. */
#include "fn_auto.h"
#include "eeprom_block.h"

int Fn_004C42(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    push_long(m, r, r->d[2]);                     /* 0x004C42 move.l D2,-(SP) */

    /* 0x004C44..0x004C6C: five 12-byte records 0x406CD4.. -> 0x406C74.. */
    for (int32_t d2 = 0; d2 < 5; ++d2) {
        uint32_t d1 = (uint32_t)d2 * 12u;         /* 0x004C46..0x004C4C */
        uint32_t a0 = d1 + 0x00406C74u;           /* 0x004C50 adda.l */
        uint32_t a1 = d1 + 0x00406CD4u;           /* 0x004C58 addi.l */
        for (unsigned i = 0; i < 3; ++i) {        /* 0x004C60..0x004C64 */
            wr32(m, a0, rd32(m, a1));
            a0 += 4u;
            a1 += 4u;
        }
    }

    push_long(m, r, r->a[2]);                     /* 0x004DF0 move.l A2,-(SP) */
    r->a[2] = EE_SETTINGS;                        /* 0x004DF0 movea.l #0x406c54 */
    int rc = settings_refresh_4DEE(m, r);         /* 0x004C6E jsr 0x004DEE */
    if (rc < 0)
        return -1;                                /* 0x004E3C jsr 0x01014E */

    r->a[2] = rd32(m, r->a[7]);                   /* 0x004E42 movea.l (SP)+,A2 */
    r->a[7] += 4u;
    r->d[2] = rd32(m, r->a[7]);                   /* 0x004C74 move.l (SP)+,D2 */
    r->a[7] = sp + 4u;
    return 0;                                     /* 0x004C76 rts */
}
