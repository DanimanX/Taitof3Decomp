/* ROM 0x00509A: read the EEPROM settings, restore the five ROM high-score
 * records and write the checksummed block back to the device. */
#include "fn_auto.h"
#include "eeprom_block.h"

int Fn_00509A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    push_long(m, r, r->d[2]);                     /* 0x00509A move.l D2,-(SP) */
    push_long(m, r, EE_SETTINGS);                 /* 0x00509C pea.l 0x406c54 */
    push_long(m, r, 0x0050A8u);                   /* 0x0050A2 jsr 0x00466E */
    Fn_00466E(m, r);
    r->a[7] += 4u;                                /* 0x0050A8 addq.l #4,SP */

    /* 0x0050AA..0x0050D2: five 12-byte records 0x1A1BF8.. -> 0x406C74.. */
    for (int32_t d2 = 0; d2 < 5; ++d2) {
        uint32_t d1 = (uint32_t)d2 * 12u;         /* 0x0050AC..0x0050B2 */
        uint32_t a0 = d1 + 0x00406C74u;           /* 0x0050B6 adda.l */
        uint32_t a1 = d1 + 0x001A1BF8u;           /* 0x0050BE addi.l */
        for (unsigned i = 0; i < 3; ++i) {        /* 0x0050C6..0x0050CA */
            wr32(m, a0, rd32(m, a1));
            a0 += 4u;
            a1 += 4u;
        }
    }

    push_long(m, r, 0x0050D8u);                 /* jsr 0x004DEE */
    if (settings_refresh_4DEE(m, r) < 0)
        return -1;                              /* 0x004E3C jsr 0x01014E */

    r->d[2] = rd32(m, r->a[7]);                   /* 0x0050D8 move.l (SP)+,D2 */
    r->a[7] = sp + 4u;
    return 0;                                     /* 0x0050DA rts */
}
