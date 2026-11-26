/* ROM 0x12F0A8: multi-segment articulated appendage/boss-arm tick (A0 = obj).
 * Calls 0x12F356 to sync from parent (+0x0C), draw 4 articulated segments
 * along (+0x4A..) via 0x0CBE12 + 0x001282, draw head/core via 0x12F47C
 * (when +0x48 <= 0x20) or mode dispatch 0x12F494[A5+0x329E] (0x12F4A0/
 * 0x12F53A/0x12F5C6), and draw base sprite 0x315A8. Then steps +0x46/+0x44
 * phase, eases (+0x4A..) angles toward 0x12F22E, and branches to 0x12F08E
 * (free on A5+0x30F2 bit 0, re-arm 0x12F0A8 while +0x48 >= 0, else step
 * 0x12F1D8 via 0x0CBD24 and transition to 0x12EF40 or 0x12F13C). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_out = rd32(m, F3_A5 - 0x0CA0u);

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

/* ROM 0x0CC2BC: D0 = (rem << 16) | quot, D1 = divisor. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                           uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1 = swap32(d1);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint16_t div = (uint16_t)d1;
    uint16_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t q = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    *d0_out = ((uint32_t)q << 16) | rem;
    *d1_out = d1;
    return rem;
}

/* ROM 0x12F4A0: mode 0 (laser/beam emitter) */
static void sub_12F4A0(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x000Cu));
    wr32(m, obj + 0x10u, 0x000315C4u);
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
    wr32(m, obj + 0x10u, 0x000315BCu);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
    *d0_io = Rng_NextMem(m);
    if (rd16(m, obj + 0x3Au) == 0u) {
        uint16_t t = rd16(m, obj + 0x3Eu);
        *d0_io = setw(*d0_io, t);
        if (t == 0u) {
            Sound_PlayHooked(m, 0x0027u);
            wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));
        }
        *d0_io = setw(*d0_io, t & 3u);
        if ((t & 3u) == 0u)
            *d0_io = Obj_Spawn(m, obj, 0x00130EE2u, 9u, 2u, 0xE40E8000u);
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0x003C) {
        wr16(m, obj + 0x3Au, (uint16_t)~rd16(m, obj + 0x3Au));
        wr16(m, obj + 0x3Eu, 0u);
    }
}

/* ROM 0x12F53A: mode 1 (spread shot) */
static void sub_12F53A(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Cu));
    Obj_Animate(m, obj, 0x0012F5ACu);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
    if (sub_0CC2BC(m, 4u, d0_io, d1_io) == 0u) {
        uint32_t d0 = Rng_NextMem(m);
        d0 = setw(d0, (uint16_t)((d0 & 0x1F00u) + 0x7000u));
        uint32_t d1 = d0 | 0xE4000000u;
        d0 = Rng_NextMem(m);
        uint16_t d0w = (uint16_t)((d0 & 7u) + 0x000Au);
        d1 = (d1 & 0xFF00FFFFu) | ((uint32_t)(d0w & 0xFFu) << 16);
        *d1_io = d1;
        *d0_io = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);
    }
}

/* ROM 0x12F5C6: mode 2 (burst barrage) */
static void sub_12F5C6(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    static const uint32_t shots[6] = {
        0xECFE8000u, 0xEC1D8000u, 0xF1067000u,
        0xF1169000u, 0xF50D6000u, 0xF50FA000u
    };
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x000Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Cu));

    for (;;) {
        *a1_io = 0x0012F6DCu;
        Obj_AnimFrameStep(m, obj, 0x0012F6DCu);
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        *d0_io = setw(*d0_io, (uint16_t)off);
        *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
        int16_t dur = (int16_t)rd16(m, 0x0012F6DCu + (uint32_t)(int32_t)off);
        if (dur > 0)
            break;
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
        wr32(m, obj + 0x10u, rd32(m, 0x0012F6DCu + (uint32_t)(int32_t)(off - 4)));
        if (sub_0CC2BC(m, 0x000Au, d0_io, d1_io) == 0u) {
            for (unsigned i = 0; i < 6u; i++)
                *d0_io = Obj_Spawn(m, obj, 0x000CCF72u, 5u, 2u, shots[i]);
            Sound_PlayHooked(m, 0x003Bu);
        }
        break;
    }

    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
}

/* ROM 0x12F242 + 0x12FF36: inherit parent coords & palette */
static void sub_12F242(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x1Cu, rd8(m, a1 + 0x1Cu));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x1Eu));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr8(m, obj + 0x4Fu, 0x40u);

    uint32_t p = rd32(m, F3_A5 + 0x32AAu);
    uint8_t c = rd8(m, p + 0x1Du);
    wr8(m, obj + 0x1Du, (c == rd8(m, p + 0x4Fu)) ? rd8(m, obj + 0x4Fu) : c);
}

