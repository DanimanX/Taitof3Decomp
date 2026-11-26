/* ROM 0x129338: boss camera/script steering intro tick (A0 = object).
 * Steers +0x20.b toward (+0x50, +0x52) from (A5+0x30D2, A5+0x30D6) via 0x11B288
 * (0x006422 atan2 clamped to [-2..2]), eases +0x56.w toward +0x54.b into +0x42
 * via 0x11B2CA, integrates camera position at (A5+0x30D2, A5+0x30D6) via
 * 0x11B262 (0x0CBE12), and decrements +0x06. While +0x06 > 0 re-arms 0x129338;
 * otherwise sets +0x54 = 2, A5+0x30F2 = 0, sets bits 2/0/3 from MSB of
 * A5+0x30EE, sets +0x06 = 0xC8, and transitions to 0x12937E. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x006422 with exact D0 upper-word behaviour (exg.l d1,d0 on quadrant != 0,6). */
static uint32_t atan2_regs(const f3mem_t *m, uint32_t d0_in, uint32_t d1_in)
{
    int16_t x = (int16_t)(uint16_t)d0_in;
    int16_t y = (int16_t)(uint16_t)d1_in;
    uint16_t d2 = 0u;
    if (x < 0) d2 |= 2u;
    if (y < 0) d2 |= 4u;
    uint16_t ang = Obj_Atan2(m, (uint16_t)x, (uint16_t)y);
    uint32_t hi = (d2 != 0u && d2 != 6u) ? (d1_in & 0xFFFF0000u) : (d0_in & 0xFFFF0000u);
    return hi | (uint8_t)ang;
}

/* ROM 0x11B288: steer +0x20.b by [-2..2] toward (+0x50, +0x52) - (A5+30D2, A5+30D6). */
static void sub_11B288(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u));
    uint32_t d0 = atan2_regs(m, setw(r->d[0], dx), setw(r->d[1], dy));
    int8_t diff = (int8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff > 0) {
        if (diff > 2)
            diff = 2;
    } else if (diff < -2) {
        diff = -2;
    }
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));
    r->d[0] = (d0 & 0xFFFFFF00u) | (uint8_t)diff;
    r->d[1] = setw(r->d[1], dy);
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 >> 8) into +0x42. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);
    if (target > cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));
    else if (target < cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));
    int16_t spd = ASR16(rd16(m, obj + 0x56u), 8);
    wr16(m, obj + 0x42u, (uint16_t)spd);
    r->d[0] = setw(r->d[0], (uint16_t)spd);
}

/* ROM 0x11B262: integrate A5+0x30D2/30D6 along +0x20.b at speed +0x42 via 0x0CBE12. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    r->d[0] = setw(r->d[0], ang);

    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    int32_t vx = (int32_t)speed * (int32_t)c;
    int32_t vy = (int32_t)speed * (int32_t)s;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);
    r->d[1] = (uint32_t)vx;
    r->d[2] = (uint32_t)vy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

int Fn_129338(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11B288(m, r);                                     /* 0x129338 */
    sub_11B2CA(m, r);                                     /* 0x12933E */
    sub_11B262(m, r);                                     /* 0x129344 */

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x12934A */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                        /* 0x12934E */
        wr32(m, obj, 0x00129338u);                        /* 0x129330 */
        return 0;
    }

    wr8(m, obj + 0x54u, 2u);                              /* 0x129350 */
    wr8(m, F3_A5 + 0x30F2u, 0u);                          /* 0x129356 */
    /* 0x12935C..0x129368: bfset bits 2, 0, 3 from MSB of A5+0x30EE */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x20u | 0x80u | 0x10u));
    wr16(m, obj + 0x06u, 0x00C8u);                        /* 0x12936E (beq 0x129396 never taken) */
    wr32(m, obj, 0x0012937Eu);                            /* 0x129376 */
    return 0;
}
