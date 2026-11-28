/* ROM 0x005080: UNVERIFIED (weak capture): reload the whole configuration -
 * identification bytes, settings fields, defaults, score table - then write the
 * block back to the serial EEPROM. Six calls, no arguments, no return value:
 *   0x004E46 install the identification bytes and re-checksum
 *   0x004E84 copy the default settings fields and re-checksum
 *   0x004F58 restore the remaining settings fields and re-checksum
 *   0x005030 install the default score table
 *   0x004DEE write both checksums and the block back through 0x0046C0
 *   0x004D06 read the block back and publish it into the RAM globals
 *
 * Only one captured pair exists, and it can never be bit-exact: 0x004DEE and
 * 0x004D06 clock the serial device through 0x0047C0, which saves the CPU status
 * register into 0x406C50. The capture shows 0x2710 there (supervisor, IPL 7,
 * X set), a state the harness does not model (no SR in f3regs_t); the shared
 * helper stores 0x2000. Everything else in the pair - the settings block, the
 * score table, the address scalars at 0x409D3A/3C and the registers - follows
 * from the bytes.
 *
 * Every path is translated; the EEPROM write-failure path (0x004E1C..0x004E3C:
 * clear the layers, draw the reset message, enter 0x01014E) returns -1 because
 * 0x01014E spins at 0x010170 forever. Nothing before it can fail in the
 * harness (the port write is discarded), so no side effect is lost. */
#include "fn_auto.h"
#include "eeprom_block.h"

int Fn_005080(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x005084u);                 /* 0x005080 jsr 0x004E46 */
    Fn_004E46(m, r);
    push_long(m, r, 0x005088u);                 /* 0x005084 jsr 0x004E84 */
    Fn_004E84(m, r);
    push_long(m, r, 0x00508Cu);                 /* 0x005088 jsr 0x004F58 */
    Fn_004F58(m, r);
    push_long(m, r, 0x005090u);                 /* 0x00508C jsr 0x005030 */
    Fn_005030(m, r);
    push_long(m, r, 0x005094u);                 /* 0x005090 jsr 0x004DEE */
    if (settings_refresh_4DEE(m, r) < 0)
        return -1;                              /* 0x004E3C jsr 0x01014E */
    push_long(m, r, 0x005098u);                 /* 0x005094 jsr 0x004D06 */
    Fn_004D06(m, r);
    r->a[7] += 4u;                              /* 0x005098 rts */
    return 0;
}
