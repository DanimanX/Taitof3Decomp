#include "fn_auto.h"
/* ROM 0x12BB96: Move the object toward the camera's target, drawing along the way, or retire it on a stop flag. */
#include "batch_obj_regs.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x006422 preserves D1-D3/A0 and exchanges the full inputs in mixed quadrants. */
static void angle_to_target(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, ret);
    wr32(m, sp - 8u, r->a[0]);
    wr32(m, sp - 12u, r->d[3]);
    wr32(m, sp - 16u, r->d[2]);
    wr32(m, sp - 20u, r->d[1]);

    int16_t x = (int16_t)r->d[0];
    int16_t y = (int16_t)r->d[1];
    uint32_t high = ((x < 0) != (y < 0)) ? r->d[1] : r->d[0];
    r->d[0] = batch_set_word(high, Obj_AtanByte(m, x, y));
}

/* ROM 0x0CBE12 leaves both signed products in D1/D2 and the table in A3. */
static void move_object(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    wr32(m, r->a[7] - 4u, ret);
    uint16_t angle = (uint16_t)r->d[0];
    int16_t speed = (int16_t)rd16(m, r->a[0] + 0x42u);
    r->a[3] = 0x27E40u;
    r->d[1] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, r->a[3] + 0x80u + 2u * angle));
    r->d[2] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, r->a[3] + 2u * angle));
    Obj_MoveByAngle(m, r->a[0], angle);
}

/* 0x12BD18: remap and stop command 0x2F through the forced dispatcher. */
static void stop_sound(const f3mem_t *m, const f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t globals = r->a[5];
    wr16(m, sp - 2u, 0x2Fu);
    wr32(m, sp - 6u, 0x12BD22u);
    uint16_t cmd = Sound_PreCmdHook(m, 0x2Fu);
    if (cmd != 0x2Fu)
        wr16(m, sp - 2u, cmd);
    wr32(m, sp - 6u, 0x12BD28u);

    /* 0x003B66 saves D0-D2/A0-A2, then dispatches through 0x003B9E. */
    for (unsigned i = 3u; i > 0u; --i)
        wr32(m, sp - 22u + 4u * i, r->a[i - 1u]);
    for (unsigned i = 3u; i > 0u; --i)
        wr32(m, sp - 34u + 4u * i, r->d[i - 1u]);
    uint32_t entry = rd32(m, rd32(m, globals - 0x1448u) + 4u * cmd);
    wr32(m, sp - 34u, 0x003B92u);
    uint8_t type = rd8(m, entry);
    if (type <= 1u && rd8(m, globals - 0x1442u + (rd8(m, entry + 1u) & 0x7Fu)) != 0u) {
        wr32(m, sp - 38u, 0x003BD0u);
        wr32(m, sp - 38u, 0x003BDEu);
    } else if (type == 2u) {
        uint16_t slot = (uint16_t)((rd8(m, entry + 1u) - 1u) * 8u);
        slot = (uint16_t)((slot & 0xFF00u) | (uint8_t)(slot + rd8(m, entry + 3u)));
        if (rd8(m, globals + 0x817u + (uint32_t)(int32_t)(int16_t)slot) != 0u)
            wr32(m, sp - 38u, 0x003C0Eu);
    }
    Sound_PlayCmdForced(m, (int16_t)cmd);
}

static uint16_t target_delta(const f3mem_t *m, uint32_t target, uint32_t obj,
                             uint32_t bounds, uint32_t pos)
{
    uint16_t middle = (uint16_t)(rd16(m, target + bounds) + rd16(m, target + bounds + 2u));
    int16_t half = (int16_t)middle >> 1;
    return (uint16_t)(half + rd16(m, target + pos) - 8 - rd16(m, obj + pos));
}

