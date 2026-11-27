#include "fn_auto.h"
/* ROM 0x004C12: copy five 12-byte records from the active table into the backup table. */
#include "fn_call_frames.h"

int Fn_004C12(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[2]);
    /* 0x004C16-0x004C3C: each record is copied by three postincrement moves. */
    for (uint32_t i = 0; i < 5u; ++i) {
        r->d[1] = i * 12u;
        r->a[0] = 0x406CD4u + r->d[1];
        r->a[1] = 0x406C74u + r->d[1];
        for (unsigned word = 0; word < 3u; ++word) {
            wr32(m, r->a[0], rd32(m, r->a[1]));
            r->a[0] += 4u;
            r->a[1] += 4u;
        }
    }
    r->d[0] = 0; /* 0x004C38: final record count minus five. */
    r->d[2] = rd32(m, r->a[7]);
    r->a[7] += 8u; /* 0x004C3E: restore D2, then RTS. */
    return 0;
}
