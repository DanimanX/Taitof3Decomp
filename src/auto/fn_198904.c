#include "fn_auto.h"
/* ROM 0x198904: verified: the capture discriminates (an empty body fails it): relay part damage, step its movement script, and update boss-relative position. */
#include "../objects/obj_script10.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

static int16_t arithmetic_shift(uint16_t word, unsigned shift)
{
    uint16_t high = (word & 0x8000u) ? (uint16_t)(0xFFFFu << (16 - shift)) : 0;
    return (int16_t)((word >> shift) | high);
}

/* ROM 0x17B92C: scale the damage by the hit type and apply it to the parent. */
static void relay_damage(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t hit = rd8(m, obj + 0x35);
    if (hit == 0)
        return;

    r->d[0] = rd32(m, obj + 0x24);
    uint32_t multiplier = 0;
    if (hit == 2) {
        multiplier = 0x4B2E;
    } else {
        r->a[1] = rd32(m, obj + 0x36);
        uint8_t phase = rd8(m, r->a[1] + 0x22);
        if (phase == 1 || phase == 2)
            multiplier = 0x4B2A + 2u * (phase - 1u);
    }
    if (multiplier != 0) {
        r->d[1] = rd16(m, r->a[5] + multiplier);
        r->d[0] = (uint32_t)((int32_t)r->d[0] / 100) * r->d[1];
    }

    r->a[4] = rd32(m, obj + 0x08); /* 0x17B9A0 */
    wr32(m, r->a[4] + 0x24, rd32(m, r->a[4] + 0x24) + r->d[0]);
    wr32(m, obj + 0x24, 0);
    wr32(m, r->a[4] + 0x36, rd32(m, obj + 0x36));
    wr16(m, r->a[4] + 0x34, rd16(m, obj + 0x34));
}

static void push_return(const f3mem_t *m, f3regs_t *r, uint32_t addr)
{
    r->a[7] -= 4;
    wr32(m, r->a[7], addr);
}

/* ROM 0x178C22 and 0x1790D6: scale the speed and integrate 16.16 X and Y. */
static void step_position(const f3mem_t *m, f3regs_t *r, uint16_t speed,
                          uint16_t heading)
{
    uint32_t obj = r->a[0];
    int16_t step = arithmetic_shift((uint16_t)(speed + 0x20u), 6);
    uint16_t index = heading >> 8;
    r->d[2] = with_word(r->d[2], index);
    r->a[1] = 0x27E40;
    r->d[0] = (uint32_t)(step * (int16_t)rd16(m, r->a[1] + 0x80 + 2u * index));
    r->d[1] = (uint32_t)(step * (int16_t)rd16(m, r->a[1] + 2u * index));
    wr32(m, obj + 0x50, rd32(m, obj + 0x50) + r->d[0]);
    wr32(m, obj + 0x54, rd32(m, obj + 0x54) + r->d[1]);
}

/* ROM 0x179074: steer toward the target until within the speed threshold. */
static void steer_to_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->d[2] = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x42);
    if ((int32_t)r->d[2] < 0) {
        wr16(m, obj + 0x42, 0);
        wr16(m, obj + 0x48, 0);
        return;
    }

    uint16_t x = (uint16_t)(rd16(m, obj + 0x4C) - rd16(m, obj + 0x50));
    uint16_t y = (uint16_t)(rd16(m, obj + 0x4E) - rd16(m, obj + 0x54));
    r->d[0] = with_word(r->d[0], x);
    r->d[1] = with_word(r->d[1], y);
    uint16_t distance = (int16_t)x < 0 ? (uint16_t)(0u - x) : x;
    if ((int16_t)distance <= (int16_t)r->d[2]) {
        distance = (int16_t)y < 0 ? (uint16_t)(0u - y) : y;
        if ((int16_t)distance <= (int16_t)r->d[2]) {
            r->d[3] = with_word(r->d[3], distance);
            wr16(m, obj + 0x48, 0);
            return;
        }
    }
    r->d[3] = with_word(r->d[3], distance);
    push_return(m, r, 0x1790B8);
    FnAuto_Angle(m, r);
    r->a[7] += 4;
    r->a[1] = rd32(m, obj + 0x44);
    r->d[2] = with_word(r->d[2], rd16(m, r->a[1] + 0x06));
    r->d[1] = with_word(r->d[1], rd16(m, r->a[1] + 0x08));
    wr16(m, obj + 0x42, (uint16_t)(rd16(m, obj + 0x42) + r->d[2]));
    uint8_t angle = (uint8_t)((r->d[0] & r->d[1]) + arithmetic_shift((uint16_t)r->d[1], 8));
    wr8(m, obj + 0x20, angle);
    step_position(m, r, rd16(m, obj + 0x42), rd16(m, obj + 0x20));
}