int Fn_12BB96(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];

    FnAuto_PlayHooked(m, r, 0x2Fu, 0x12BBA0u, 0x12BBA6u);
    wr32(m, sp - 4u, 0x12BBB0u);
    Obj_ResetFields(m, obj);
    wr16(m, obj + 6u, 0u);
    wr16(m, obj + 0x42u, 0x20u);

    /* 0x12BBC4 tests the old bit while clearing it. 0x12BBC0 bypasses it. */
    int stop = (rd8(m, globals - 0x144Bu) & 0x10u) != 0u;
    if (!stop) {
        uint8_t flags = rd8(m, globals + 0x30EFu);
        wr8(m, globals + 0x30EFu, (uint8_t)(flags & ~0x10u));
        stop = (flags & 0x10u) != 0u;
    }
    if (stop) {
        stop_sound(m, r);
        batch_free_object(m, r, 0x12BD34u);
        r->a[7] += 4u;
        return 0;
    }

    /* 0x12BBCE..0x12BBF2 initializes the position and the animation. */
    r->a[1] = rd32(m, obj + 8u);
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
    r->a[1] = 0x12BD3Eu; /* 0x12BBE8 + 0x156 */
    wr32(m, sp - 4u, 0x12BBF2u);
    Obj_Animate(m, obj, r->a[1]);
    r->d[0] = batch_set_word(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = batch_set_word(r->d[1], rd16(m, obj + 0x3Eu));
    wr32(m, obj + 0x36u, rd32(m, obj + 0x10u));

    for (;;) { /* 0x12BBF8 */
        uint32_t target = rd32(m, globals + 0x30E2u);
        r->a[1] = target;
        uint16_t speed = rd16(m, obj + 6u) >> 2;
        /* 0x12BC06 also serves the 0x12BD14 re-entry, which skips the timer clear. */
        if (speed > 0xF0u)
            speed = 0xF0u;
        wr32(m, sp - 4u, 0x12BC14u);
        r->d[0] = Rng_NextMem(m);
        speed = (uint16_t)(speed + (r->d[0] & 0x1Fu));
        r->d[2] = batch_set_word(r->d[2], speed);
        r->d[0] = batch_set_word(r->d[0], (uint16_t)(rd16(m, globals + 0x30D2u) - 0x63u - rd16(m, target + 0x14u)));
        r->d[1] = batch_set_word(r->d[1], (uint16_t)(rd16(m, globals + 0x30D6u) - 0xEu - rd16(m, target + 0x18u)));
        angle_to_target(m, r, 0x12BC38u);

        /* 0x12BC38..0x12BC58 moves the target using the signed sine table. */
        r->a[2] = 0x27E40u;
        uint32_t phase = 2u * (uint16_t)r->d[0];
        speed = (uint16_t)(speed * 12u);
        int32_t dx = ((int32_t)speed * (int16_t)rd16(m, r->a[2] + 0x80u + phase)) >> 8;
        int32_t dy = ((int32_t)speed * (int16_t)rd16(m, r->a[2] + phase)) >> 8;
        r->d[1] = (uint32_t)dx;
        r->d[2] = (uint32_t)dy;
        wr32(m, target + 0x14u, rd32(m, target + 0x14u) + (uint32_t)dx);
        wr32(m, target + 0x18u, rd32(m, target + 0x18u) + (uint32_t)dy);

        int16_t x = (int16_t)target_delta(m, target, obj, 0x2Cu, 0x14u);
        int16_t y = (int16_t)target_delta(m, target, obj, 0x30u, 0x18u);
        r->d[0] = batch_set_word(r->d[0], (uint16_t)x);
        r->d[1] = batch_set_word(r->d[1], (uint16_t)y);
        if (x >= -16 && x <= 16 && y >= -16 && y <= 16)
            break; /* 0x12BCA4 branches to 0x12BD08. */

        /* 0x12BCA8..0x12BCCC applies a random byte offset to the heading. */
        angle_to_target(m, r, 0x12BCAEu);
        r->d[1] = batch_set_word(r->d[1], (uint16_t)r->d[0]);
        wr32(m, sp - 4u, 0x12BCB6u);
        r->d[0] = Rng_NextMem(m);
        uint8_t angle = (uint8_t)(r->d[1] + (r->d[0] & 0x3Fu) - 0x20u);
        wr16(m, obj + 0x20u, angle);
        r->d[0] = batch_set_word(r->d[0], angle);
        move_object(m, r, 0x12BCCCu);

        /* 0x12BCD0..0x12BCEE selects a sprite and flip from the adjusted heading. */
        uint8_t adjusted = (uint8_t)(rd8(m, obj + 0x21u) + 0x48u);
        uint16_t index = (uint16_t)((adjusted >> 4) & 7u);
        r->d[1] = adjusted >> 6;
        r->d[0] = batch_set_word(r->d[0], index);
        r->a[1] = rd32(m, obj + 0x36u);
        wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * index));
        r->a[1] = 0xCC10Eu;
        wr8(m, obj + 0x1Cu, rd8(m, r->a[1] + r->d[1]));
        batch_draw_sprite(m, r, 0x12BCFAu);
        r->d[0] = batch_set_word(r->d[0], rd16(m, obj + 0x20u));
        move_object(m, r, 0x12BD04u);
    }

    wr16(m, obj + 6u, (uint16_t)(rd16(m, obj + 6u) + 1u)); /* 0x12BD08 */
    wr32(m, obj, 0x12BD14u);
    r->a[7] += 4u;
    return 0;
}
