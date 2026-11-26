/* ROM 0x12001C: multi-part enemy tick (A0 = object, +0x08 = parent).
 * Frees self (0x0105EC) if parent->+0x59 != 0. When A5-0x144B bit 4 is set
 * and (Rng_NextMem & 0xFF) == 0, aims at player/scroll target (0x006422) and
 * switches to 0x120174. When +0x24 <= 0, decrements A5+0x3260, adds 0x12C5F
 * to A5+0x886, awards +0x4C to score receiver (0x0CBDDE), plays sound 0x16,
 * spawns explosion 0x13E4EE (0x13E4D0 + 0x1057C), and switches to 0x120114.
 * Otherwise loads pose/hitbox via 0x1201E0/0x12048C (table +0x0C[A5+0x3262]
 * or 0x0CBD52 animation), saves +0x48 = A1+4, optionally fires aimed bullet
 * 0x0CCD04 via 0x0CC2DE(0x10) and 0x0CC33A when A5+0x30EE bit 4 is set,
 * updates collision/flash (0x11B06E) and draws sprite (0x11B09A), then
 * re-arms 0x12001C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../enemy/obj_11B06E.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }
static uint32_t swap32(uint32_t v) { return (v << 16) | (v >> 16); }

/* ROM 0x0CC2DE: (Rng_NextMem & 0xFFFF) mod max(1, ((0x180 - A5+0x87E) * d0w) >> 8) */
static uint16_t sub_0CC2DE(const f3mem_t *m, uint16_t d0w, uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = swap32(d1);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t rng = Rng_NextMem(m) & 0x0000FFFFu;
    uint16_t div = (uint16_t)d1;
    uint16_t q = (uint16_t)(rng / div);
    uint16_t rem = (uint16_t)(rng % div);
    *d0_out = ((uint32_t)rem << 16) | q;
    *d1_out = d1;
    return rem;
}

/* ROM 0x0CC33A: aim angle toward player at A5+0x1D28 (or 0x80 if null) */
static void sub_0CC33A(const f3mem_t *m, uint32_t obj, int16_t d0w, int16_t d1w,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t player = rd32(m, F3_A5 + 0x1D28u);        /* 0x0CC33A */
    if (player == 0u) {
        *d0_io = setw(*d0_io, 0x0080u);                  /* 0x0CC342 */
        *a1_out = 0xFFFFFFFFu;                           /* 0x0CC346 */
        return;
    }
    *a1_out = player;                                    /* 0x0CC34E */
    int16_t hx = (int16_t)((int16_t)rd16(m, player + 0x2Cu) + (int16_t)rd16(m, player + 0x2Eu));
    hx = (hx < 0) ? (int16_t)(~((uint16_t)(~hx) >> 1)) : (int16_t)((uint16_t)hx >> 1);
    int16_t dx = (int16_t)(-d0w + hx + (int16_t)rd16(m, player + 0x14u) - (int16_t)rd16(m, obj + 0x14u));

    int16_t hy = (int16_t)((int16_t)rd16(m, player + 0x30u) + (int16_t)rd16(m, player + 0x32u));
    hy = (hy < 0) ? (int16_t)(~((uint16_t)(~hy) >> 1)) : (int16_t)((uint16_t)hy >> 1);
    int16_t dy = (int16_t)(-d1w + hy + (int16_t)rd16(m, player + 0x18u) - (int16_t)rd16(m, obj + 0x18u));

    uint16_t ang = Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy);
    *d0_io = setw(*d0_io, ang);
    *d1_io = setw(*d1_io, (uint16_t)dy);
}

/* ROM 0x12048C: unpack pose/hitbox record at a1 and return advanced a1 */
static uint32_t sub_12048C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12048C..0x120490 */
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x120494..0x120498 */
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;                   /* 0x12049C */
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x1204A0..0x1204A4 */
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x1204A8..0x1204AC */
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x1204B0..0x1204B4 */
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x1204B8..0x1204BC */
    return a1;
}

/* ROM 0x1201E0: select pose record and apply via 0x12048C */
static uint32_t sub_1201E0(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *d1_io)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x1201E0 */
    uint8_t idx = rd8(m, F3_A5 + 0x3262u);               /* 0x1201E6 */
    *d0_io = (uint32_t)idx;
    if (idx != 0u) {                                     /* 0x1201EA */
        a1 = rd32(m, a1 + (uint32_t)idx * 4u - 4u);      /* 0x1201EE */
    } else {
        a1 += 0x10u;                                     /* 0x1201F6 */
        Obj_Animate(m, obj, a1);                         /* 0x1201FA */
        *d0_io = setw(*d0_io, rd16(m, obj + 0x3Au));
        *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
        a1 = rd32(m, obj + 0x10u);                       /* 0x120200 */
    }
    return sub_12048C(m, obj, a1);                       /* 0x120204 */
}

