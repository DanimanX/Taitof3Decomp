#include "fn_auto.h"
/* ROM 0x137460: follow the boss, spawn periodic explosions and clear its line effects on expiry. */
#include "fn_call_frames.h"
#include "../objects/obj_shared_ext.h"

static uint32_t random_value(const f3mem_t *m, f3regs_t *r, uint32_t return_pc)
{
    push_long(m, r, return_pc);
    uint32_t value = Rng_NextMem(m); /* 0x001D8A */
    r->a[7] += 4u;
    return value;
}

int Fn_137460(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], a5 = r->a[5];
    wr32(m, a5 + 0x32FAu, rd32(m, a5 + 0x30D2u));
    wr16(m, a5 + 0x32FAu, (uint16_t)(rd16(m, a5 + 0x32FAu) + 0x78u));
    wr32(m, a5 + 0x32FCu, rd32(m, a5 + 0x30D6u));
    wr16(m, a5 + 0x32FCu, (uint16_t)(rd16(m, a5 + 0x32FCu) + 0x10u));
    wr16(m, obj + 0x14u, rd16(m, a5 + 0x32FAu));
    wr16(m, obj + 0x18u, rd16(m, a5 + 0x32FCu));

    r->d[0] = set_word(r->d[0], rd16(m, obj + 6u) & 3u);
    if ((uint16_t)r->d[0] == 0u) {
        /* 0x1374A0/0x1374B2: BFINS counts from the most significant bit. */
        r->d[1] = 3u;
        r->d[0] = random_value(m, r, 0x137498u);
        r->d[0] = set_word(r->d[0], (uint16_t)((r->d[0] & 0x7Fu) - 0x64u));
        r->d[1] |= (uint32_t)(uint8_t)r->d[0] << 24;
        r->d[0] = random_value(m, r, 0x1374AAu);
        r->d[0] = set_word(r->d[0], (uint16_t)((r->d[0] & 0x3Fu) - 0x20u));
        r->d[1] |= (uint32_t)(uint8_t)r->d[0] << 16;
        call_spawn(m, r, 0x1374CCu, 0x13E4EEu, 5u, 2u, r->d[1]);
        r->d[0] = set_word(r->d[0], rd16(m, obj + 6u) & 7u);
        if ((uint16_t)r->d[0] == 0u) {
            r->d[1] = set_word(r->d[1], 0x0600u);
            call_spawn(m, r, 0x1374F6u, 0x13EE08u, 5u, 2u, r->d[1]);
        }
    }

    uint16_t old_timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(old_timer - 1u));
    if ((int16_t)old_timer > 1) {
        wr32(m, obj, 0x137460u);
    } else {
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[7]);
        uint32_t line = 0x627400u + (uint32_t)(int32_t)(int16_t)rd16(m, a5 - 0x5D56u);
        if (m->gfx)
            for (unsigned i = 0; i < 0xE8u; ++i, line += 2u)
                wr16(m, line, rd16(m, line) & 0x0FFFu);
        r->a[7] += 8u;
        wr8(m, obj + 0x59u, 0xFFu);

        /* 0x11AE2C is a tail jump and reuses the caller's return address. */
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[7]);
        r->a[7] += 8u;
        push_long(m, r, r->a[0]);
        push_long(m, r, 0x11AE5Cu);
        push_long(m, r, r->a[1]);
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[0]);
        Obj_11AE2C(m, obj);
        r->a[7] += 20u;
    }
    r->a[7] += 4u;
    return 0;
}
