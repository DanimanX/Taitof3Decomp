#include "fn_auto.h"
/* ROM 0x198CEC: UNVERIFIED (weak capture): Run a scripted flight burst and draw a randomized directional trail along it. */
#include "batch_obj_regs.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x006422 preserves D1-D3/A0. The EXG at 0x006474 can
 * replace the high half of D0 even though the return angle is a byte. */
static void flight_angle(const f3mem_t *m, f3regs_t *r, uint32_t ret, uint32_t sp)
{
    wr32(m, sp - 4u, ret);
    wr32(m, sp - 8u, r->a[0]);
    wr32(m, sp - 12u, r->d[3]);
    wr32(m, sp - 16u, r->d[2]);
    wr32(m, sp - 20u, r->d[1]);
    uint16_t x = (uint16_t)r->d[0];
    uint16_t y = (uint16_t)r->d[1];
    uint32_t high = ((int16_t)x < 0) != ((int16_t)y < 0) ? r->d[1] : r->d[0];
    r->d[0] = batch_set_word(high, Obj_Atan2(m, x, y));
}

/* ROM 0x178FCE and 0x17910A share target checks and movement. The former
 * turns toward the target by a fixed step. The latter turns with RNG jitter. */
static void steer(const f3mem_t *m, f3regs_t *r, uint32_t script, int jitter)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7] - 8u;
    int8_t radius = (int8_t)rd8(m, obj + 0x42u);
    r->d[2] = (uint32_t)(int32_t)radius;
    if (radius < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    uint16_t x = (uint16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    uint16_t y = (uint16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    r->d[0] = batch_set_word(r->d[0], x);
    r->d[1] = batch_set_word(r->d[1], y);
    int16_t abs_axis = (int16_t)x;
    if (abs_axis < 0)
        abs_axis = (int16_t)-abs_axis;
    if (abs_axis <= radius) {
        abs_axis = (int16_t)y;
        if (abs_axis < 0)
            abs_axis = (int16_t)-abs_axis;
        if (abs_axis <= radius) {
            r->d[3] = batch_set_word(r->d[3], (uint16_t)abs_axis);
            wr16(m, obj + 0x48u, 0u);
            return;
        }
    }
    r->d[3] = batch_set_word(r->d[3], (uint16_t)abs_axis);
    flight_angle(m, r, jitter ? 0x17914Eu : 0x179012u, sp);
    if (jitter)
        wr8(m, obj + 0x20u, (uint8_t)r->d[0]);

    r->a[1] = script;
    uint16_t accel = rd16(m, script + 6u);
    r->d[2] = batch_set_word(r->d[2], accel);
    uint16_t turn;
    if (jitter) {
        wr32(m, sp - 4u, 0x179164u);
        wr32(m, sp - 8u, r->d[2]);
        wr32(m, sp - 12u, r->d[1]);
        turn = Obj_RandRange(m, rd8(m, script + 9u));
    } else {
        int8_t diff = (int8_t)(r->d[0] - rd8(m, obj + 0x20u));
        turn = rd16(m, script + 8u);
        if (diff < 0)
            turn = (uint16_t)(0u - turn);
        if (diff == 0)
            turn = 0;
    }

    uint16_t speed = (uint16_t)(rd16(m, obj + 0x42u) + accel);
    wr16(m, obj + 0x42u, speed);
    if (jitter)
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + turn));
    else
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + turn));

    int16_t velocity = (int16_t)(speed + 0x20u) >> 6;
    uint16_t heading = rd8(m, obj + 0x20u);
    r->a[1] = 0x027E40u;
    r->d[2] = batch_set_word(r->d[2], heading);
    r->d[0] = (uint32_t)((int32_t)velocity * (int16_t)rd16(m, 0x027EC0u + heading * 2u));
    r->d[1] = (uint32_t)((int32_t)velocity * (int16_t)rd16(m, 0x027E40u + heading * 2u));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[0]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[1]);
}

/* ROM 0x178982 for the five records at 0x198DA2 and the terminal record
 * at 0x198DD4. The latter loads -1 into +0x44 via 0x1789EA. */
