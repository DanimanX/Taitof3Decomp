#include "fn_auto.h"
/* ROM 0x18F432: step the return movement script, apply its displacement to the parent, and free the controller at the terminal record. */
#include "fn_call_frames.h"

/* 0x178CEA..0x178CFE: convert a signed velocity word to a 16.16 step. */
static uint32_t velocity_step(uint16_t velocity)
{
    uint32_t scaled = (uint32_t)(int32_t)(int16_t)(uint16_t)(velocity + 0x20u);
    return (scaled << 8) & 0xFFFFC000u;
}

/* ROM 0x18F4A2: advance Cartesian velocity (0x178CCE) until Y >= 210. */
static void move_until_bottom(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, 0x18F4A8u);
    r->a[1] = rd32(m, obj + 0x44u);
    uint16_t vx = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, r->a[1] + 6u));
    uint16_t vy = (uint16_t)(rd16(m, obj + 0x5Cu) + rd16(m, r->a[1] + 8u));
    wr16(m, obj + 0x42u, vx);
    wr16(m, obj + 0x5Cu, vy);
    r->d[0] = velocity_step(vx);
    r->d[1] = velocity_step(vy);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + r->d[0]);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + r->d[1]);
    r->d[0] = set_word(r->d[0], vx);
    r->d[1] = set_word(r->d[1], vy);
    push_long(m, r, 0x178D18u);
    FnAuto_Angle(m, r);                                /* 0x178D12 -> 0x006422 */
    r->a[7] += 4u;
    wr8(m, obj + 0x20u, (uint8_t)r->d[0]);
    r->a[7] += 4u;
    if ((int16_t)rd16(m, obj + 0x54u) >= 0xD2)
        wr16(m, obj + 0x48u, 0u);                      /* 0x18F4B2 */
}

/* ROM 0x178982, for the two commands in the script at 0x18F484. */
static void step_return_script(const f3mem_t *m, f3regs_t *r, uint32_t command)
{
    uint32_t obj = r->a[0];
    for (;;) {
        r->a[1] = rd32(m, obj + 0x44u);
        if (command == 0u)
            break;                                     /* 0x178988 -> 0x1789C8 */

        r->a[1] = command;
        push_long(m, r, 0x178990u);
        if (command == 0x178B38u) {
            r->a[1] = rd32(m, obj + 0x44u);
            wr16(m, obj + 0x42u, rd16(m, r->a[1] + 6u));
            wr16(m, obj + 0x5Cu, rd16(m, r->a[1] + 8u));
        } else {
            move_until_bottom(m, r);                   /* 0x18F4A2 */
        }
        r->a[7] += 4u;

        int16_t timer = (int16_t)rd16(m, obj + 0x48u);
        if (timer < 0)
            return;                                    /* 0x178994 */
        if (timer > 0) {
            wr16(m, obj + 0x48u, (uint16_t)(timer - 1));
            if (timer > 1)
                return;                                /* 0x1789B6 */
            r->a[1] = rd32(m, obj + 0x44u) + 10u;
            if ((int32_t)rd32(m, r->a[1]) > 0)
                goto store_record;                     /* 0x1789C4 -> 0x1789EE */
            break;
        }

        r->a[1] = rd32(m, obj + 0x44u) + 10u;          /* 0x17899C */
        wr32(m, obj + 0x44u, r->a[1]);
        wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
        command = rd32(m, r->a[1]);
    }

    int16_t repeat = (int16_t)rd16(m, obj + 0x4Au);      /* 0x1789C8 */
    if (repeat >= 0) {
        if (repeat == 0)
            wr16(m, obj + 0x4Au, rd16(m, r->a[1] + 4u));
        uint16_t remaining = (uint16_t)(rd16(m, obj + 0x4Au) - 1u);
        wr16(m, obj + 0x4Au, remaining);
        if (remaining == 0u) {
            r->a[1] += 10u;
            goto store_record;                         /* 0x1789E6 */
        }
    }
    r->a[1] = rd32(m, r->a[1] + 6u);                   /* 0x1789EA */
store_record:
    wr32(m, obj + 0x44u, r->a[1]);                      /* 0x1789F0 */
    wr16(m, obj + 0x48u, rd16(m, r->a[1] + 4u));
}

int Fn_18F432(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t script = rd32(m, obj + 0x44u);
    uint32_t command = rd32(m, script);
    /* 0x18F416 installs 0x18F484, whose only callable records are at
     * 0x18F484 and 0x18F48E. A zero command directly enters the loop marker. */
    if (command != 0u && script != 0x18F484u && script != 0x18F48Eu)
        return -1;

    r->a[4] = rd32(m, obj + 8u);                        /* 0x18F432 */
    wr16(m, obj + 0x50u, rd16(m, r->a[4] + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, r->a[4] + 0x54u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    push_long(m, r, 0x18F454u);
    step_return_script(m, r, command);
    r->a[7] += 4u;

    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u)));
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u)));
    wr16(m, r->a[4] + 0x50u, (uint16_t)(rd16(m, r->a[4] + 0x50u) + r->d[0]));
    wr16(m, r->a[4] + 0x54u, (uint16_t)(rd16(m, r->a[4] + 0x54u) + r->d[1]));
    if (rd32(m, obj + 0x44u) == UINT32_MAX)
        call_free(m, r, 0x18F47Eu);                    /* 0x18F476 */
    else
        wr32(m, obj, 0x18F432u);                       /* 0x18F42A */
    r->a[7] += 4u;
    return 0;
}
