/* ROM 0x0D63A8: late-phase enemy tick (A0 = object; the routine body starts at
 * 0x0D62C8, the entry 0x0D63A8 is a `bra.w` into it and the re-arm value).
 * HP (+0x24) <= 0 or bomb-clear (A5-0x144B bit 5) -> kill: credit +0x4C to the
 * +0x36 receiver via 0x0CBDDE, play hooked sound +0x65, spawn 0x13E4EE (packed
 * sprite centre from 0x13E4D0) and park the fn at the free trampoline 0x0D63FE.
 * Otherwise step animation table 0x0D6464 (0x0CBD24); while it still has a
 * duration, only run the common tail. When the animation ended: bomb check
 * (A5+0x1D58 bit 1 and -0x0A <= X <= 0x11C) fires 0x05D84C into the sprite
 * pointer and relays +0x4C before jumping to the explosion tick 0x0CBB2C;
 * else the 0x0CC2BC gate (D0 = 8) may spawn two 0xD9056 escorts (group 9,
 * count 2) and restart the animation. Tail: sub-scroll velocity (0x0CBE00),
 * collision lists (0x0CBDA4), blink colour (0x0CBE34), sprite (0x001282), then
 * free off-screen (X outside [-0x20,0x140], cmp2.w 0x0CC112) or re-arm
 * 0x0D63A8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"       /* Sound_PlayHooked */
#include "../objects/obj_common.h"    /* Obj_AnimFrameStep */
#include "../objects/obj_pool.h"      /* Obj_Spawn, Obj_Free */
#include "../objects/obj_shared.h"    /* Obj_BlinkColour, Obj_ExplodeTick */
#include "../objects/obj_shared_ext.h"/* Obj_0CBDDE, Obj_13E4D0 */
#include "../player/player_move.h"   /* Obj_SubScrollVel */
#include "../video/sprite.h"          /* Obj_DrawSprite */

#define TBL_0D6464 0x000D6464u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CC2BC: stage-speed-scaled modulo tick gate. D0.w = scale, D1.w =
 * 0x180 - (A5+0x87E); divu of (A5+0x878). Returns the divu/swap halves. */
static void gate_0CC2BC(const f3mem_t *m, uint16_t d0, uint16_t *rem_out, uint16_t *quot_out)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0 * (uint32_t)d1;
    uint16_t div = (uint16_t)((prod << 8) >> 16);       /* lsl.l #8 / swap */
    if (div == 0u)
        div = 1u;                                       /* tst.w D1w / moveq #1 */
    uint32_t num = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    *quot_out = (uint16_t)(num / div);                  /* divu.w: quotient low */
    *rem_out = (uint16_t)(num % div);                   /* ... remainder high */
}

/* ROM 0x0CBDA4: append obj to the A5+0x8FE list and the hazard list A5+0x906;
 * leaves A1 = the incremented hazard pointer. */
static uint32_t add_coll_regs(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    return a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x00649A with the full 32-bit D0 result (D1 is preserved there). */
static uint32_t dist_full(const f3mem_t *m, uint16_t dx_in, uint16_t dy_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)dx_in * (int32_t)(int16_t)dx_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)dy_in * (int32_t)(int16_t)dy_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    return (d0 << 16) | (d0 >> 16);
}


/* ROM 0x0CBB2C with register tracking, reached from 0x0D6414 (jmp): orient
 * the sprite along {+0x10}[0..1] scaled by the zoom bytes, distance/angle
 * polar re-placement, and three exits: too close (0x0CBC80, D0.w = dist - R),
 * bomb bit clear (0x0CBC80, D0.w = angle << 8), or the full draw which leaves
 * the 0x001282 sprite registers (0x0CBC70 re-arms 0x0CBC78). */
static void explode_tick_regs(const f3mem_t *m, uint32_t obj,
                              uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d1_in = *d1_io;
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);   /* 0x0CBB7C */
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);   /* 0x0CBB84 */

    uint32_t a1 = rd32(m, obj + 0x10u);                        /* 0x0CBB54 */
    int16_t off_y = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1)) >> 5;
    int16_t off_x = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1 + 2u)) >> 5;
    dx = (uint16_t)(dx - (uint16_t)off_x);
    dy = (uint16_t)(dy - (uint16_t)off_y);

    uint32_t d0_dist = dist_full(m, dx, dy);                   /* 0x0CBB8C */
    uint16_t d0w = (uint16_t)((uint16_t)d0_dist << 2);         /* 0x0CBB92 */
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);                /* 0x0CBB94 */
    int is_lt = (int16_t)d0w < (int16_t)thresh;                /* blt 0x0CBC80 */
    uint16_t diff = (uint16_t)(d0w - thresh);
    uint16_t ang = Obj_Atan2(m, dx, dy);                       /* 0x0CBBB2 */
    /* 0xCBB7C..0xCBB88 built D0 = {d0_dist.h, dx} and D1 = {d1_in.h, dy}, so the
     * atan2's exg decides whose high word survives. */
    int atan_exg = (((int16_t)dx < 0) != ((int16_t)dy < 0));
    uint32_t ang_hi = atan_exg ? (d1_in & 0xFFFF0000u) : (d0_dist & 0xFFFF0000u);

    Obj_ExplodeTick(m, obj);              /* the same 0x0CBB2C body, memory only */

    if (is_lt) {                                               /* 0x0CBC80 */
        *d0_io = setw(d0_dist, diff);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {             /* 0x0CBBCA */
        *d0_io = (ang_hi & 0xFFFF0000u) | (uint16_t)(ang << 8);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    /* 0x0CBC1A..0x0CBC26 leave D1 = the new 16.16 X before 0x001282. */
    *d1_io = rd32(m, obj + 0x14u);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);             /* 0x0CBC6A */
}

