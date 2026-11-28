/* ROM 0x004D06: reload the EEPROM settings, publish the RAM globals,
 * rescale the timing table and back up the high-score records. */
#include "fn_auto.h"
#include "eeprom_block.h"

int Fn_004D06(const f3mem_t *m, f3regs_t *r)
{
    settings_apply_4D06(m, r);      /* 0x004D06..0x004DEC, see eeprom_block.h */
    return 0;
}
