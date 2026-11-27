#include "fn_auto.h"
/* ROM 0x19B65A: verified: the capture discriminates (an empty body fails it): Wait for a boss trigger, then move and intermittently draw a part along its approach script. */
#include "fn_sprite_regs.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared_ext.h"

/* 0x179074: move toward the target while the velocity and distances allow it. */
static void approach(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t limit = (int8_t)rd8(m, obj + 0x42u);
    r->d[2] = (uint32_t)(int32_t)limit;
    if (limit < 0) {
        wr16(m, obj + 0x42u, 0); /* 0x179100 */
        wr16(m, obj + 0x48u, 0);
        return;
    }

    int16_t dx = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    r->d[0] = set_word(r->d[0], (uint16_t)dx);
    r->d[1] = set_word(r->d[1], (uint16_t)dy);
    int16_t distance = dx < 0 ? (int16_t)-dx : dx;
    if (distance <= limit)
        distance = dy < 0 ? (int16_t)-dy : dy;
    r->d[3] = set_word(r->d[3], (uint16_t)distance);
    if (distance <= limit) {
        wr16(m, obj + 0x48u, 0); /* 0x179104 */
        return;
    }

    push_long(m, r, 0x1790B8u);
    push_long(m, r, obj); /* 0x006422 MOVEM */
    for (unsigned i = 4; i > 1;)
        push_long(m, r, r->d[--i]);
    uint16_t angle = Obj_AtanByte(m, dx, dy);
    r->a[7] += 20u;

    uint32_t record = rd32(m, obj + 0x44u);
    uint16_t speed = rd16(m, obj + 0x42u) + rd16(m, record + 6u);
    uint16_t adjustment = rd16(m, record + 8u);
    wr16(m, obj + 0x42u, speed);
    angle = (uint8_t)((angle & adjustment) + (adjustment >> 8));
    wr8(m, obj + 0x20u, (uint8_t)angle);

    int16_t magnitude = (int16_t)(speed + 0x20u) >> 6;
    r->d[2] = angle;
    r->a[1] = 0x027E40u;
    r->d[0] = (uint32_t)((int32_t)magnitude * (int16_t)rd16(m, 0x027EC0u + 2u * angle));
    r->d[1] = (uint32_t)((int32_t)magnitude * (int16_t)rd16(m, 0x027E40u + 2u * angle));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[0]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[1]);
}

/* 0x178982 specialized to the two commands in the fixed script at 0x19B74C.
 * Tracking the A1 and D1 clobbers matters even when drawing is skipped. */
static void step_approach_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, 0x19B6C6u);
    for (;;) {
        uint32_t record = rd32(m, obj + 0x44u);
        r->a[1] = record;
        uint32_t command = rd32(m, record);
        if (command != 0) {
            r->a[1] = command;
            push_long(m, r, 0x178990u);
            if (command == 0x178B20u) {
                r->a[1] = record;
                Obj_Script10_SetHeading(m, obj);
            } else { /* 0x179074 is the script's second and final callback. */
                approach(m, r);
            }
            r->a[7] += 4u;
            int16_t remaining = (int16_t)rd16(m, obj + 0x48u);
            if (remaining < 0)
                break;

            if (remaining == 0) {
                r->a[1] = record + 10u;
                wr32(m, obj + 0x44u, r->a[1]);
                wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
                continue;
            }

            wr16(m, obj + 0x48u, (uint16_t)(remaining - 1));
            if (remaining > 1)
                break;

            r->a[1] = record + 10u;
            if ((int32_t)rd32(m, r->a[1]) > 0)
                goto store;
        }

        /* 0x1789C8: both early arrival and timeout reach 0x19B760's
         * zero command, -1 repeat count, and -1 link. */
        int16_t repeat = (int16_t)rd16(m, obj + 0x4Au);
        if (repeat >= 0) {
            if (repeat == 0) {
                repeat = (int16_t)rd16(m, r->a[1] + 4u);
                wr16(m, obj + 0x4Au, (uint16_t)repeat);
            }
            --repeat;
            wr16(m, obj + 0x4Au, (uint16_t)repeat);
            if (repeat == 0) {
                r->a[1] += 10u;
                goto store;
            }
        }
        r->a[1] = rd32(m, r->a[1] + 6u);

store:
        wr32(m, obj + 0x44u, r->a[1]);
        wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
        break;
    }
    r->a[7] += 4u;
}

