/* ROM 0x004C42: copy the five backup high-score records into the active
 * table, recompute both checksums and write the settings block to EEPROM. */
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

    push_long(m, r, 0x004C74u);                 /* jsr 0x004DEE */
    if (settings_refresh_4DEE(m, r) < 0)
        return -1;                              /* 0x004E3C jsr 0x01014E */

    r->d[2] = rd32(m, r->a[7]);                   /* 0x004C74 move.l (SP)+,D2 */
    r->a[7] = sp + 4u;
    return 0;                                     /* 0x004C76 rts */
}