/* ROM 0x178B7A: record the target, optionally backing up one divided step. */
static void set_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 0x44);
    uint16_t x = rd16(m, r->a[1] + 0x06);
    uint16_t y = rd16(m, r->a[1] + 0x08);
    wr16(m, obj + 0x4C, x);
    wr16(m, obj + 0x4E, y);
    int32_t dx = (int16_t)(uint16_t)(x - rd16(m, obj + 0x50));
    int32_t dy = (int16_t)(uint16_t)(y - rd16(m, obj + 0x54));
    r->d[0] = (uint32_t)dx;
    r->d[1] = (uint32_t)dy;
    int16_t count = (int16_t)rd16(m, obj + 0x48);
    r->d[2] = with_word(r->d[2], (uint16_t)count);
    if (count > 0) {
        r->d[0] = ((uint32_t)(uint16_t)(dx % count) << 16) | (uint16_t)(dx / count);
        r->d[1] = ((uint32_t)(uint16_t)(dy % count) << 16) | (uint16_t)(dy / count);
        wr16(m, obj + 0x4C, (uint16_t)(rd16(m, obj + 0x4C) - r->d[0]));
        wr16(m, obj + 0x4E, (uint16_t)(rd16(m, obj + 0x4E) - r->d[1]));
    }
    wr16(m, obj + 0x48, 0);
}

static void store_script(const f3mem_t *m, uint32_t obj, uint32_t script)
{
    wr32(m, obj + 0x44, script);
    wr16(m, obj + 0x48, rd16(m, script + 4));
}

/* ROM 0x198A0A..0x198C61 contains all 60 records for this part's two descriptors. */
static void script_command(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    switch (r->a[1]) {
    case 0x178B02:
        r->a[1] = rd32(m, obj + 0x44);
        wr16(m, obj + 0x20, rd16(m, r->a[1] + 8));
        break;
    case 0x178B20:
        r->a[1] = rd32(m, obj + 0x44);
        Obj_Script10_SetHeading(m, obj);
        break;
    case 0x178B7A:
        set_target(m, r);
        break;
    case 0x178C06: {
        r->a[1] = rd32(m, obj + 0x44);
        uint16_t speed = (uint16_t)(rd16(m, obj + 0x42) + rd16(m, r->a[1] + 6));
        uint16_t heading = (uint16_t)(rd16(m, obj + 0x20) + rd16(m, r->a[1] + 8));
        wr16(m, obj + 0x42, speed);
        wr16(m, obj + 0x20, heading);
        step_position(m, r, speed, heading);
        break;
    }
    case 0x179074:
        steer_to_target(m, r);
        break;
    case 0x17921A: /* Bare RTS. */
        break;
    case 0x198C62: {
        r->a[1] = rd32(m, obj + 0x0C);
        uint16_t index = rd16(m, r->a[5] + 0x4BB2) & 3u;
        r->d[0] = with_word(r->d[0], index);
        r->a[1] = rd32(m, r->a[1] + 8u + 4u * index);
        store_script(m, obj, r->a[1]);
        wr16(m, obj + 0x4A, 0);
        break;
    }
    case 0x198C84:
        r->a[4] = rd32(m, obj + 8);
        wr16(m, obj + 0x4C, rd16(m, r->a[4] + 0x50));
        wr16(m, obj + 0x4E, rd16(m, r->a[4] + 0x54));
        r->a[1] = rd32(m, obj + 0x44);
        r->d[0] = with_word(r->d[0], rd16(m, r->a[1] + 6));
        r->d[1] = with_word(r->d[1], rd16(m, r->a[1] + 8));
        wr16(m, obj + 0x4C, (uint16_t)(rd16(m, obj + 0x4C) + r->d[0]));
        wr16(m, obj + 0x4E, (uint16_t)(rd16(m, obj + 0x4E) + r->d[1]));
        break;
    case 0x198CAA:
        r->a[4] = rd32(m, obj + 8);
        wr16(m, obj + 0x4C, rd16(m, r->a[4] + 0x50));
        wr16(m, obj + 0x4E, rd16(m, r->a[4] + 0x54));
        push_return(m, r, 0x198CC0);
        steer_to_target(m, r);
        r->a[7] += 4;
        if (rd16(m, obj + 0x48) == 0) {
            wr16(m, obj + 0x16, 0);
            wr16(m, obj + 0x1A, 0);
        }
        break;
    }
}

