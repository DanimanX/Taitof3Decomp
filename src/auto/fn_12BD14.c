#include "fn_auto.h"
/* ROM 0x12BD14: Animate a moving boss effect and chase its target until it reaches the target or is retired. */
#include "fn_boss_camera.h"
#include "batch_obj_regs.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBD52. Obj_Animate implements the table and timer loop. */
static void load_direction_table(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    /* Extension at 0x12BBE8 + 0x156, table of {duration, pointer}. */
    r->a[1] = 0x12BD3Eu;
    wr32(m, r->a[7] - 4u, 0x12BBF2u);
    Obj_Animate(m, obj, r->a[1]);
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = set_word(r->d[1], rd16(m, obj + 0x3Eu));
    wr32(m, obj + 0x36u, rd32(m, obj + 0x10u));
}

/* ROM 0x12BD18 through 0x12BD38. Remap the stack argument, invoke
 * the forced sound entry, then free the object. */
static void retire_effect(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    push_word(m, r, 0x2Fu);
    push_long(m, r, 0x12BD22u);
    int16_t command = Sound_PreCmdHook(m, 0x2Fu);
    if (command != 0x2F)
        wr16(m, sp - 2u, (uint16_t)command); /* 0x13F254 */

    wr32(m, r->a[7], 0x12BD28u);
    for (unsigned i = 3; i != 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 3; i != 0;)
        push_long(m, r, r->d[--i]);

    uint32_t table = rd32(m, r->a[5] - 0x1448u);
    uint32_t data = rd32(m, table + (uint32_t)(4 * (int32_t)command));
    uint8_t type = rd8(m, data++);
    push_long(m, r, 0x003B92u); /* 0x003B8E */
    if (type <= 1u) {
        uint32_t slot = r->a[5] - 0x1442u + (rd8(m, data) & 0x7Fu);
        if (rd8(m, slot) != 0) {
            wr32(m, r->a[7] - 4u, 0x003BD0u);
            wr32(m, r->a[7] - 4u, 0x003BDEu);
        }
    } else if (type == 2u) {
        uint16_t offset = (uint16_t)((rd8(m, data) - 1u) * 8u);
        offset = (uint16_t)((offset & 0xFF00u) | (uint8_t)(offset + rd8(m, data + 2u)));
        uint32_t slot = r->a[5] + 0x817u + (uint32_t)(int32_t)(int16_t)offset;
        if (rd8(m, slot) != 0)
            wr32(m, r->a[7] - 4u, 0x003C0Eu);
    }
    Sound_PlayCmdForced(m, command);
    r->a[7] = sp;
    call_free(m, r, 0x12BD34u);
}

int Fn_12BD14(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    if ((rd8(m, globals - 0x144Bu) & 0x10u) != 0) /* 0x12BBC0 */
        goto retire;

    uint8_t flags = rd8(m, globals + 0x30EFu);
    wr8(m, globals + 0x30EFu, (uint8_t)(flags & ~0x10u)); /* 0x12BBC4 BFCLR {11:1} */
    if ((flags & 0x10u) != 0)
        goto retire;

    r->a[1] = rd32(m, obj + 8u);
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
    load_direction_table(m, r);

    for (;;) { /* 0x12BBF8, back edge at 0x12BD04 */
        uint32_t target = rd32(m, globals + 0x30E2u);
        r->a[1] = target;
        uint16_t speed = rd16(m, obj + 6u) >> 2;
        if (speed > 0xF0u)
            speed = 0xF0u;

        wr32(m, r->a[7] - 4u, 0x12BC14u);
        r->d[0] = Rng_NextMem(m);
        speed += r->d[0] & 0x1Fu;
        r->d[2] = set_word(r->d[2], speed);

        uint16_t dx = (uint16_t)(rd16(m, globals + 0x30D2u) - 0x63u - rd16(m, target + 0x14u));
        uint16_t dy = (uint16_t)(rd16(m, globals + 0x30D6u) - 0x0Eu - rd16(m, target + 0x18u));
        r->d[0] = set_word(r->d[0], dx);
        r->d[1] = set_word(r->d[1], dy);
        boss_angle(m, r, 0x12BC38u);

        r->a[2] = 0x027E40u;
        uint32_t trig = r->a[2] + 2u * (uint16_t)r->d[0];
        int32_t scaled_speed = speed * 12;
        r->d[1] = (uint32_t)((scaled_speed * (int16_t)rd16(m, trig + 0x80u)) >> 8);
        r->d[2] = (uint32_t)((scaled_speed * (int16_t)rd16(m, trig)) >> 8);
        wr32(m, target + 0x14u, rd32(m, target + 0x14u) + r->d[1]);
        wr32(m, target + 0x18u, rd32(m, target + 0x18u) + r->d[2]);

        int16_t center_x = (int16_t)(rd16(m, target + 0x2Cu) + rd16(m, target + 0x2Eu));
        int16_t center_y = (int16_t)(rd16(m, target + 0x30u) + rd16(m, target + 0x32u));
        dx = (uint16_t)((center_x >> 1) + rd16(m, target + 0x14u) - 8 - rd16(m, obj + 0x14u));
        dy = (uint16_t)((center_y >> 1) + rd16(m, target + 0x18u) - 8 - rd16(m, obj + 0x18u));
        r->d[0] = set_word(r->d[0], dx);
        r->d[1] = set_word(r->d[1], dy);
        if ((int16_t)dx >= -0x10 && (int16_t)dx <= 0x10 &&
            (int16_t)dy >= -0x10 && (int16_t)dy <= 0x10)
            break; /* 0x12BCA4 */

        boss_angle(m, r, 0x12BCAEu);
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
        wr32(m, r->a[7] - 4u, 0x12BCB6u);
        r->d[0] = Rng_NextMem(m);
        uint8_t jitter = (uint8_t)((r->d[0] & 0x3Fu) - 0x20u);
        r->d[0] = set_word(r->d[0], jitter);
        uint16_t heading = (uint8_t)(r->d[1] + jitter);
        r->d[1] = set_word(r->d[1], heading);
        wr16(m, obj + 0x20u, heading);

        r->d[0] = set_word(r->d[0], heading);
        boss_move(m, r, 0x12BCCCu);
        heading = rd16(m, obj + 0x20u);
        uint16_t direction = (uint16_t)((heading & 0xFF00u) | (uint8_t)(heading + 0x48u));
        r->d[1] = (direction >> 6) & 3u; /* 0x12BCD4 BFEXTU {24:2} */
        r->d[0] = set_word(r->d[0], (direction >> 4) & 7u);
        r->a[1] = rd32(m, obj + 0x36u);
        wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * (uint16_t)r->d[0]));
        r->a[1] = 0x0CC10Eu;
        wr8(m, obj + 0x1Cu, rd8(m, r->a[1] + r->d[1]));
        batch_draw_sprite(m, r, 0x12BCFAu);

        r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u));
        boss_move(m, r, 0x12BD04u);
    }

    wr16(m, obj + 6u, (uint16_t)(rd16(m, obj + 6u) + 1u)); /* 0x12BD08 */
    wr32(m, obj, 0x12BD14u);
    r->a[7] += 4u;
    return 0;

retire:
    retire_effect(m, r);
    r->a[7] += 4u;
    return 0;
}
