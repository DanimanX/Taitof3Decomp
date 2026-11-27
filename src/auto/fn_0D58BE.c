#include "fn_auto.h"
/* ROM 0x0D58BE: Check proximity, screen bounds and the fire roll before spawning an aimed shot. */
#include "fn_call_frames.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared.h"

/* The ASR.W in 0x0CC1E0, 0x0CC24A, 0x0CC306 and 0x0CC33A. */
static uint16_t half_word(uint16_t value)
{
    return (value >> 1) | (value & 0x8000u);
}

/* Each CMP2.W addresses adjacent lower and upper bounds. */
static int outside_bounds(const f3mem_t *m, uint16_t value, uint32_t bounds)
{
    return (int16_t)value < (int16_t)rd16(m, bounds) ||
           (int16_t)value > (int16_t)rd16(m, bounds + 2u);
}

/* 0x0CC1E0: walk the list and preserve the final A1/A2 and D0/D1. */
static int nearby(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = F3_A5 + 0x1D2Cu;
    uint16_t count = rd16(m, F3_A5 + 0x1D34u);
    for (;;) {
        uint16_t remaining = (uint16_t)(count - 1u);
        r->d[0] = set_word(r->d[0], remaining);
        if ((int16_t)count <= 0) { /* 0x0CC1EA BLT tests N xor V. */
            r->d[0] &= 0xFFFFFF00u;
            return 0;
        }
        count = remaining;
        uint32_t neighbor = rd32(m, r->a[1]);
        r->a[1] += 4u;
        r->a[2] = neighbor;

        uint16_t dx = (uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x2Eu) -
                                  rd16(m, neighbor + 0x2Cu) - rd16(m, neighbor + 0x2Eu));
        dx = (uint16_t)(half_word(dx) + rd16(m, obj + 0x14u) - rd16(m, neighbor + 0x14u));
        r->d[1] = set_word(r->d[1], dx);
        if (outside_bounds(m, dx, 0x0CC242u)) /* 0x0CC20E + 0x34 */
            continue;

        uint16_t dy = (uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u) -
                                  rd16(m, neighbor + 0x30u) - rd16(m, neighbor + 0x32u));
        dy = (uint16_t)(half_word(dy) + rd16(m, obj + 0x18u) - rd16(m, neighbor + 0x18u));
        r->d[1] = set_word(r->d[1], dy);
        if (outside_bounds(m, dy, 0x0CC246u)) /* 0x0CC232 + 0x14 */
            continue;

        r->d[0] |= 0xFFu; /* 0x0CC238 ST */
        return 1;
    }
}

/* 0x0CC24A: test the horizontal centre first and leave the last coordinate in D1.w. */
static int offscreen(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t x = half_word((uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x2Eu)));
    x = (uint16_t)(x + rd16(m, obj + 0x14u));
    r->d[1] = set_word(r->d[1], x);
    int outside = outside_bounds(m, x, 0x0CC288u); /* 0x0CC25C + 0x2C */
    if (!outside) {
        uint16_t y = half_word((uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u)));
        y = (uint16_t)(y + rd16(m, obj + 0x18u));
        r->d[1] = set_word(r->d[1], y);
        outside = outside_bounds(m, y, 0x0CC28Cu); /* 0x0CC276 + 0x16 */
    }
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (outside ? 0xFFu : 0u);
    return outside;
}

/* 0x0CC306: average the difficulty threshold and D1.w, then roll once. */
static int fire_blocked(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x0CC30Cu);
    uint16_t threshold = Diff_FireChance(m);
    r->a[7] += 4u;
    threshold = half_word((uint16_t)(threshold + r->d[1]));
    r->d[1] = set_word(r->d[1], threshold);

    push_long(m, r, 0x0CC316u);
    uint32_t random = Rng_NextMem(m);
    r->a[7] += 4u;
    int blocked = (int16_t)threshold <= (uint8_t)random; /* CMP.W; SLE */
    r->d[0] = (random & 0xFFFF0000u) | (blocked ? 0xFFu : 0u);
    return blocked;
}

/* 0x0CC33A: aim from the supplied offset toward the player's hitbox centre. */
static void aim(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t player = rd32(m, F3_A5 + 0x1D28u);
    r->d[2] = player;
    if (!player) { /* 0x0CC342–0x0CC34A */
        r->d[0] = set_word(r->d[0], 0x80u);
        r->a[1] = UINT32_MAX;
        return;
    }

    r->a[1] = player;
    uint16_t x = half_word((uint16_t)(rd16(m, player + 0x2Cu) + rd16(m, player + 0x2Eu)));
    uint16_t dx = (uint16_t)(x + rd16(m, player + 0x14u) - rd16(m, obj + 0x14u) - (uint16_t)r->d[0]);
    uint16_t y = half_word((uint16_t)(rd16(m, player + 0x30u) + rd16(m, player + 0x32u)));
    uint16_t dy = (uint16_t)(y + rd16(m, player + 0x18u) - rd16(m, obj + 0x18u) - (uint16_t)r->d[1]);
    r->d[2] = set_word(r->d[2], y);
    r->d[1] = set_word(r->d[1], dy);

    uint32_t sp = r->a[7];
    push_long(m, r, 0x0CC382u); /* 0x0CC37C JSR 0x006422 */
    push_long(m, r, r->a[0]); /* 0x006422 MOVEM.L D1-D3/A0,-(SP) */
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    /* 0x006474 EXG.L swaps the upper words as well when the signs differ. */
    uint32_t d0 = ((dx ^ dy) & 0x8000u) ? r->d[1] : r->d[0];
    r->d[0] = set_word(d0, Obj_Atan2(m, dx, dy));
    r->a[7] = sp;
}

int Fn_0D58BE(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x0D58C4u);
    int blocked = nearby(m, r);
    r->a[7] += 4u;
    if (blocked) /* 0x0D58C6 */
        goto done;

    push_long(m, r, 0x0D58D0u);
    blocked = offscreen(m, r);
    r->a[7] += 4u;
    if (blocked) /* 0x0D58D2 */
        goto done;

    r->d[1] = set_word(r->d[1], 2u);
    push_long(m, r, 0x0D58E0u);
    blocked = fire_blocked(m, r);
    r->a[7] += 4u;
    if (blocked) /* 0x0D58E2 */
        goto done;

    r->d[0] = set_word(r->d[0], 0x10u);
    r->d[1] = set_word(r->d[1], 0x10u);
    push_long(m, r, 0x0D58F4u);
    aim(m, r);
    r->a[7] += 4u;
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] << 8));
    r->d[1] = 0x10100000u | (uint16_t)r->d[0];
    call_spawn(m, r, 0x0D5914u, 0x0D901Au, 9u, 2u, r->d[1]); /* 0x0D58FE–0x0D5914 */

done:
    r->a[7] += 4u; /* 0x0D5918 RTS */
    return 0;
}
