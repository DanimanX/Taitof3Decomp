#include "fn_auto.h"
/* ROM 0x13AB36: verified: the capture discriminates (an empty body fails it): React to background stage changes with effects and tilemap strips, or clear the layers when the parent retires. */
#include "fn_sound_frames.h"
#include "../video/sprite.h"

/* 0x138772: select a strip destination and adjust its horizontal tile offset. */
static void draw_strip(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                       uint32_t descriptor, uint16_t y, uint32_t source)
{
    r->d[0] = set_word(r->d[0], 0);
    r->d[1] = set_word(r->d[1], y);
    r->d[2] = source;
    r->d[3] = set_word(r->d[3], 0);
    r->a[1] = descriptor;
    push_long(m, r, ret);

    /* 0x138778 and 0x1387A4 use the extension word as the PC base. */
    uint16_t index = (uint16_t)(rd16(m, descriptor + 6u) << 4);
    r->a[2] = rd32(m, 0x13877Au - 0x198u + (int16_t)index);
    int16_t scroll = (int16_t)rd16(m, r->a[2]) >> 7;
    r->d[5] = set_word(r->d[5], (uint16_t)scroll);
    int16_t group = scroll / 12;
    if (scroll < 0)
        --group; /* 0x138792 subtracts even for an exact negative multiple. */
    r->d[4] = (uint32_t)((int32_t)group * 12);
    uint16_t x = ((uint16_t)r->d[4] & 3u) << 3;
    r->d[0] = set_word(r->d[0], x);
    uint16_t layer = rd16(m, descriptor);
    r->d[3] = set_word(r->d[3], layer);
    r->a[1] = rd32(m, 0x1387A6u - 0x184u + 4 * (int32_t)(int16_t)layer);

    uint32_t sp = r->a[7];
    push_long(m, r, 0x1387B0u); /* 0x1387AC BSR to 0x1385B2 */
    push_long(m, r, r->a[1]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);
    uint32_t dst = r->a[1] + (int16_t)(x * 4u + y * 0x80u);
    uint32_t tile_source = source & 0x7FFFFFFFu;
    push_word(m, r, (uint16_t)((source >> 31) << 9));
    push_long(m, r, dst);
    push_long(m, r, tile_source);
    push_long(m, r, 0x1385D8u); /* 0x1385D2 JSR to 0x0053B2 */
    push_long(m, r, r->a[2]);
    push_long(m, r, dst);
    push_long(m, r, r->a[0]);
    for (unsigned i = 8; i > 4;)
        push_long(m, r, r->d[--i]);
    push_long(m, r, tile_source);
    push_long(m, r, r->d[2]);
    push_long(m, r, (uint32_t)((int32_t)(int16_t)y * 0x80));
    push_long(m, r, set_word(r->d[0], (uint16_t)(x * 4u + y * 0x80u)));
    push_long(m, r, 0x0053DEu); /* 0x0053DC calls the unflipped mode. */
    Fn_1385B2(m, r);
    r->a[7] = sp + 4u;
}

/* The stage paths set position, spawn the effect, and play sound 0x16. */
static void spawn_effect(const f3mem_t *m, f3regs_t *r, int16_t x_offset,
                         uint32_t arg, uint32_t spawn_ret)
{
    uint32_t obj = r->a[0];
    wr16(m, obj + 0x14u, rd16(m, r->a[5] + 0x30D2u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x14u) + x_offset);
    wr16(m, obj + 0x18u, rd16(m, r->a[5] + 0x30D6u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x18u)); /* ADDI.W #0 */
    call_spawn(m, r, spawn_ret, 0x13E4EEu, 5u, 2u, arg);
    call_hooked_sound(m, r, 0x16u, spawn_ret + 0xEu, spawn_ret + 0x14u);
}

/* 0x13AD2C: clear the three layers, then return the object to its pool. */
static void clear_layers_and_free(const f3mem_t *m, f3regs_t *r)
{
    static void (*const clear[])(const f3mem_t *) = {
        Gfx_ClearPf0, Gfx_ClearPf0b, Gfx_ClearPf1AndLineRam
    };
    uint32_t sp = r->a[7];
    for (unsigned i = 0; i < 3; ++i) {
        push_long(m, r, 0x13AD32u + 6u * i);
        push_long(m, r, r->a[1]);
        push_long(m, r, r->a[0]);
        if (i == 2)
            push_long(m, r, r->d[2]);
        push_long(m, r, r->d[1]);
        push_long(m, r, r->d[0]);
        clear[i](m);
        r->a[7] = sp;
    }
    call_free(m, r, 0x13AD46u);
}

int Fn_13AB36(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], globals = r->a[5];
    r->a[1] = rd32(m, obj + 8u);
    if (rd8(m, r->a[1] + 0x59u)) {
        clear_layers_and_free(m, r);
    } else {
        uint16_t old_stage = rd16(m, obj + 0x22u);
        r->d[0] = set_word(r->d[0], old_stage);
        uint16_t stage = rd16(m, globals + 0x32D6u);
        if (old_stage != stage) {
            switch (stage) {
            case 1: /* 0x13AB56 */
                spawn_effect(m, r, -0x280, 5u, 0x13AB88u);
                draw_strip(m, r, 0x13ABBAu, 0x13ABB4u - 0x24AAu, 2u, 0x08833Au);
                break;
            case 2: /* 0x13ABC4 */
                spawn_effect(m, r, -0x228, 5u, 0x13ABF6u);
                draw_strip(m, r, 0x13AC28u, 0x13AC22u - 0x2518u, 0u, 0x066D20u);
                draw_strip(m, r, 0x13AC42u, 0x13AC3Cu - 0x2602u, 1u, 0x0883A2u);
                break;
            case 3: { /* 0x13AC4C */
                uint16_t delay = rd16(m, obj + 6u) + 1u;
                wr16(m, obj + 6u, delay);
                if ((int16_t)delay < 0x78)
                    goto rearm; /* 0x13AC56 -> 0x13AB2E skips the stage copy. */
                spawn_effect(m, r, -0x1E0, 5u, 0x13AC8Cu);
                draw_strip(m, r, 0x13ACBEu, 0x13ACB8u - 0x267Eu, 1u, 0x066D20u);
                wr16(m, globals + 0x32D8u, 1u);
                break;
            }
            case 5: /* 0x13ACD8 */
                spawn_effect(m, r, -0xD8, 0xE00005u, 0x13AD0Au);
                wr16(m, globals + 0x32D8u, 3u);
                break;
            default: /* The stage-4 branch at 0x13ACCA targets its fall-through. */
                break;
            }
        }
        wr16(m, obj + 0x22u, rd16(m, globals + 0x32D6u)); /* 0x13AB28 */
rearm:
        wr32(m, obj, 0x13AB36u); /* 0x13AB2E */
    }
    r->a[7] += 4u;
    return 0;
}
