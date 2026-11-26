/* ROM 0x12937E: boss camera/script steering tick (A0 = object).
 * Steers +0x20.b toward (+0x50, +0x52) from (A5+0x30D2, A5+0x30D6) via 0x11B288
 * (0x006422 atan2 clamped to [-2..2]), eases +0x56.w toward +0x54.b into +0x42
 * via 0x11B2CA, integrates camera position at (A5+0x30D2, A5+0x30D6) via
 * 0x11B262 (0x0CBE12), and decrements +0x06. When +0x06 > 0 re-arms 0x12937E;
 * otherwise clears A5+0x30EE bits 7/5/4 from MSB and transitions by A5+0x30DA
 * (>= 0x578 -> 0x129338, >= 0x3E8 -> 0x1293FA, >= 0x1F4 -> 0x129768,
 * < 0x1F4 -> 0x129D80) playing Sound_PlayHooked 0x13 per threshold step. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B288: steer +0x20.b by [-2..2] toward (+0x50, +0x52) - (A5+30D2, A5+30D6). */
static void sub_11B288(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u));
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int8_t diff = (int8_t)((uint8_t)ang - rd8(m, obj + 0x20u));
    if (diff > 0) {
        if (diff > 2)
            diff = 2;
    } else if (diff < -2) {
        diff = -2;
    }
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)diff;
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

int Fn_12937E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11B288(m, r);                                         /* 0x12937E */
    sub_11B2CA(m, r);                                         /* 0x129384 */
    sub_11B262(m, r);                                         /* 0x12938A */

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);       /* 0x129390 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                            /* 0x129394 */
        wr32(m, obj, 0x0012937Eu);                            /* 0x129376 */
        return 0;
    }

    /* 0x129396..0x1293A2: bfclr bits 2, 0, 3 from MSB of A5+0x30EE */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~(0x20u | 0x80u | 0x10u)));

    int16_t v = (int16_t)rd16(m, F3_A5 + 0x30DAu);            /* 0x1293A8 */
    if (v >= 0x0578) {                                        /* 0x1293AE */
        wr8(m, obj + 0x54u, 0u);                              /* 0x129324 */
        wr16(m, obj + 0x06u, 0x003Cu);                        /* 0x129328 */
        wr32(m, obj, 0x00129338u);                            /* 0x129330 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                             /* 0x1293B6..0x1293C6 */
    if (v >= 0x03E8) {                                        /* 0x1293CA..0x1293D0 */
        wr32(m, F3_A5 + 0x30EEu, 0u);                         /* 0x1293D4 */
        wr8(m, obj + 0x54u, 6u);                              /* 0x1293D8 */
        wr16(m, obj + 0x50u, 0x00FAu);                        /* 0x1293DE */
        wr16(m, obj + 0x52u, 0x0078u);                        /* 0x1293E4 */
        wr16(m, obj + 0x06u, 0x003Cu);                        /* 0x1293EA */
        wr32(m, obj, 0x001293FAu);                            /* 0x1293F2 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                             /* 0x129724..0x129734 */
    if (v >= 0x01F4) {                                        /* 0x129738..0x12973E */
        wr32(m, F3_A5 + 0x30EEu, 0u);                         /* 0x129742 */
        wr8(m, obj + 0x54u, 6u);                              /* 0x129746 */
        wr16(m, obj + 0x50u, 0x00FAu);                        /* 0x12974C */
        wr16(m, obj + 0x52u, 0x0078u);                        /* 0x129752 */
        wr16(m, obj + 0x06u, 0x003Cu);                        /* 0x129758 */
        wr32(m, obj, 0x00129768u);                            /* 0x129760 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                             /* 0x129D46..0x129D56 */
    wr32(m, F3_A5 + 0x30EEu, 0u);                             /* 0x129D5A */
    wr8(m, obj + 0x54u, 6u);                                  /* 0x129D5E */
    wr16(m, obj + 0x50u, 0x00FAu);                            /* 0x129D64 */
    wr16(m, obj + 0x52u, 0x0078u);                            /* 0x129D6A */
    wr16(m, obj + 0x06u, 0x003Cu);                            /* 0x129D70 */
    wr32(m, obj, 0x00129D80u);                                /* 0x129D78 */
    return 0;
}
