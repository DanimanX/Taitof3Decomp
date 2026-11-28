#include "fn_auto.h"
/* ROM 0x17AC00: apply the scripted displacement to the parent while the bomb is active. */
#include "fn_call_frames.h"
#include "../objects/obj_shared.h"

static uint32_t swap_words(uint32_t value)
{
    return (value << 16) | (value >> 16);
}

/* ROM 0x1794A4: bounded signed jitter and the swapped 32-bit RNG update. */
static uint16_t script_jitter(const f3mem_t *m, f3regs_t *r, uint8_t range)
{
    uint32_t seed_addr = r->a[5] + 0x4B3Cu;
    uint32_t seed = rd32(m, seed_addr);
    uint16_t result = range;
    if (range) {
        unsigned index = 15u;
        for (unsigned value = range; value > 1u; value >>= 1)
            --index;
        uint16_t mask = rd16(m, 0x179530u + 2u * index);
        result = (uint16_t)(seed >> 16) & mask;
        if (range < result)
            result = (uint16_t)(result - range);
        if (seed & 0x01000000u)
            result = (uint16_t)-result;
    }
    wr32(m, seed_addr, swap_words(seed * 5u - 1u));
    return result;
}

/* ROM 0x178B7A: install the target point and advance past this record. */
static void set_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], entry = rd32(m, obj + 0x44u);
    r->a[1] = entry;
    uint16_t x = rd16(m, entry + 6u), y = rd16(m, entry + 8u);
    wr16(m, obj + 0x4Cu, x);
    wr16(m, obj + 0x4Eu, y);
    int16_t dx = (int16_t)(x - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(y - rd16(m, obj + 0x54u));
    int16_t count = (int16_t)rd16(m, obj + 0x48u);
    r->d[0] = (uint32_t)(int32_t)dx;
    r->d[1] = (uint32_t)(int32_t)dy;
    r->d[2] = set_word(r->d[2], (uint16_t)count);
    if (count > 0) {
        r->d[0] = ((uint32_t)(uint16_t)(dx % count) << 16) | (uint16_t)(dx / count);
        r->d[1] = ((uint32_t)(uint16_t)(dy % count) << 16) | (uint16_t)(dy / count);
        wr16(m, obj + 0x4Cu, (uint16_t)(x - r->d[0]));
        wr16(m, obj + 0x4Eu, (uint16_t)(y - r->d[1]));
    }
    wr16(m, obj + 0x48u, 0u);
}

static int16_t abs_word(int16_t word)
{
    return word < 0 ? (int16_t)-word : word;
}

/* ROM 0x179074: move toward the target or end the record at its threshold. */
static void move_toward_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t limit = (int8_t)rd8(m, obj + 0x42u);
    r->d[2] = (uint32_t)(int32_t)limit;
    if (limit < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t dx = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    r->d[0] = set_word(r->d[0], (uint16_t)dx);
    r->d[1] = set_word(r->d[1], (uint16_t)dy);
    int16_t distance = abs_word(dx);
    if (distance <= limit)
        distance = abs_word(dy);
    r->d[3] = set_word(r->d[3], (uint16_t)distance);
    if (distance <= limit) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint32_t sp = r->a[7];
    push_long(m, r, 0x1790B8u);
    push_long(m, r, obj);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    uint8_t angle = (uint8_t)Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy);
    r->a[7] = sp;
    uint32_t entry = rd32(m, obj + 0x44u);
    uint16_t accel = rd16(m, entry + 6u), mask = rd16(m, entry + 8u);
    uint16_t speed = (uint16_t)(rd16(m, obj + 0x42u) + accel);
    wr16(m, obj + 0x42u, speed);
    angle = (uint8_t)((angle & mask) + ((int16_t)mask >> 8));
    wr8(m, obj + 0x20u, angle);
    int16_t magnitude = (int16_t)(uint16_t)(speed + 0x20u) >> 6;
    r->d[2] = angle;
    r->a[1] = 0x027E40u;
    r->d[0] = (uint32_t)(magnitude * (int16_t)rd16(m, 0x027EC0u + 2u * angle));
    r->d[1] = (uint32_t)(magnitude * (int16_t)rd16(m, 0x027E40u + 2u * angle));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[0]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[1]);
}

