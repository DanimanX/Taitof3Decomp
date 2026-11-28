#include "fn_auto.h"
/* ROM 0x1305C2: update and draw a beam from its parent, or release it and stop its sound. */
#include "batch_obj_regs.h"
#include "fn_call_frames.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_shared_ext.h"

/* 0x130592: command remap, forced sound dispatch, then Obj_Free. */
static void release_beam(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t globals = r->a[5];
    wr16(m, sp - 2u, 0x22u);
    wr32(m, sp - 6u, 0x13059Cu);
    uint16_t cmd = Sound_PreCmdHook(m, 0x22u);
    if (cmd != 0x22u)
        wr16(m, sp - 2u, cmd);
    wr32(m, sp - 6u, 0x1305A2u);

    uint32_t save = sp - 6u;                              /* 0x003B66 */
    for (unsigned i = 3; i != 0;)
        wr32(m, save -= 4u, r->a[--i]);
    for (unsigned i = 3; i != 0;)
        wr32(m, save -= 4u, r->d[--i]);
    uint32_t entry = rd32(m, rd32(m, globals - 0x1448u) + 4u * cmd);
    wr32(m, save - 4u, 0x003B92u);
    uint8_t type = rd8(m, entry);
    if (type <= 1u && rd8(m, globals - 0x1442u + (rd8(m, entry + 1u) & 0x7Fu)) != 0u) {
        wr32(m, save - 8u, 0x003BD0u);
        wr32(m, save - 8u, 0x003BDEu);
    } else if (type == 2u) {
        uint16_t slot = (uint16_t)((rd8(m, entry + 1u) - 1u) * 8u);
        slot = (slot & 0xFF00u) | (uint8_t)(slot + rd8(m, entry + 3u));
        if (rd8(m, globals + 0x817u + (int16_t)slot) != 0u)
            wr32(m, save - 8u, 0x003C0Eu);
    }
    Sound_PlayCmdForced(m, (int16_t)cmd);
    for (unsigned i = 0; i < 3; i++, save += 4u)
        r->d[i] = rd32(m, save);
    for (unsigned i = 0; i < 3; i++, save += 4u)
        r->a[i] = rd32(m, save);
    batch_free_object(m, r, 0x1305AEu);
}

/* 0x001282: reuse the shared renderer and scratch-register calculation.
 * The ROM keeps the column step in D6.w and the row step in D7.w. */
static void draw_beam_sprite(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    int multi = rd32(m, rd32(m, r->a[0] + 0x10u)) != 0x00010001u;
    batch_draw_sprite(m, r, ret);
    if (multi) {
        uint16_t col_step = (uint16_t)r->d[7];
        r->d[7] = set_word(r->d[7], (uint16_t)r->d[6]);
        r->d[6] = set_word(r->d[6], col_step);
    }
}

/* ROM 0x1305E4: copy the parent's position and add signed packed offsets. */
static void place_beam(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 8u);
    wr16(m, obj + 0x14u, rd16(m, r->a[1] + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, r->a[1] + 0x18u));
    r->d[0] = rd32(m, obj + 0x0Cu);
    r->d[1] = (uint32_t)(int32_t)(int8_t)(r->d[0] >> 24);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + r->d[1]));
    r->d[1] = (uint32_t)(int32_t)(int8_t)(r->d[0] >> 16);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + r->d[1]));
    r->a[7] += 4u;
}

/* ROM 0x13060A: queue the animated palette and draw the beam to the left. */
static void draw_beam(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    uint16_t active = rd16(m, globals + 0x32A6u);
    if (active != 0)
        wr16(m, globals + 0x32A6u, (uint16_t)(active + 1u));

    uint8_t flip = (rd8(m, globals + 0x879u) >> 1) & 1u;
    wr8(m, obj + 0x1Cu, flip);
    wr8(m, obj + 0x1Du, 0xDEu);
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x3Au));
    /* 0x130632 full extension 0520, signed D0.w * 4, word displacement 1E44. */
    r->a[1] = 0x132476u + (int16_t)r->d[0] * 4;
    FnAuto_QueuePalette(m, r, r->a[1], 0x7784u, 0x1Du, 0x13064Cu);

    r->a[1] = 0x37FB0u;
    wr32(m, obj + 0x10u, r->a[1]);
    int16_t scale = (int16_t)(rd8(m, obj + 0x1Eu) - 0x100u);
    r->d[0] = (uint32_t)((int32_t)scale * (int16_t)rd16(m, r->a[1]));
    int16_t y_delta = (int16_t)r->d[0] >> 5;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + y_delta));
    wr16(m, obj + 0x2Cu, 0);
    wr16(m, obj + 0x30u, 0);
    wr16(m, obj + 0x2Eu, 0xFFFFu);
    r->d[0] = set_word(r->d[0], (uint16_t)(-2 * y_delta));
    wr16(m, obj + 0x32u, (uint16_t)r->d[0]);

    for (;;) {                                           /* 0x130688 */
        uint16_t x = (uint16_t)(rd16(m, obj + 0x14u) - 0x20u);
        wr16(m, obj + 0x14u, x);
        wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x20u));
        if ((int16_t)x <= -0x20)
            break;
        draw_beam_sprite(m, r, 0x1306A4u);
    }
    uint16_t palette_frame = (uint16_t)(rd16(m, obj + 0x3Au) + 1u);
    wr16(m, obj + 0x3Au, palette_frame);
    if ((int16_t)palette_frame >= 0x1D)
        wr16(m, obj + 0x3Au, 0);
    r->a[7] += 4u;
}

/* ROM 0x1306BA: draw the beam emitter at the parent's position. */
static void draw_emitter(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_word(m, r, rd16(m, obj + 0x14u));
    push_word(m, r, rd16(m, obj + 0x18u));
    push_word(m, r, rd16(m, obj + 0x1Eu));
    r->a[1] = rd32(m, obj + 8u);
    wr16(m, obj + 0x14u, rd16(m, r->a[1] + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, r->a[1] + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x38u));
    wr16(m, obj + 0x1Eu, 0);
    wr8(m, obj + 0x1Cu, 0);
    wr8(m, obj + 0x1Du, rd8(m, r->a[1] + 0x1Du));
    wr32(m, obj + 0x10u, 0x37FCCu);
    draw_beam_sprite(m, r, 0x1306F8u);
    wr16(m, obj + 0x1Eu, rd16(m, r->a[7]));
    r->a[7] += 2u;
    wr16(m, obj + 0x18u, rd16(m, r->a[7]));
    r->a[7] += 2u;
    wr16(m, obj + 0x14u, rd16(m, r->a[7]));
    r->a[7] += 6u;
}

int Fn_1305C2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((rd8(m, r->a[5] - 0x144Bu) & 0x10u) == 0) {
        push_long(m, r, 0x1305CEu);
        place_beam(m, r);
        push_long(m, r, 0x1305D2u);
        draw_beam(m, r);
        push_long(m, r, 0x1305D6u);
        draw_emitter(m, r);

        uint8_t zoom = rd8(m, obj + 0x1Eu);
        if (zoom < 0xF8u) {                               /* 0x1305DC BCC */
            wr8(m, obj + 0x1Eu, (uint8_t)(zoom + 8u));
            wr32(m, obj, 0x1305C2u);                      /* 0x1305BA */
            r->a[7] += 4u;
            return 0;
        }
    }
    release_beam(m, r);
    r->a[7] += 4u;
    return 0;
}
