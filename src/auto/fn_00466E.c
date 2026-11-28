#include "eeprom_block.h"

/* ROM 0x00466E: read 64 EEPROM words into the block at 4(SP). */
int Fn_00466E(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    push_long(m, r, r->d[2]);
    uint32_t block = rd32(m, r->a[7] + 12u);
    uint16_t sr = m->eeprom ? r->sr : 0x2000u;

    ee_xfer(m, 0x130u, 9, sr);                   /* EWEN */
    for (unsigned word = F3_EEP_WORDS; word-- > 0;) {
        uint32_t data = ee_xfer(m, 0x80000180u | word, 9, sr);
        wr16(m, block + 2u * word, (uint16_t)data);
    }
    r->d[0] = ee_xfer(m, 0x100u, 9, sr);         /* EWDS, D0 = 0x400 */
    r->d[1] = 0xFFFFu;
    r->a[0] = block;
    r->d[2] = rd32(m, r->a[7]);
    r->a[6] = rd32(m, r->a[7] + 4u);
    r->a[7] += 12u;
    return 0;
}
