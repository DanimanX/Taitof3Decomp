#include "fn_auto.h"
/* ROM 0x18F3C4: apply movement-script deltas to the parent during the bomb effect. */
#include "fn_boss_camera.h"

static int16_t abs_word(uint32_t value)
{
    int16_t word = (int16_t)value;
    return word < 0 ? (int16_t)(-(int32_t)word) : word;
}

/* ROM 0x178B7A, with the registers left by the target-point setup command. */
static void set_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t entry = r->a[1];
    uint16_t x = rd16(m, entry + 6u);
    uint16_t y = rd16(m, entry + 8u);
    wr16(m, obj + 0x4Cu, x);
    wr16(m, obj + 0x4Eu, y);
    int32_t dx = (int16_t)(x - rd16(m, obj + 0x50u));
    int32_t dy = (int16_t)(y - rd16(m, obj + 0x54u));
    r->d[0] = (uint32_t)dx;
    r->d[1] = (uint32_t)dy;
    int16_t duration = (int16_t)rd16(m, obj + 0x48u);
    r->d[2] = set_word(r->d[2], (uint16_t)duration);
    if (duration > 0) {
        r->d[0] = ((uint32_t)(uint16_t)(dx % duration) << 16) | (uint16_t)(dx / duration);
        r->d[1] = ((uint32_t)(uint16_t)(dy % duration) << 16) | (uint16_t)(dy / duration);
        wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - r->d[0]));
        wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, obj + 0x4Eu) - r->d[1]));
    }
    wr16(m, obj + 0x48u, 0u);
}

/* ROM 0x179074, with D2/D3 and the output registers of its movement step. */
static void approach_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int32_t radius = (int8_t)rd8(m, obj + 0x42u);
    r->d[2] = (uint32_t)radius;
    if (radius < 0) {
        wr16(m, obj + 0x42u, 0u);                                /* 0x179100 */
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u)));
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u)));
    int16_t distance = abs_word(r->d[0]);
    if (distance <= radius)
        distance = abs_word(r->d[1]);
    r->d[3] = set_word(r->d[3], (uint16_t)distance);
    if (distance <= radius) {
        wr16(m, obj + 0x48u, 0u);                                /* 0x179104 */
        return;
    }

    boss_angle(m, r, 0x1790B8u);
    uint32_t entry = rd32(m, obj + 0x44u);
    r->d[2] = set_word(r->d[2], rd16(m, entry + 6u));
    uint16_t mask = rd16(m, entry + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + r->d[2]));
    wr8(m, obj + 0x20u, (uint8_t)((r->d[0] & (uint8_t)mask) + (mask >> 8)));

    int16_t speed = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint8_t angle = rd8(m, obj + 0x20u);
    r->d[2] = angle;
    r->a[1] = 0x027E40u;
    uint32_t address = r->a[1] + 2u * angle;
    r->d[0] = (uint32_t)(speed * (int16_t)rd16(m, address + 0x80u));
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, address));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[0]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[1]);
}

/* ROM 0x178E00: advance speed and select a randomized heading. */
static void random_heading(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t entry = r->a[1];
    uint16_t speed_word = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, entry + 6u));
    r->d[1] = set_word(r->d[1], speed_word);
    r->d[0] = rd8(m, entry + 9u);                                 /* bfextu {8:8} */
    uint32_t sp = r->a[7];
    push_long(m, r, 0x178E16u);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    uint8_t angle = (uint8_t)(Obj_RandRange(m, (uint16_t)r->d[0]) + rd8(m, entry + 8u));
    r->a[7] = sp;
    wr8(m, obj + 0x20u, angle);
    wr16(m, obj + 0x42u, speed_word);
    int16_t speed = (int16_t)(uint16_t)(speed_word + 0x20u) >> 6;
    r->a[1] = 0x027E40u;
    r->d[0] = angle;
    uint32_t address = r->a[1] + 2u * angle;
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, address + 0x80u));
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, address));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[1]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[2]);
}

