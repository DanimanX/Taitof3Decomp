#include "fn_auto.h"
/* ROM 0x120BD6: verified on a capture from the Y04 re-survey states: initialize a projectile from its parent and set its velocity. */

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

int Fn_120BD6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x10, 0x294E8);
    wr16(m, obj + 0x1E, 0);
    wr16(m, obj + 0x1C, 0);
    wr16(m, obj + 0x2C, 6);
    wr16(m, obj + 0x2E, 9);
    wr16(m, obj + 0x30, 6);
    wr16(m, obj + 0x32, 9);
    wr16(m, obj + 0x50, 0xFFF8);
    wr16(m, obj + 0x52, 0xFFF8);
    wr32(m, obj + 0x24, 0x10000);

    r->a[7] -= 4;
    wr32(m, r->a[7], 0x120C18);
    FnAuto_InheritSpawnPosition(m, obj, &r->d[0], &r->d[1], &r->a[1]); /* 0x0CCC08 */
    r->a[7] += 4;

    r->a[1] = 0x27E40; /* 0x120C18 */
    r->d[0] = with_word(r->d[0], rd16(m, obj + 0x20));
    r->d[1] = with_word(r->d[1], 20);
    r->d[2] = with_word(r->d[2], (uint16_t)r->d[1]);

    /* 0x120C28, 0x120C32: extension 0x0320 selects D0.w * 2 and d16. */
    uint32_t index = (uint32_t)((int32_t)(int16_t)r->d[0] * 2);
    r->d[1] = (uint32_t)((int16_t)r->d[1] * (int16_t)rd16(m, r->a[1] + index + 0x80));
    wr32(m, obj + 0x3A, r->d[1]);
    r->d[2] = (uint32_t)((int16_t)r->d[2] * (int16_t)rd16(m, r->a[1] + index));
    wr32(m, obj + 0x3E, r->d[2]);
    wr32(m, obj, 0x120C44); /* 0x120C3C */
    return 0;
}
