#include "fn_auto.h"
/* ROM 0x148B4C: fill line RAM and set the playfield priorities by scanline. */
#include "fn_call_frames.h"

/* 0x17669E writes the pair for each row before advancing D7. */
static void fill_pair(const f3mem_t *m, f3regs_t *r)
{
    do {
        uint32_t offset = (uint32_t)((int32_t)(int16_t)r->d[7] * 2);
        wr16(m, r->a[0] + offset, (uint16_t)r->d[0]);
        wr16(m, r->a[1] + offset, (uint16_t)r->d[1]);
        r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] + 1u));
    } while ((int16_t)r->d[7] <= (int16_t)r->d[6]);
    r->a[7] += 4u;
}

/* 0x17668A inserts the low nibble of D0 into the word loaded into D1. */
static void fill_priority(const f3mem_t *m, f3regs_t *r)
{
    do {
        uint32_t addr = r->a[0] + (uint32_t)((int32_t)(int16_t)r->d[7] * 2);
        uint16_t word = (uint16_t)((rd16(m, addr) & 0xFFF0u) | (r->d[0] & 0xFu));
        r->d[1] = set_word(r->d[1], word);
        wr16(m, addr, word);
        r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] + 1u));
    } while ((int16_t)r->d[7] <= (int16_t)r->d[6]);
    r->a[7] += 4u;
}

int Fn_148B4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t offset_addr = r->a[5] - 0x5D56u;
    push_long(m, r, r->a[1]);                         /* 0x148B4C MOVEM */
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    push_long(m, r, r->d[6]);
    r->a[0] = 0x627600u + (uint32_t)(int32_t)(int16_t)rd16(m, offset_addr);
    r->a[1] = 0x626000u + (uint32_t)(int32_t)(int16_t)rd16(m, offset_addr);
    push_word(m, r, 0u);
    push_word(m, r, 0xE7u);
    push_word(m, r, 0x2FFu);
    r->d[0] = set_word(r->d[0], 0xECCCu);
    r->d[1] = set_word(r->d[1], 0x2FFu);
    r->d[6] = set_word(r->d[6], 0xE7u);
    r->d[7] = set_word(r->d[7], 0u);
    r->a[7] += 6u;
    push_long(m, r, 0x148B80u);
    fill_pair(m, r);
    r->d[6] = rd32(m, sp - 16u);
    r->d[7] = rd32(m, sp - 12u);
    r->a[0] = rd32(m, sp - 8u);
    r->a[1] = rd32(m, sp - 4u);
    r->a[7] = sp;

    static const struct {
        uint32_t base, ret;
        uint16_t first, last, priority;
    } fills[] = {
        { 0x62B400u, 0x148BA8u, 0x00u, 0x72u, 0x8u },
        { 0x62B400u, 0x148BD0u, 0x73u, 0xE7u, 0xAu },
        { 0x62B000u, 0x148BF8u, 0x00u, 0xE7u, 0xDu },
        { 0x62B600u, 0x148C20u, 0x00u, 0xE7u, 0x6u },
    };
    for (unsigned i = 0; i < sizeof fills / sizeof fills[0]; ++i) {
        push_long(m, r, r->a[0]);                    /* 0x148B84 + i*$28 */
        push_long(m, r, r->d[7]);
        push_long(m, r, r->d[6]);
        r->a[0] = fills[i].base + (uint32_t)(int32_t)(int16_t)rd16(m, offset_addr);
        push_word(m, r, fills[i].first);
        push_word(m, r, fills[i].last);
        r->d[0] = set_word(r->d[0], fills[i].priority);
        r->d[6] = set_word(r->d[6], fills[i].last);
        r->d[7] = set_word(r->d[7], fills[i].first);
        r->a[7] += 4u;
        push_long(m, r, fills[i].ret);
        fill_priority(m, r);
        r->d[6] = rd32(m, sp - 12u);
        r->d[7] = rd32(m, sp - 8u);
        r->a[0] = rd32(m, sp - 4u);
        r->a[7] = sp;
    }
    r->a[7] += 4u;                                   /* 0x148C24 RTS */
    return 0;
}
