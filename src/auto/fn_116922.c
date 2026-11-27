#include "fn_auto.h"
/* ROM 0x116922: verified on a capture from the Y27d re-survey states (zone 27, input down): follow the boss target, update dive motion, and arm the next state. */
#include "../objects/obj_shared_ext.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

static void push_return(const f3mem_t *m, f3regs_t *r, uint32_t addr)
{
    r->a[7] -= 4;
    wr32(m, r->a[7], addr);
}

/* ROM 0x006422: the mixed-sign quadrants exchange the full inputs before the lookup. */
void FnAuto_Angle(const f3mem_t *m, f3regs_t *r)
{
    int16_t x = (int16_t)r->d[0];
    int16_t y = (int16_t)r->d[1];
    r->a[7] -= 16;
    wr32(m, r->a[7], r->d[1]);
    wr32(m, r->a[7] + 4, r->d[2]);
    wr32(m, r->a[7] + 8, r->d[3]);
    wr32(m, r->a[7] + 12, r->a[0]);
    uint32_t high = ((x < 0) != (y < 0)) ? r->d[1] : r->d[0];
    r->d[0] = with_word(high, Obj_AtanByte(m, x, y));
    r->a[7] += 16;
}

/* ROM 0x113456: during the global effect, temporarily use a random movement speed. */
static void scatter(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (!(rd8(m, r->a[5] + 0x1D58) & 2u))
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x14);
    if (x < -10 || x > 0x11C)
        return;

    r->a[7] -= 2;
    wr16(m, r->a[7], rd16(m, obj + 0x42));
    push_return(m, r, 0x113482);
    r->d[0] = Rng_NextMem(m);
    r->a[7] += 4;
    wr16(m, obj + 0x42, (uint16_t)((r->d[0] & 0x0Fu) - 5u));

    r->d[0] = with_word(r->d[0], (uint16_t)(rd16(m, obj + 0x14) - 0xA0u));
    r->d[1] = with_word(r->d[1], (uint16_t)(rd16(m, obj + 0x18) - 0x74u));
    push_return(m, r, 0x1134A2);
    FnAuto_Angle(m, r);
    r->a[7] += 4;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)(r->d[0] + 0x58u);

    push_return(m, r, 0x1134AC);
    FnAuto_MoveByAngle(m, r, (uint16_t)r->d[0]); /* 0x0CBE12 */
    r->a[7] += 4;
    wr16(m, obj + 0x42, rd16(m, r->a[7]));
    r->a[7] += 2;
}

