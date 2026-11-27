#include "fn_auto.h"
/* ROM 0x1A006C: load 72 longwords of character data into graphics RAM. */

int Fn_1A006C(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1A006E: both 0x1DB0 extensions suppress the base register
     * and index their 32-bit displacements with D1.l scaled by four. */
    r->d[1] = 0;
    do {
        uint32_t offset = 4u * r->d[1];
        wr32(m, 0x61E200u + offset, rd32(m, 0x1A1C78u + offset));
        r->d[1]++;
        r->d[0] = 0x48u;
    } while (r->d[1] < r->d[0]);

    r->a[7] += 4u;
    return 0;
}