/* ROM 0x11B09A with exact D0/D1/A1 register outputs */
static void sub_11B09A_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, y);
    *d0_io = setw(*d0_io, y);

    if ((int16_t)y < 0x00E8 && (int16_t)x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t w = (uint16_t)(rd16(m, a1) << 4);
        a1 += 2u;
        *d0_io = setw(*d0_io, w);
        *a1_io = a1;
        if ((int16_t)(w + y) > 0) {
            uint16_t h = (uint16_t)(rd16(m, a1) << 4);
            a1 += 2u;
            *d0_io = setw(*d0_io, h);
            *a1_io = a1;
            if ((int16_t)(h + x) > 0) {
                Obj_DrawSprite(m, obj);
                uint32_t a2 = rd32(m, obj + 0x10u);
                *a1_io = rd32(m, SPR_CURSOR);
                if (rd32(m, a2) == 0x00010001u) {
                    uint32_t d0 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
                    d0 &= 0x0FFF0FFFu;
                    d0 = setw(d0, rd16(m, a2 + 6u));
                    if (rd8(m, obj + 0x1Du) != 0u)
                        d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
                    *d0_io = d0;
                    *d1_io = setw(*d1_io, (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u));
                } else {
                    *d0_io = 0x0000FFFFu;
                    *d1_io = ((uint32_t)(uint16_t)(rd16(m, a2) - 1u) << 16) | 0xFFFFu;
                }
            }
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
    *d0_io = setw(*d0_io, offy);
}

int Fn_12001C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12001C */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x120020..0x120024 */
        Obj_Free(m, obj);                                /* 0x1201D4 (preserves D0,A0,A1) */
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {       /* 0x120028..0x12002E */
        d0 = Rng_NextMem(m);                             /* 0x120032 */
        d0 = setw(d0, (uint16_t)(d0 & 0x00FFu));         /* 0x120038 */
        if ((uint16_t)d0 == 0u) {                        /* 0x12003C */
            uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, F3_A5 + 0x30D2u));
            uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, F3_A5 + 0x30D6u));
            uint16_t ang = Obj_Atan2(m, dx, dy);         /* 0x120158 */
            wr16(m, obj + 0x20u, ang);                   /* 0x12015E */
            wr16(m, obj + 0x42u, 0x0018u);               /* 0x120162 */
            wr8(m, obj + 0x1Du, 0u);                     /* 0x120168 */
            wr32(m, obj + 0x00u, 0x00120174u);           /* 0x12016C */
            r->d[0] = setw(d0, ang);
            r->d[1] = setw(d1, dy);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x120040..0x120044 */
        wr8(m, F3_A5 + 0x3260u, (uint8_t)(rd8(m, F3_A5 + 0x3260u) - 1u)); /* 0x1200B4 */
        wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x1200B8 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x1200C0 */
        a1 = Obj_0CBDDE(m, obj);                         /* 0x1200C6 */
        d0 = rd16(m, obj + 0x4Cu);                       /* 0x1200CC..0x1200CE */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);   /* 0x1200D2 */
        Sound_PlayHooked(m, 0x0016u);                    /* 0x1200D6..0x1200E6 */
        d0 = Obj_13E4D0(m, obj, 3u);                     /* 0x1200EA..0x1200EC */
        d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0); /* 0x1200F2..0x120108 */
        wr32(m, obj + 0x00u, 0x00120114u);               /* 0x12010C */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = sub_1201E0(m, obj, &d0, &d1);                   /* 0x120048 */
    a1 += 4u;                                            /* 0x12004C */
    wr32(m, obj + 0x48u, a1);                            /* 0x120050 */

    /* 0x120054: bftst (0x30EE,A5){0x4:0x1} -> bit 3 of byte (0x80 >> 4 = 0x08) */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x08u) != 0u) {
        if (sub_0CC2DE(m, 0x0010u, &d0, &d1) == 0u) {    /* 0x120062 */
            uint32_t d2 = rd32(m, a1);                   /* 0x12006E */
            int16_t ox = (int8_t)rd8(m, a1++);           /* 0x120074..0x120076 */
            int16_t oy = (int8_t)rd8(m, a1++);           /* 0x120078..0x12007A */
            sub_0CC33A(m, obj, ox, oy, &d0, &d1, &a1);   /* 0x12007C */
            uint16_t ang = (uint16_t)((uint16_t)d0 << 8);/* 0x120086 */
            d0 = setw(d0, ang);
            d2 = setw(d2, ang);                          /* 0x120088 */
            d0 = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d2); /* 0x12009A */
        }
    }

    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
        a1 = rd32(m, F3_A5 + 0x0906u) + 4u;
    Obj_11B06E_Tick(m, obj);                             /* 0x1200A4 */
    sub_11B09A_regs(m, obj, &d0, &d1, &a1);              /* 0x1200AA */
    wr32(m, obj + 0x00u, 0x0012001Cu);                   /* 0x1200B0 -> 0x120014 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