static void flight_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t script = rd32(m, obj + 0x44u);
    wr32(m, r->a[7] - 4u, 0x198D4Eu);
    for (;;) {
        uint32_t command = rd32(m, script);
        r->a[1] = script;
        if (command != 0) {
            r->a[1] = command;
            wr32(m, r->a[7] - 8u, 0x178990u);
            if (command == 0x178B0Eu) {
                r->a[1] = script;
                wr16(m, obj + 0x42u, rd16(m, script + 6u));
                wr16(m, obj + 0x20u, rd16(m, script + 8u));
            } else {
                steer(m, r, script, command == 0x17910Au);
            }

            int16_t time = (int16_t)rd16(m, obj + 0x48u);
            if (time < 0)
                return;
            if (time == 0) {
                script = rd32(m, obj + 0x44u) + 10u;
                wr32(m, obj + 0x44u, script);
                wr16(m, obj + 0x48u, rd16(m, script + 4u));
                continue;
            }

            wr16(m, obj + 0x48u, (uint16_t)(time - 1));
            if (time > 1)
                return;
            script = rd32(m, obj + 0x44u) + 10u;
            if ((int32_t)rd32(m, script) > 0)
                break;
        }

        int16_t loops = (int16_t)rd16(m, obj + 0x4Au);
        if (loops >= 0) {
            if (loops == 0) {
                loops = (int16_t)rd16(m, script + 4u);
                wr16(m, obj + 0x4Au, (uint16_t)loops);
            }
            loops = (int16_t)(loops - 1);
            wr16(m, obj + 0x4Au, (uint16_t)loops);
            if (loops == 0) {
                script += 10u;
                break;
            }
        }
        script = rd32(m, script + 6u);
        break;
    }

    r->a[1] = script;
    wr32(m, obj + 0x44u, script);
    wr16(m, obj + 0x48u, rd16(m, script + 4u));
}

int Fn_198CEC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    if (rd16(m, a5 + 0x4B38u) == 0xFFFFu) {
        wr16(m, obj + 6u, 2u); /* 0x17A84C makes the next BEQ false. */
        wr32(m, obj, 0x17A85Cu);
        return 0;
    }
    if ((rd8(m, a5 + 0x4B3Au) & 1u) == 0) {
        wr32(m, obj, 0x198CECu);
        return 0;
    }

    /* The initializer at 0x198CD8 sets index 9. The four lists at 0x198DDE
     * contain 16 six-byte records for its 0x179898 tick. */
    uint16_t anim = rd16(m, obj + 0x40u);
    if (anim != 9u)
        return -1;
    r->a[3] = rd32(m, a5 + 0x4BAAu);
    wr16(m, obj + 0x50u, rd16(m, r->a[3] + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, r->a[3] + 0x54u));
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + 0x1Eu));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + 2u));
    r->a[3] = rd32(m, a5 + 0x4BAEu);
    wr16(m, obj + 0x4Cu, rd16(m, r->a[3] + 0x50u));
    wr16(m, obj + 0x4Eu, rd16(m, r->a[3] + 0x54u));
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) + 0x0Cu));
    wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, obj + 0x4Eu) + 2u));
    r->a[1] = 0x198DA2u;
    wr32(m, obj + 0x44u, r->a[1]);
    wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
    wr16(m, obj + 0x4Au, 0u);

    do {
        flight_script(m, r);
        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
        wr32(m, sp - 4u, 0x198D62u);
        wr32(m, sp - 8u, r->d[2]);
        wr32(m, sp - 12u, r->d[1]);
        uint16_t variant = Obj_RandRangeU(m, 3u);
        wr32(m, obj + 0x3Au, rd32(m, 0x198DDEu + variant * 4u));
        wr32(m, sp - 4u, 0x198D7Au);
        Obj_AnimTick(m, obj, anim);
        r->a[1] = rd32(m, obj + 0x10u);
        r->d[1] = batch_set_word(r->d[1], (uint16_t)(rd16(m, r->a[1]) << 3));
        r->d[0] = (uint16_t)(rd16(m, r->a[1] + 2u) << 3);
        r->a[1] += 4u;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - r->d[0]));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - r->d[1]));
        batch_draw_sprite(m, r, 0x198D94u);
    } while (rd32(m, obj + 0x44u) != UINT32_MAX);

    wr32(m, obj, 0x198CECu);
    return 0;
}