/* ROM 0x178E00: accelerate, jitter the heading, and integrate X and Y. */
static void move_with_jitter(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], entry = rd32(m, obj + 0x44u);
    r->a[1] = entry;
    uint16_t speed = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, entry + 6u));
    r->d[1] = set_word(r->d[1], speed);
    uint8_t range = rd8(m, entry + 9u);
    r->d[0] = range;
    push_long(m, r, 0x178E16u);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    uint8_t angle = (uint8_t)(script_jitter(m, r, range) + rd8(m, entry + 8u));
    r->a[7] += 12u;
    wr8(m, obj + 0x20u, angle);
    wr16(m, obj + 0x42u, speed);
    int16_t magnitude = (int16_t)(uint16_t)(speed + 0x20u) >> 6;
    r->d[0] = angle;
    r->a[1] = 0x027E40u;
    r->d[1] = (uint32_t)(magnitude * (int16_t)rd16(m, 0x027EC0u + 2u * angle));
    r->d[2] = (uint32_t)(magnitude * (int16_t)rd16(m, 0x027E40u + 2u * angle));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[1]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[2]);
}

/* The script interpreter can encounter an unmapped pointer on an injected
 * call. For unmapped script reads the capture's bus supplies zero. */
static uint32_t script_read(const f3mem_t *m, uint32_t addr, unsigned size)
{
    uint32_t bus = addr & 0xFFFFFFu;
    if (bus < F3_ROM_SIZE ||
        (bus >= F3_WRAM_BASE && bus < F3_WRAM_BASE + 2u * F3_WRAM_SIZE) ||
        (m->spr && bus >= F3_SPR_BASE && bus < F3_SPR_BASE + F3_SPR_SIZE) ||
        (m->gfx && bus >= F3_GFX_BASE && bus < F3_GFX_BASE + F3_GFX_SIZE) ||
        (m->dpram && bus >= F3_DPRAM_BASE && bus < F3_DPRAM_BASE + F3_DPRAM_SIZE))
        return size == 2u ? rd16(m, addr) : rd32(m, addr);
    return 0u;
}

/* ROM 0x178982, with the three callbacks installed by 0x17ABDC. */
static void walk_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], entry;
    for (;;) {
        entry = rd32(m, obj + 0x44u);
        r->a[1] = entry;
        uint32_t fn = script_read(m, entry, 4u);
        if (fn == 0u)
            break;
        r->a[1] = fn;
        push_long(m, r, 0x178990u);
        switch (fn) {
        case 0x178B7Au: set_target(m, r); break;
        case 0x179074u: move_toward_target(m, r); break;
        case 0x178E00u: move_with_jitter(m, r); break;
        }
        r->a[7] += 4u;

        int16_t count = (int16_t)rd16(m, obj + 0x48u);
        if (count < 0)
            return;
        if (count > 0) {
            wr16(m, obj + 0x48u, (uint16_t)--count);
            if (count > 0)
                return;
            entry = rd32(m, obj + 0x44u) + 10u;
            if ((int32_t)script_read(m, entry, 4u) > 0)
                goto store;
            break;
        }
        entry = rd32(m, obj + 0x44u) + 10u;
        wr32(m, obj + 0x44u, entry);
        wr16(m, obj + 0x48u, (uint16_t)script_read(m, entry + 4u, 2u));
    }

    int16_t repeat = (int16_t)rd16(m, obj + 0x4Au);
    if (repeat >= 0) {
        if (repeat == 0) {
            repeat = (int16_t)script_read(m, entry + 4u, 2u);
            wr16(m, obj + 0x4Au, (uint16_t)repeat);
        }
        repeat = (int16_t)(repeat - 1);
        wr16(m, obj + 0x4Au, (uint16_t)repeat);
        if (repeat == 0) {
            entry += 10u;
            goto store;
        }
    }
    entry = script_read(m, entry + 6u, 4u);

store: /* 0x1789EE */
    r->a[1] = entry;
    wr32(m, obj + 0x44u, entry);
    wr16(m, obj + 0x48u, (uint16_t)script_read(m, entry + 4u, 2u));
}

int Fn_17AC00(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t script = rd32(m, obj + 0x44u);
    if ((script < 0x17AC56u || script > 0x17AC74u ||
         (script - 0x17AC56u) % 10u) && script_read(m, script, 4u) != 0u)
        return -1;

    uint32_t parent = rd32(m, obj + 8u);
    r->a[4] = parent;
    wr16(m, obj + 0x50u, rd16(m, parent + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, parent + 0x54u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    push_long(m, r, 0x17AC22u);
    walk_script(m, r);
    r->a[7] += 4u;

    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u)));
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u)));
    wr16(m, parent + 0x50u, (uint16_t)(rd16(m, parent + 0x50u) + r->d[0]));
    wr16(m, parent + 0x54u, (uint16_t)(rd16(m, parent + 0x54u) + r->d[1]));
    if (rd8(m, r->a[5] + 0x1D58u) & 2u) {
        wr32(m, obj, 0x17AC00u); /* 0x17AC40 branches to 0x17ABF8. */
    } else {
        wr8(m, parent + 0x23u, rd8(m, parent + 0x23u) & 0x7Fu);
        call_free(m, r, 0x17AC50u);
    }
    r->a[7] += 4u;
    return 0;
}