/* The entry initializer installs the four records at 0x17AC56. Check any
 * other entry's reachable immediate commands before modifying the object. */
static int supported_step(const f3mem_t *m, uint32_t entry, int16_t duration)
{
    for (;;) {
        switch (rd32(m, entry)) {
        case 0u:
            return 1;
        case 0x178E00u:
            if (duration != 0)
                return 1;
            break;
        case 0x178B7Au:
        case 0x179074u:
            break;
        default:
            return 0;
        }
        entry += 10u;
        duration = (int16_t)rd16(m, entry + 4u);
    }
}

/* ROM 0x178982, including the register effects of all commands from 0x17AC56. */
static void move_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    for (;;) {
        r->a[1] = rd32(m, obj + 0x44u);
        uint32_t command = rd32(m, r->a[1]);
        if (command == 0u)
            break;
        push_long(m, r, 0x178990u);
        switch (command) {
        case 0x178B7Au: set_target(m, r); break;
        case 0x179074u:
            r->a[1] = command;                                  /* indirect JSR */
            approach_target(m, r);
            break;
        case 0x178E00u: random_heading(m, r); break;
        }
        r->a[7] += 4u;

        int16_t duration = (int16_t)rd16(m, obj + 0x48u);
        if (duration < 0)
            return;
        if (duration > 0) {
            wr16(m, obj + 0x48u, (uint16_t)(duration - 1));
            if (duration > 1)
                return;
            r->a[1] = rd32(m, obj + 0x44u) + 10u;
            if ((int32_t)rd32(m, r->a[1]) > 0)
                goto store;
            break;
        }

        r->a[1] = rd32(m, obj + 0x44u) + 10u;
        wr32(m, obj + 0x44u, r->a[1]);
        wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
    }

    int16_t count = (int16_t)rd16(m, obj + 0x4Au);                /* 0x1789C8 */
    if (count >= 0) {
        if (count == 0) {
            count = (int16_t)rd16(m, r->a[1] + 4u);
            wr16(m, obj + 0x4Au, (uint16_t)count);
        }
        wr16(m, obj + 0x4Au, (uint16_t)((uint16_t)count - 1u));
        if (count == 1) {
            r->a[1] += 10u;
            goto store;
        }
    }
    r->a[1] = rd32(m, r->a[1] + 6u);

store:
    wr32(m, obj + 0x44u, r->a[1]);
    wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
}

int Fn_18F3C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t entry = rd32(m, obj + 0x44u);
    if (!supported_step(m, entry, (int16_t)rd16(m, obj + 0x48u)))
        return -1;
    r->a[4] = rd32(m, obj + 8u);
    uint32_t parent = r->a[4];
    wr16(m, obj + 0x50u, rd16(m, parent + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, parent + 0x54u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    push_long(m, r, 0x18F3E6u);
    move_script(m, r);
    r->a[7] += 4u;
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
    r->d[0] = set_word(r->d[0], dx);
    r->d[1] = set_word(r->d[1], dy);
    wr16(m, parent + 0x50u, (uint16_t)(rd16(m, parent + 0x50u) + dx));
    wr16(m, parent + 0x54u, (uint16_t)(rd16(m, parent + 0x54u) + dy));
    if (rd8(m, r->a[5] + 0x1D58u) & 2u) {
        wr32(m, obj, 0x18F3C4u);                                 /* 0x18F3BC */
    } else {
        wr8(m, parent + 0x23u, (uint8_t)(rd8(m, parent + 0x23u) & 0x7Fu));
        if (rd8(m, r->a[5] + 0x4B3Bu) & 0x10u) {                 /* bitfield {11:1} */
            r->a[1] = 0x18F484u;
            wr32(m, obj + 0x44u, r->a[1]);
            wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr32(m, obj, 0x18F432u);
        } else {
            call_free(m, r, 0x18F47Eu);
        }
    }
    r->a[7] += 4u;
    return 0;
}
