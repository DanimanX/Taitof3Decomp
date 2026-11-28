#include "eeprom_block.h"

/* ROM 0x0046C0: write and verify changed words from the block at 4(SP). */
int Fn_0046C0(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    push_long(m, r, r->d[2]);
    uint32_t block = rd32(m, r->a[7] + 12u);
    uint16_t sr = m->eeprom ? r->sr : 0x2000u;
    uint32_t result = 0;

    ee_xfer(m, 0x130u, 9, sr);                   /* EWEN */
    for (unsigned word = F3_EEP_WORDS; word-- > 0;) {
        uint32_t read_cmd = 0x80000180u | word;
        uint32_t previous = ee_xfer(m, read_cmd, 9, sr);
        uint16_t data = rd16(m, block + 2u * word);
        if (previous == data)
            continue;

        ee_xfer(m, 0x400001C0u | word, 9, sr);
        ee_xfer(m, 0x41400000u | (word << 16) | data, 25, sr);
        if (ee_xfer(m, read_cmd, 9, sr) != data) {
            result = 0xFFu;
            break;
        }
    }

    ee_xfer(m, 0x100u, 9, sr);                   /* EWDS */
    r->d[0] = r->a[0] = result;
    r->d[1] = 0xFFFFu;
    r->d[2] = rd32(m, r->a[7]);
    r->a[6] = rd32(m, r->a[7] + 4u);
    r->a[7] += 12u;
    return 0;
}
