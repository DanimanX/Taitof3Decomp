#include "fn_auto.h"
/* ROM 0x11F18E: verified on a capture from the Y04 re-survey states: swing the camera, accelerate the angle, and trigger the next state. */

int Fn_11F18E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t camera_x = r->a[5] + 0x30D2;
    uint32_t camera_y = r->a[5] + 0x30D6;
    if (rd8(m, r->a[5] + 0x1D58) & 2u) {
        uint32_t step = (int16_t)rd16(m, camera_x) > 0xA0 ? 0xFFFF8000u : 0x8000u;
        wr32(m, camera_x, rd32(m, camera_x) + step);
    }

    r->d[0] = rd32(m, obj + 0x14); /* 0x11F1B0 */
    wr32(m, camera_x, rd32(m, camera_x) - r->d[0]);
    wr32(m, camera_y, rd32(m, obj + 0x18));
    r->a[1] = 0x27E40;
    r->d[0] = rd8(m, obj + 0x20);
    /* 0x11F1CA: 0x0320 encodes D0.w * 2 and a zero word displacement. */
    uint16_t sine = rd16(m, r->a[1] + 2u * r->d[0]);
    uint16_t offset = (uint16_t)((sine >> 7) | ((sine & 0x8000u) ? 0xFE00u : 0u));
    r->d[1] = (r->d[1] & 0xFFFF0000u) | offset;
    wr16(m, camera_y, (uint16_t)(rd16(m, camera_y) + offset));

    uint16_t speed = rd16(m, obj + 0x42);
    r->d[0] = speed;
    wr16(m, obj + 0x20, (uint16_t)(rd16(m, obj + 0x20) - speed));
    wr16(m, obj + 0x42, (uint16_t)(rd16(m, obj + 0x42) + 4u));
    if ((int16_t)rd16(m, obj + 0x20) <= 0) {
        wr32(m, obj, 0x11F18E); /* 0x11F186 */
        return 0;
    }

    /* 0x11F1E8: bitfield offset 12 is bit 3 in the second byte. */
    uint32_t flags = r->a[5] + 0x30EF;
    wr8(m, flags, rd8(m, flags) & 0xF7u);
    FnAuto_PlayHooked(m, r, 0x25, 0x11F1F8, 0x11F1FE);
    wr16(m, obj + 0x42, 0x10);
    wr16(m, obj + 0x20, 0x4000);
    wr16(m, obj + 0x06, 4);
    wr32(m, obj, 0x11F21C); /* 0x11F214 */
    return 0;
}