/* ROM 0x12F356: draw 4 articulated segments + weapon head + base sprite */
static void sub_12F356(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    sub_12F242(m, obj);
    wr8(m, obj + 0x1Cu, 0u);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u)
        + ((rd8(m, a1 + 0x1Cu) & 1u) ? 0x0046u : 0x0019u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0078u));

    uint16_t d1w = rd8(m, obj + 0x44u);
    a1 = obj + 0x4Au;
    for (int d0w = 3; d0w >= 0; d0w--) {
        int16_t idx = (d0w & 1) ? (int16_t)(7 - (int16_t)d1w) : (int16_t)d1w;
        wr32(m, obj + 0x10u, rd32(m, 0x0012F45Cu + (uint32_t)(int32_t)idx * 4u));
        uint16_t ang = (uint16_t)(rd16(m, a1) >> 8);
        wr16(m, obj + 0x42u, rd16(m, a1 + 2u));
        Obj_MoveByAngle(m, obj, ang);
        Obj_DrawSprite(m, obj);
        a1 += 4u;
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x42u, 0x0044u);
    Obj_MoveByAngle(m, obj, 0x00C9u);

    uint16_t save_x = rd16(m, obj + 0x14u);
    uint16_t save_y = rd16(m, obj + 0x18u);
    int16_t mode = (int16_t)rd16(m, F3_A5 + 0x329Eu);

    if ((int16_t)rd16(m, obj + 0x48u) <= 0x0020) {
        uint32_t tbl = 0x0012F47Cu + (uint32_t)(int32_t)mode * 8u;
        int16_t rem = (int16_t)(rd16(m, obj + 0x48u) - 3u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)rem));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
        wr32(m, obj + 0x10u, rd32(m, tbl));
        Obj_DrawSprite(m, obj);
        if (rem > 0x0D) {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
            wr32(m, obj + 0x10u, rd32(m, tbl + 4u));
            Obj_DrawSprite(m, obj);
        }
    } else {
        uint32_t fn = rd32(m, 0x0012F494u + (uint32_t)(int32_t)mode * 4u);
        switch (fn) {
        case 0x0012F4A0u: sub_12F4A0(m, obj, &r->d[0], &r->d[1], &r->a[1]); break;
        case 0x0012F53Au: sub_12F53A(m, obj, &r->d[0], &r->d[1], &r->a[1]); break;
        case 0x0012F5C6u: sub_12F5C6(m, obj, &r->d[0], &r->d[1], &r->a[1]); break;
        default: break;
        }
    }

    wr16(m, obj + 0x18u, save_y);
    wr16(m, obj + 0x14u, save_x);
    wr32(m, obj + 0x10u, 0x000315A8u);
    draw_sprite_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);
}

int Fn_12F0A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_12F356(m, r);                                    /* 0x12F0A8 */

    int16_t d0w = (int16_t)rd16(m, obj + 0x46u);         /* 0x12F0AC */
    if (d0w > 0)                                         /* 0x12F0B0..0x12F0B4 */
        wr16(m, obj + 0x46u, (uint16_t)(d0w - 4));       /* 0x12F0B8 */

    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + (uint16_t)d0w)); /* 0x12F0BC */
    wr8(m, obj + 0x44u, (uint8_t)(rd8(m, obj + 0x44u) & 7u)); /* 0x12F0C0 */

    {
        uint32_t a1 = obj + 0x4Au;                       /* 0x12F0C6 */
        uint32_t a2 = 0x0012F22Eu;                       /* 0x12F0CA */
        uint32_t a3 = rd32(m, obj + 0x0Cu);              /* 0x12F0D0 */
        int16_t d1w = 0;

        for (int i = 3; i >= 0; i--) {                   /* 0x12F0D4..0x12F0F8 */
            d1w = (int16_t)rd16(m, a2);
            if ((rd8(m, a3 + 0x1Cu) & 1u) != 0u)         /* 0x12F0DA */
                d1w = (int16_t)(0x4000 - (d1w - 0x4000));
            d1w = (int16_t)(d1w - (int16_t)rd16(m, a1)); /* 0x12F0EE */
            d1w = (int16_t)(d1w >> 4);                   /* 0x12F0F0 */
            wr16(m, a1, (uint16_t)(rd16(m, a1) + d1w));  /* 0x12F0F2 */
            a1 += 4u;
            a2 += 4u;
        }
        r->d[0] = setw(r->d[0], 0xFFFFu);
        r->d[1] = setw(r->d[1], (uint16_t)d1w);
        r->a[1] = a1;
        r->a[2] = a2;
        r->a[3] = a3;
    }

    /* 0x12F0FC -> 0x12F08E */
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u) {          /* 0x12F08E */
        uint32_t a1 = rd32(m, obj + 0x0Cu);              /* 0x12F160 */
        wr8(m, a1 + 0x3Eu, 0u);
        r->a[1] = a1;
        Obj_Free(m, obj);
        return 0;
    }

    {
        int16_t timer = (int16_t)(rd16(m, obj + 0x48u) - 1u); /* 0x12F098 */
        wr16(m, obj + 0x48u, (uint16_t)timer);
        if (timer >= 0) {                                /* 0x12F09C */
            wr32(m, obj, 0x0012F0A8u);                   /* 0x12F0A0 */
            return 0;
        }
    }

    /* 0x12F0FE */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    if ((rd8(m, F3_A5 + 0x30F2u) & 1u) != 0u) {          /* 0x12F106 */
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        wr8(m, a1 + 0x3Eu, 0u);
        r->a[1] = a1;
        Obj_Free(m, obj);
        return 0;
    }

    Obj_AnimFrameStep(m, obj, 0x0012F1D8u);              /* 0x12F116 */
    {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        int16_t dur = (int16_t)rd16(m, 0x0012F1D8u + (uint32_t)(int32_t)off);
        r->d[0] = setw(r->d[0], (uint16_t)off);
        if (dur > 0)
            r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));

        uint32_t a1 = rd32(m, obj + 0x0Cu);
        r->a[1] = a1;
        if (dur <= 0) {                                  /* 0x12F120 */
            wr8(m, a1 + 0x3Eu, 0u);                      /* 0x12F158 */
            wr32(m, obj, 0x0012EF40u);                   /* 0x12EF38 */
            return 0;
        }
        wr8(m, a1 + 0x3Eu, (rd32(m, obj + 0x10u) != 0x00028740u) ? 0xFFu : 0u); /* 0x12F128..0x12F130 */
        wr32(m, obj, 0x0012F13Cu);                       /* 0x12F134 */
    }
    return 0;
}
