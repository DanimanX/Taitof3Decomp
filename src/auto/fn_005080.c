/* ROM 0x005080: rebuild the defaults, write them to the serial EEPROM,
 * then reload the settings into the runtime globals. */
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