/* ROM 0x0D63BC: hooked sound, 0x13E4EE burst object, park at 0x0D63FE. */
static uint32_t kill_enemy(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x65u);        /* 0x0D63BE moveq/move.b */
    Sound_PlayHooked(m, (uint16_t)d0);                  /* 0x0D63C4 / 0x0D63CA */
    d0 = Obj_13E4D0(m, obj, 2u);                        /* 0x0D63D6 moveq #2 */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);    /* 0x0D63EC */
    wr32(m, obj + 0x00u, 0x000D63FEu);                  /* 0x0D63F6 */
    return d0;
}

int Fn_0D63A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0D62C8 tst.l/ble */
        a1 = Obj_0CBDDE(m, obj);                        /* 0x0D63AC */
        d0 = (uint32_t)rd16(m, obj + 0x4Cu);            /* 0x0D63B2 moveq/move.w */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);  /* 0x0D63B8 */
        d0 = kill_enemy(m, obj);
        goto out;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x0D62D0 btst #5 */
        d0 = kill_enemy(m, obj);                        /* 0x0D62D6 */
        goto out;
    }

    Obj_AnimFrameStep(m, obj, TBL_0D6464);              /* 0x0D62E0 */
    a1 = TBL_0D6464;
    d0 = (uint32_t)rd16(m, obj + 0x3Au);                /* D0.w = idx on return */
    if ((int16_t)rd16(m, TBL_0D6464 + d0) > 0)          /* 0x0D62E6 tst/bgt */
        goto tail;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0D62EE btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {               /* 0x0D62F6..0x0D6304 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0D6306 */
            wr32(m, obj + 0x10u, 0x0005D84Cu);          /* 0x0D640C */
            a1 = rd32(m, F3_A5 + 0x1D50u);              /* 0x0CBD12 */
            d0 = (uint32_t)rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            explode_tick_regs(m, obj, &d0, &d1, &a1);   /* 0x0D6414 jmp 0x0CBB2C */
            goto out;
        }
    }

    {
        uint16_t rem, quot;
        gate_0CC2BC(m, 8u, &rem, &quot);               /* 0x0D6312..0x0D6316 */
        d0 = ((uint32_t)rem << 16) | (uint32_t)quot;    /* after the first swap */
        if (rem != 0u)                                  /* 0x0D631C tst/bne */
            goto tail;
        d0 = ((uint32_t)rem << 16) | (uint32_t)(quot & 7u); /* 0x0D6322 swap/andi */
        if ((int16_t)(quot & 7u) >= 4)                  /* 0x0D6328 cmpi/bge */
            goto tail;
        Obj_Spawn(m, obj, 0x000D9056u, 9u, 2u, 0x0B139B00u); /* 0x0D6346 */
        d0 = Obj_Spawn(m, obj, 0x000D9056u, 9u, 2u, 0x2513E500u); /* 0x0D6366 */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x0D6370 */
        wr16(m, obj + 0x3Eu, 0u);                       /* 0x0D6374 */
    }

tail:                                                   /* 0x0D6378 */
    Obj_SubScrollVel(m, obj);                           /* 0x0D6378 -> 0x0CBE00 */
    a1 = add_coll_regs(m, obj);                         /* 0x0D637E -> 0x0CBDA4 */
    Obj_BlinkColour(m, obj);                            /* 0x0D6384 -> 0x0CBE34 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0D638A -> 0x001282 */

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);      /* 0x0D6390 */
        d0 = setw(d0, (uint16_t)x);
        /* 0x0D6394 cmp2.w 0x0CC112,D0: bounds {lo,hi} = {-0x20, 0x140}. */
        int16_t lo = (int16_t)rd16(m, 0x000CC112u);
        int16_t hi = (int16_t)rd16(m, 0x000CC114u);
        if (x < lo || x > hi) {
            Obj_Free(m, obj);                           /* 0x0D639C -> 0x0D63FE */
            goto out;
        }
    }
    wr32(m, obj + 0x00u, 0x000D63A8u);                  /* 0x0D63A0 */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
