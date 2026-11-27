#ifndef FN_SOUND_FRAMES_H
#define FN_SOUND_FRAMES_H

#include "fn_call_frames.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"

/* 0x13F238 edits the pushed argument. 0x0039D8 preserves all registers. */
static inline void call_hooked_sound(const f3mem_t *m, f3regs_t *r, uint16_t id,
                                     uint32_t hook_ret, uint32_t sound_ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, id);
    push_long(m, r, hook_ret);
    uint16_t cmd = Sound_PreCmdHook(m, id);
    if (cmd != id)
        wr16(m, r->a[7] + 4u, cmd);
    r->a[7] += 4u;
    push_long(m, r, sound_ret);
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->d[--i]);

    /* Return addresses pushed by sound dispatch and its scratch writes. */
    if (!rd8(m, r->a[5] - 0x1444u) &&
        (int16_t)cmd >= 0 && (int16_t)cmd <= (int16_t)rd16(m, 0x6750u)) {
        uint32_t table = rd32(m, r->a[5] - 0x1448u);
        uint32_t data = rd32(m, table + 4u * cmd);
        uint8_t type = rd8(m, data++);
        push_long(m, r, 0x003A16u);
        uint32_t nested = r->a[7] - 4u;
        switch (type) {
        case 0:
            if ((int8_t)rd8(m, data) >= 0) {
                wr32(m, nested, 0x003A48u);
                wr32(m, nested, 0x003A5Au);
            } else {
                wr32(m, nested, 0x003A78u);
                wr32(m, nested, 0x003A8Au);
                wr32(m, nested, 0x003A9Cu);
            }
            break;
        case 1:
            wr32(m, nested, 0x003ABCu);
            wr32(m, nested, 0x003ACEu);
            wr32(m, nested, 0x003AE0u);
            break;
        case 2: {
            uint16_t off = (uint16_t)((rd8(m, data) - 1u) * 8u);
            off = (off & 0xFF00u) | (uint8_t)(off + rd8(m, data + 2u));
            uint32_t slot = r->a[5] + 0x817u + (int16_t)off;
            wr32(m, nested, slot);
            if (rd8(m, slot))
                wr32(m, nested - 4u, 0x003B12u);
            wr32(m, nested - 4u, 0x003B2Cu);
            wr32(m, nested - 4u, 0x003B46u);
            break;
        }
        default:
            break;
        }
    }
    Sound_PlayHooked(m, id);
    r->a[7] = sp;
}

#endif