/* ROM 0x1169D8: update the dive phase, fade, position, and shared scale. */
static void dive_tick(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t scale = r->a[5] + 0x30BC;
    if (rd8(m, obj + 0x51) & 2u) {
        /* 0x116AAA */
        wr16(m, obj + 0x3E, (uint16_t)(rd16(m, obj + 0x3E) + 0x10u));
        if ((int16_t)rd16(m, obj + 0x3E) > 0x400)
            wr16(m, obj + 0x3E, 0x400);
        wr16(m, scale, (uint16_t)(rd16(m, scale) + 0x100u));
        if ((int16_t)rd16(m, scale) > 0x3A00)
            wr16(m, scale, 0x3A00);
        wr32(m, obj + 0x14, rd32(m, obj + 0x14) - 0x4000u);
        wr32(m, obj + 0x18, rd32(m, obj + 0x18) + 0x4000u);
        wr16(m, obj + 0x40, (uint16_t)(rd16(m, obj + 0x40) + 1u));
        if ((int16_t)rd16(m, obj + 0x40) > 0xDC) {
            wr16(m, obj + 0x40, 0);
            wr8(m, obj + 0x51, rd8(m, obj + 0x51) & 0xFDu);
        }
    } else {
        /* 0x1169E2: BGE consumes the flags of SUBI.W, including overflow. */
        int16_t fade = (int16_t)rd16(m, obj + 0x3E);
        wr16(m, obj + 0x3E, (uint16_t)(fade - 0x10));
        if (fade < 0x10)
            wr16(m, obj + 0x3E, 0);
        int16_t phase = (int16_t)rd16(m, obj + 0x22);
        if (phase <= 0) {
            if (phase == 0) {
                r->d[0] = with_word(r->d[0], (uint16_t)(rd16(m, obj + 0x2C) - rd16(m, obj + 0x14)));
                r->d[1] = with_word(r->d[1], (uint16_t)(rd16(m, obj + 0x30) - rd16(m, obj + 0x18)));
                push_return(m, r, 0x116A12);
                FnAuto_Angle(m, r);
                r->a[7] += 4;
                wr16(m, obj + 0x20, (uint16_t)r->d[0]);
                wr16(m, obj + 0x42, 0x14);
            }
            /* 0x116A1C */
            wr16(m, obj + 0x22, (uint16_t)(rd16(m, obj + 0x22) + 1u));
            wr32(m, obj + 0x14, rd32(m, obj + 0x14) - 0x4000u);
            wr32(m, obj + 0x18, rd32(m, obj + 0x18) + 0x4000u);
        } else if (phase == 1) {
            /* 0x116A3E */
            wr16(m, scale, (uint16_t)(rd16(m, scale) + 0x200u));
            wr32(m, obj + 0x14, rd32(m, obj + 0x14) - 0x4000u);
            wr32(m, obj + 0x18, rd32(m, obj + 0x18) + 0x2000u);
            if ((int16_t)rd16(m, scale) >= 0x5000) {
                wr16(m, scale, 0x5000);
                wr16(m, obj + 0x22, (uint16_t)(rd16(m, obj + 0x22) + 1u));
            }
        } else {
            /* 0x116A6C */
            wr16(m, scale, (uint16_t)(rd16(m, scale) - 0x400u));
            if ((int16_t)rd16(m, scale) < -0x800)
                wr16(m, scale, 0xF800);
            r->d[0] = with_word(r->d[0], rd16(m, obj + 0x20));
            push_return(m, r, 0x116A8C);
            FnAuto_MoveByAngle(m, r, (uint16_t)r->d[0]); /* 0x0CBE12 */
            r->a[7] += 4;
            r->d[0] = with_word(r->d[0], rd16(m, r->a[5] + 0x878) & 1u);
            if ((uint16_t)r->d[0] == 0) {
                int16_t timer = (int16_t)rd16(m, obj + 0x42);
                wr16(m, obj + 0x42, (uint16_t)(timer - 1));
                if (timer <= 1)
                    wr16(m, obj + 0x22, 0xFFEC);
            }
        }
    }

    r->d[0] = with_word(r->d[0], rd16(m, obj + 0x3E)); /* 0x116B02 */
    uint32_t fade_total = r->a[5] + 0x30BA;
    wr16(m, fade_total, (uint16_t)(rd16(m, fade_total) + r->d[0]));
}

int Fn_116922(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (rd8(m, r->a[5] - 0x144B) & 0x20u)
        goto pause;
    r->a[1] = rd32(m, obj + 0x36);
    if (rd8(m, r->a[1] + 0x65) & 1u)
        goto pause;

    r->a[1] = rd32(m, obj + 0x36); /* 0x11693A */
    wr32(m, obj + 0x2C, rd32(m, r->a[1] + 0x14));
    wr16(m, obj + 0x2C, (uint16_t)(rd16(m, obj + 0x2C) + 0x18u));
    wr32(m, obj + 0x30, rd32(m, r->a[1] + 0x18));
    wr16(m, obj + 0x30, (uint16_t)(rd16(m, obj + 0x30) + 0x10u));

    push_return(m, r, 0x11695A);
    dive_tick(m, r);
    r->a[7] += 4;
    push_return(m, r, 0x116960);
    scatter(m, r);
    r->a[7] += 4;

    r->a[1] = rd32(m, obj + 0x36);
    r->a[1] = rd32(m, r->a[1] + 0x44);
    r->d[0] = 0xFFFFu ^ ((rd8(m, r->a[1]) >> 4) & 1u); /* 0x116968 */
    wr8(m, obj + 0x51, (rd8(m, obj + 0x51) & 0xFEu) | (r->d[0] & 1u));

    int16_t timer = (int16_t)rd16(m, obj + 0x06);
    wr16(m, obj + 0x06, (uint16_t)(timer - 1));
    if (timer > 1) {
        wr32(m, obj, 0x116922); /* 0x11691A */
        return 0;
    }

pause: /* 0x11697A */
    wr8(m, obj + 0x51, 0);
    wr8(m, obj + 0x1D, rd8(m, obj + 0x4F));
    wr8(m, obj + 0x50, rd8(m, obj + 0x50) | 0x10u);
    wr16(m, obj + 0x06, 0x78);
    /* 0x116990: MOVE.W #0x78 clears Z, so the free path is unreachable. */
    wr32(m, obj, 0x11699A);
    return 0;
}
