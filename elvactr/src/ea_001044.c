/* ROM 0x001044: call routine 0x001338 and return */
#include "ea_auto.h"

int Ea_001044(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x001048u); /* 0x001044 jsr $1338.w */
    return Ea_001338(m, r);      /* 0x001048 rts */
}