int Fn_19B65A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], globals = r->a[5];
    if (rd16(m, globals + 0x4B38u) == 0xFFFFu) {
        wr16(m, obj + 6u, 2u); /* 0x17A84C sets Z clear, so 0x17A852 falls through. */
        wr32(m, obj, 0x17A85Cu);
        r->a[7] += 4u;
        return 0;
    }

    uint32_t args = rd32(m, obj + 0x0Cu);
    int32_t bit = (int32_t)rd32(m, args);
    uint32_t bit_byte = globals + 0x4BA8u + (bit >> 3);
    if (!(rd8(m, bit_byte) & (0x80u >> (bit & 7)))) {
        r->a[1] = args + 4u;
        r->d[0] = (uint32_t)bit;
    } else {
        r->d[0] = (uint32_t)bit;
        r->a[3] = rd32(m, globals + 0x4BB4u);
        wr16(m, obj + 0x50u, rd16(m, r->a[3] + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, r->a[3] + 0x54u));
        r->d[0] = set_word(r->d[0], rd16(m, args + 4u));
        r->d[1] = set_word(r->d[1], rd16(m, args + 6u));
        wr16(m, obj + 0x50u, rd16(m, obj + 0x50u) + (uint16_t)r->d[0]);
        wr16(m, obj + 0x54u, rd16(m, obj + 0x54u) + (uint16_t)r->d[1]);
        r->a[2] = rd32(m, args + 8u);
        r->a[3] = rd32(m, r->a[2]);
        wr16(m, obj + 0x4Cu, rd16(m, r->a[3] + 0x50u));
        wr16(m, obj + 0x4Eu, rd16(m, r->a[3] + 0x54u));
        r->d[0] = set_word(r->d[0], rd16(m, args + 12u));
        r->d[1] = set_word(r->d[1], rd16(m, args + 14u));
        wr16(m, obj + 0x4Cu, rd16(m, obj + 0x4Cu) + (uint16_t)r->d[0]);
        wr16(m, obj + 0x4Eu, rd16(m, obj + 0x4Eu) + (uint16_t)r->d[1]);
        r->a[1] = 0x19B74Cu;
        wr32(m, obj + 0x44u, r->a[1]);
        wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
        wr16(m, obj + 0x4Au, 0);

        do { /* 0x19B6C0 */
            step_approach_script(m, r);
            wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
            wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

            uint16_t clock = rd16(m, globals + 0x878u);
            uint32_t animation = rd32(m, 0x19B6E0u + 0x8Au + 4u * (clock % 3u));
            wr32(m, obj + 0x3Au, animation);

            /* 0x19B6F0 brief extension 0x0604: D0.w scaled by eight.
             * 0x19B646 installs mode 16, and the script preserves it.
             * The tick pointer at 0x179AE0 is 0x179A1E. */
            push_long(m, r, 0x19B6F6u);
            Obj_AnimTick(m, obj, 0x10u);
            r->a[7] += 4u;

            uint32_t sprite = rd32(m, obj + 0x10u);
            uint16_t height = (uint16_t)(rd16(m, sprite) << 3);
            uint16_t width = (uint16_t)(rd16(m, sprite + 2u) << 3);
            r->d[1] = set_word(r->d[1], height);
            r->a[1] = sprite + 4u;
            wr16(m, obj + 0x14u, rd16(m, obj + 0x14u) - width);
            wr16(m, obj + 0x18u, rd16(m, obj + 0x18u) - height);

            r->d[0] = 4;
            push_long(m, r, 0x19B712u);
            push_long(m, r, r->d[2]);
            push_long(m, r, r->d[1]);
            r->d[0] = Obj_RandRange(m, 4);
            r->a[7] += 12u;
            if (!r->d[0])
                call_draw_sprite(m, r, 0x19B71Eu);
        } while (rd32(m, obj + 0x44u) != 0xFFFFFFFFu);
    }

    wr32(m, obj, 0x19B65Au); /* 0x19B652 */
    r->a[7] += 4u;
    return 0;
}
