#include "fn_auto.h"
/* ROM 0x120C44: verified on a capture from the Y04 re-survey states: Integrate the falling hazard, draw it, and switch to its exit animation or explosion. */
#include "batch_obj_regs.h"
#include "../objects/obj_shared.h"

/* ROM 0x00649A returns the scaled distance in D0.W after swapping the
 * long result. Keep the other half for the exit from 0x0CBB98. */
static uint32_t distance_regs(const f3mem_t *m, uint16_t x, uint16_t y)
{
    uint32_t sum = (uint32_t)((int32_t)(int16_t)x * (int16_t)x)
                 + (uint32_t)((int32_t)(int16_t)y * (int16_t)y);
    sum *= 2u;
    unsigned shift = 4u;
    while (shift < 16u && (int32_t)sum >= 0x200) {
        sum >>= 8;
        shift += 4u;
    }
    uint32_t scaled = batch_set_word(sum, rd16(m, 0x028340u + (int32_t)(int16_t)sum * 2));
    scaled <<= shift;
    return (scaled << 16) | (scaled >> 16);
}

/* ROM 0x0CBB2C is a tail call. The shared helper performs its memory
 * writes. Track the distance and angle paths around its register saves. */
static void explode_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7];
    uint32_t sprite = rd32(m, obj + 0x10u);
    uint16_t h = rd16(m, sprite);
    uint16_t w = rd16(m, sprite + 2u);
    int16_t off_y = (int16_t)((rd8(m, obj + 0x1Eu) - 256) * (int16_t)h);
    int16_t off_x = (int16_t)((rd8(m, obj + 0x1Fu) - 256) * (int16_t)w);
    uint16_t x = (uint16_t)(rd16(m, obj + 0x14u) - (off_x >> 5) - 0xA0);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x18u) - (off_y >> 5) - 0x74);
    r->a[1] = sprite + 4u;
    r->d[1] = batch_set_word(r->d[1], y);
    wr32(m, sp - 4u, 0x0CBB92u);
    wr32(m, sp - 8u, obj);
    wr32(m, sp - 12u, r->d[1]);

    uint32_t distance = distance_regs(m, x, y);
    uint16_t radius = (uint16_t)(distance << 2);
    uint16_t threshold = rd16(m, r->a[5] + 0x1D5Cu);
    r->d[0] = batch_set_word(distance, (uint16_t)(radius - threshold));
    if ((int16_t)radius >= (int16_t)threshold) {
        wr32(m, sp - 4u, 0x0CBBB8u);
        wr32(m, sp - 8u, obj);
        wr32(m, sp - 12u, r->d[3]);
        wr32(m, sp - 16u, r->d[2]);
        wr32(m, sp - 20u, r->d[1]);
        uint32_t angle_high = ((int16_t)x < 0) != ((int16_t)y < 0) ? r->d[1] : distance;
        r->d[0] = batch_set_word(angle_high, (uint16_t)(Obj_Atan2(m, x, y) << 8));
    }
    int draws = (int16_t)radius >= (int16_t)threshold;
    if (draws)
        wr32(m, sp - 4u, 0x0CBC70u);
    Obj_ExplodeTick(m, obj);
    if (draws) {
        uint16_t angle = rd16(m, obj + 0x2Eu) >> 8;
        int16_t distance_to_center = (int16_t)((rd16(m, obj + 0x2Cu) >> 6) + threshold);
        r->d[1] = (uint32_t)((int32_t)distance_to_center *
                   (int16_t)rd16(m, 0x027EC0u + angle * 2u)) + 0x00A00000u;
        r->d[2] = (uint32_t)((int32_t)distance_to_center *
                   (int16_t)rd16(m, 0x027E40u + angle * 2u)) + 0x00740000u;
        r->d[0] = (uint32_t)(-256 * (int16_t)h);
        r->d[0] = batch_set_word(r->d[0], (uint16_t)((int16_t)r->d[0] >> 5));
        batch_sprite_outputs(m, r);
    }
}

int Fn_120C44(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if ((int32_t)rd32(m, obj + 0x24u) > 0 && (rd8(m, a5 - 0x144Bu) & 0x10u) == 0) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if ((rd8(m, a5 + 0x1D58u) & 2u) != 0 && x >= -10 && x <= 0x11C) {
            explode_regs(m, r); /* 0x120C6E */
            return 0;
        }

        r->d[0] = rd32(m, obj + 0x3Au);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + r->d[0]);
        r->d[0] = rd32(m, obj + 0x3Eu);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + r->d[0]);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x1C00u);
        r->a[1] = rd32(m, a5 + 0x906u);
        wr32(m, r->a[1], obj);
        r->a[1] += 4u;
        wr32(m, a5 + 0x906u, r->a[1]);
        batch_draw_sprite(m, r, 0x120C9Cu);

        x = (int16_t)rd16(m, obj + 0x14u);
        r->d[0] = batch_set_word(r->d[0], (uint16_t)x);
        if (x > (int16_t)rd16(m, 0x0CC112u)) {
            int16_t y = (int16_t)rd16(m, obj + 0x18u);
            r->d[0] = batch_set_word(r->d[0], (uint16_t)y);
            if (y < (int16_t)rd16(m, 0x0CC118u)) {
                wr32(m, obj, 0x120C44u); /* 0x120CB8 -> 0x120C3C */
                return 0;
            }
        }
    }

    wr16(m, obj + 0x42u, 0u);
    /* 0x0CD3D8 + 0x6C, with D0.W fixed to 0 by 0x120CBE.
     * The selected record at 0x0CD450 starts with a zero sound ID. */
    r->a[1] = rd32(m, 0x0CD444u);
    r->d[0] = batch_set_word(r->d[0], rd16(m, r->a[1]));
    r->a[1] += 2u;
    wr32(m, obj + 0x0Cu, r->a[1]);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj, 0x0CD412u);
    return 0;
}
