#include "fn_auto.h"
/* ROM 0x1609EC: verified: the capture discriminates (an empty body fails it): Spawn a ground effect and play its sound when the scroll and object checks allow it. */
#include "fn_sound_frames.h"

int Fn_1609EC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], globals = r->a[5];
    int16_t scroll = (int16_t)rd16(m, globals - 0x5DA4u);
    if ((int16_t)rd16(m, globals - 0x5DA8u) >= 0xF00 || scroll >= 0x100)
        goto done; /* 0x1609F2 / 0x1609FC -> 0x160A5E */

    int32_t dy = (int16_t)r->d[0] - (int16_t)rd16(m, obj + 0x18u);
    r->d[0] = set_word(r->d[0], (uint16_t)dy);
    int16_t direction = (int16_t)r->d[1];
    /* 0x160A04 tests N xor V from SUB.W, so compare before truncating. */
    if ((dy < 0 ? direction < 0 : direction > 0) ||
        (int16_t)rd16(m, obj + 0x4Cu) < 0 || scroll < 0)
        goto done;

    push_long(m, r, r->d[0]); /* 0x160A28 MOVEM */
    call_spawn(m, r, 0x160A42u, 0x1113ACu, 0xBu, 1u, obj);
    call_hooked_sound(m, r, 0x24u, 0x160A50u, 0x160A56u);
    r->d[0] = rd32(m, r->a[7]); /* 0x160A5A MOVEM */
    r->a[7] += 4u;

done:
    r->a[7] += 4u;
    return 0;
}