/* ROM 0x178982: run the active record and follow the 10-byte script's loop markers. */
static void script_tick(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    for (;;) {
        r->a[1] = rd32(m, obj + 0x44);
        uint32_t command = rd32(m, r->a[1]);
        if (command == 0)
            break;
        r->a[1] = command;
        push_return(m, r, 0x178990);
        script_command(m, r);
        r->a[7] += 4;

        int16_t count = (int16_t)rd16(m, obj + 0x48);
        if (count < 0)
            return;
        if (count > 0) {
            wr16(m, obj + 0x48, (uint16_t)(count - 1));
            if (count > 1)
                return;
            r->a[1] = rd32(m, obj + 0x44) + 10u;
            if ((int32_t)rd32(m, r->a[1]) > 0) {
                store_script(m, obj, r->a[1]);
                return;
            }
            break;
        }
        r->a[1] = rd32(m, obj + 0x44) + 10u;
        store_script(m, obj, r->a[1]);
    }

    /* 0x1789C8: negative loops jump immediately, zero initializes the counter. */
    uint16_t loop = rd16(m, obj + 0x4A);
    if ((int16_t)loop >= 0) {
        if (loop == 0) {
            loop = rd16(m, r->a[1] + 4);
            wr16(m, obj + 0x4A, loop);
        }
        loop = (uint16_t)(loop - 1u);
        wr16(m, obj + 0x4A, loop);
        if (loop == 0) {
            r->a[1] += 10;
            store_script(m, obj, r->a[1]);
            return;
        }
    }
    r->a[1] = rd32(m, r->a[1] + 6);
    store_script(m, obj, r->a[1]);
}

int Fn_198904(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (rd16(m, r->a[5] + 0x4B38) == 0xFFFF) {
        /* 0x17A84C: MOVE.W #2 clears Z and makes the free branch unreachable. */
        wr16(m, obj + 0x06, 2);
        wr32(m, obj, 0x17A85C);
        return 0;
    }

    uint32_t descriptor = rd32(m, obj + 0x0C);
    uint32_t script = rd32(m, obj + 0x44);
    int alternate = (rd8(m, r->a[5] + 0x4B3A) & 2u) != 0;
    if (alternate || (int32_t)script >= 0) {
        /* 0x1981AE and 0x1981D0 supply the two six-pointer descriptors. */
        if (descriptor != 0x1989DA && descriptor != 0x1989F2)
            return -1;
        uint32_t entry = script;
        if (alternate && (int32_t)script <= 0)
            entry = rd32(m, descriptor + 4);
        else if (!alternate && script < rd32(m, descriptor))
            entry = rd32(m, descriptor);
        if (entry < 0x198A0A || entry > 0x198C58 || (entry - 0x198A0A) % 10u != 0)
            return -1;
    }

    push_return(m, r, 0x198916);
    relay_damage(m, r);
    r->a[7] += 4;
    r->a[1] = rd32(m, obj + 0x0C);
    if (alternate) {
        if ((int32_t)rd32(m, obj + 0x44) <= 0) {
            r->a[1] = rd32(m, r->a[1] + 4);
            store_script(m, obj, r->a[1]);
            wr16(m, obj + 0x4A, 0);
        }
    } else if ((int32_t)rd32(m, obj + 0x44) >= 0) {
        r->d[0] = rd32(m, obj + 0x44);
        if ((int32_t)r->d[0] < (int32_t)rd32(m, r->a[1])) {
            r->a[1] = rd32(m, r->a[1]);
            store_script(m, obj, r->a[1]);
            wr16(m, obj + 0x4A, 0);
        }
    } else {
        goto position;
    }

    wr16(m, obj + 0x64, rd16(m, obj + 0x50)); /* 0x198964 */
    wr16(m, obj + 0x68, rd16(m, obj + 0x54));
    push_return(m, r, 0x198976);
    script_tick(m, r);
    r->a[7] += 4;
    r->d[0] = with_word(r->d[0], (uint16_t)(rd16(m, obj + 0x50) - rd16(m, obj + 0x64)));
    r->d[1] = with_word(r->d[1], (uint16_t)(rd16(m, obj + 0x54) - rd16(m, obj + 0x68)));
    wr16(m, obj + 0x16, (uint16_t)(rd16(m, obj + 0x16) + r->d[0]));
    wr16(m, obj + 0x1A, (uint16_t)(rd16(m, obj + 0x1A) + r->d[1]));

position: /* 0x19898E */
    r->a[4] = rd32(m, obj + 0x08);
    uint16_t x = (uint16_t)(rd16(m, r->a[4] + 0x50) + rd16(m, obj + 0x16));
    uint16_t y = (uint16_t)(rd16(m, r->a[4] + 0x54) + rd16(m, obj + 0x1A));
    wr16(m, obj + 0x50, x);
    wr16(m, obj + 0x54, y);
    wr16(m, obj + 0x14, x);
    wr16(m, obj + 0x18, y);
    r->d[0] = rd8(m, obj + 0x22);
    uint32_t addr = r->a[5] + 0x4BA8 + (r->d[0] >> 3);
    uint8_t bit = 0x80u >> (r->d[0] & 7u);
    wr8(m, addr, rd8(m, addr) | bit);
    r->d[1] = with_word(r->d[1], rd16(m, obj + 0x16) | rd16(m, obj + 0x1A));
    if ((uint16_t)r->d[1] == 0)
        wr8(m, addr, rd8(m, addr) & (uint8_t)~bit);
    wr8(m, obj + 0x35, 0);
    wr32(m, obj, 0x198904); /* 0x1988FC */
    return 0;
}
