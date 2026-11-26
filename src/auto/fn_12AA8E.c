/* ROM 0x12AA8E: multi-segment dragon/serpent body follower tick (A0 = object,
 * +0x08 = parent, +0x44 = previous segment). Frees self (0x0105EC) when
 * parent->+0x59 != 0. Otherwise snaps position to prev segment, updates
 * heading/offsets (+0x20/+0x0C/+0x0E) via 0x12ABFC (segments 0 and 7) or
 * propagates angle + A5+0x327A/327C via 0x12B3F4, moves via 0x0CBE12, sets
 * directional sprite/hitbox via 0x12B41E, pushes collision/flash via
 * 0x11B06E, copies grandparent +0x1D into +0x4F, runs hit-reflect/spawn logic
 * (0x12AB7C and 0x12C126) for segments 7..16, and re-arms 0x12AA8E. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x12AB7C: reflect player shot (+0x36) for segments 7..16 */
static void sub_12AB7C(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x36u);                  /* 0x12AB80 */
    if (d0 == 0u)                                        /* 0x12AB84 */
        return;
    wr32(m, obj + 0x36u, 0u);                            /* 0x12AB88 */
    if (rd8(m, obj + 0x35u) == 2u)                       /* 0x12AB8C..0x12AB92 */
        return;

    uint32_t a1 = d0;                                    /* 0x12AB96 */
    uint8_t k = rd8(m, a1 + 0x22u);                      /* 0x12AB98..0x12ABBC */
    if (k == 0u || k == 4u || k == 5u || k == 6u)
        return;

    int32_t hp = (int32_t)rd32(m, a1 + 0x48u);           /* 0x12ABC0 */
    if (hp <= 0)                                         /* 0x12ABC4 */
        return;

    hp = -hp;                                            /* 0x12ABC8 */
    hp >>= 3;                                            /* 0x12ABCA asr.l #3,D0 */
    wr32(m, a1 + 0x48u, (uint32_t)hp);                   /* 0x12ABCC */

    uint32_t rng = Rng_NextMem(m);                       /* 0x12ABD0 */
    wr32(m, a1 + 0x4Cu, (uint32_t)(int32_t)(int16_t)(uint16_t)rng); /* 0x12ABD6..0x12ABD8 */
    wr8(m, a1 + 0x1Cu, (uint8_t)(rd8(m, a1 + 0x1Cu) | 0x01u)); /* 0x12ABDC */
    Sound_PlayHooked(m, 0x000Fu);                        /* 0x12ABE2..0x12ABF2 */
}

/* ROM 0x12ABFC: ease +0x20 (>>3) and +0x0C/+0x0E (>>2) toward (d0, d1, d2) */
static void sub_12ABFC(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint16_t d2w)
{
    int16_t d0 = (int16_t)((uint16_t)*d0_io - rd16(m, obj + 0x20u));
    d0 = (int16_t)(d0 >> 3);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)d0));
    *d0_io = setw(*d0_io, (uint16_t)d0);

    int16_t d1 = (int16_t)((uint16_t)*d1_io - rd16(m, obj + 0x0Cu));
    d1 = (int16_t)(d1 >> 2);
    wr16(m, obj + 0x0Cu, (uint16_t)(rd16(m, obj + 0x0Cu) + (uint16_t)d1));
    *d1_io = setw(*d1_io, (uint16_t)d1);

    int16_t d2 = (int16_t)(d2w - rd16(m, obj + 0x0Eu));
    d2 = (int16_t)(d2 >> 2);
    wr16(m, obj + 0x0Eu, (uint16_t)(rd16(m, obj + 0x0Eu) + (uint16_t)d2));
}

/* ROM 0x12B3E2: copy +0x14/+0x18 from previous segment at +0x44 */
static void sub_12B3E2(const f3mem_t *m, uint32_t obj, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    *a1_out = a1;
}

/* ROM 0x12B3F4: propagate heading + A5+0x327A from prev segment (+0x44) */
static void sub_12B3F4(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, obj + 0x44u);                  /* 0x12B3F4 */
    uint16_t d1w = rd16(m, obj + 0x20u);                 /* 0x12B3F8 */
    uint16_t d0w = rd16(m, a1 + 0x20u);                  /* 0x12B3FC */
    d0w = (uint16_t)(d0w - d1w);                         /* 0x12B400 */
    d0w = (uint16_t)(d0w + rd16(m, F3_A5 + 0x327Au));    /* 0x12B402 */
    int32_t num = (int32_t)(int16_t)d0w;                 /* 0x12B406 ext.l */
    int16_t q = (int16_t)(num / 8);                      /* 0x12B408 divs.w #8,D0 */
    int16_t rem = (int16_t)(num % 8);
    d0w = (uint16_t)((uint16_t)q + d1w);                 /* 0x12B40C */
    wr16(m, obj + 0x20u, d0w);                           /* 0x12B40E */
    d0w = (uint16_t)(d0w >> 8);                          /* 0x12B412 */
    *d0_io = ((uint32_t)(uint16_t)rem << 16) | d0w;
    d1w = rd16(m, F3_A5 + 0x327Cu);                      /* 0x12B414 */
    wr16(m, F3_A5 + 0x327Au, (uint16_t)(rd16(m, F3_A5 + 0x327Au) + d1w)); /* 0x12B418 */
    *d1_io = setw(*d1_io, d1w);
    *a1_out = a1;
}

/* ROM 0x12B41E: set sprite, flip, hitboxes and draw offset from +0x20 angle */
static void sub_12B41E(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x20u) >> 8); /* 0x12B41E..0x12B422 */
    d0w = (uint16_t)((d0w & 0xFF00u) | (uint8_t)(d0w + 4u)); /* 0x12B424 addq.b #4 */
    d0w >>= 3;                                           /* 0x12B426 */
    uint32_t d1 = (d0w & 0x0010u) ? 0xFFFFFFFFu : 0u;    /* 0x12B428 bfexts d0{27:1},d1 */
    d0w &= 0x000Fu;                                      /* 0x12B42C */

    uint32_t a1 = rd32(m, obj + 0x3Au);                  /* 0x12B430 */
    uint32_t a2 = rd32(m, a1); a1 += 4u;                 /* 0x12B434 */
    a1 += (uint32_t)(int32_t)(int16_t)d0w * 8u;          /* 0x12B436 */
    a2 += (uint32_t)(int32_t)(int16_t)d0w * 4u;          /* 0x12B43A */

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++)); /* 0x12B43E..0x12B442 */
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++)); /* 0x12B446..0x12B44A */
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++)); /* 0x12B44E..0x12B452 */
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++)); /* 0x12B456..0x12B45A */

    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;         /* 0x12B45E */
    wr8(m, obj + 0x1Cu, rd8(m, a1++));                   /* 0x12B462 */
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12B466..0x12B46A */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);       /* 0x12B46E..0x12B470 */
    wr16(m, obj + 0x52u, d0w);                           /* 0x12B472 */

    if ((uint16_t)d1 != 0u) {                            /* 0x12B476..0x12B478 */
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x03u)); /* 0x12B47C */
        wr16(m, obj + 0x50u, (uint16_t)(-(int16_t)rd16(m, obj + 0x50u))); /* 0x12B482 */
        wr16(m, obj + 0x52u, (uint16_t)(-(int16_t)rd16(m, obj + 0x52u))); /* 0x12B486 */
    }

    *d0_io = setw(*d0_io, d0w);
    *d1_io = d1;
    *a1_out = a1;
}

int Fn_12AA8E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12AA8E */

    uint8_t flag = rd8(m, a1 + 0x59u);                   /* 0x12AA92 */
    wr8(m, obj + 0x59u, flag);
    if (flag != 0u) {                                    /* 0x12AA98 */
        Obj_Free(m, obj);                                /* 0x12AB70 (preserves D0,A0,A1) */
        r->a[1] = a1;
        return 0;
    }

    sub_12B3E2(m, obj, &a1);                             /* 0x12AA9C */
    d1 = setw(d1, rd16(m, obj + 0x22u));                 /* 0x12AAA0 */

    if ((uint16_t)d1 == 0u) {                            /* 0x12AAA4..0x12AAA8 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u)); /* 0x12AAAC */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u)); /* 0x12AAB0 */
        d0 = setw(d0, rd16(m, F3_A5 + 0x3282u));         /* 0x12AAB4 */
        d1 = setw(d1, rd16(m, F3_A5 + 0x3284u));         /* 0x12AAB8 */
        uint16_t d2w = rd16(m, F3_A5 + 0x3286u);         /* 0x12AABC */
        sub_12ABFC(m, obj, &d0, &d1, d2w);               /* 0x12AAF4 */
        wr16(m, F3_A5 + 0x327Au, rd16(m, obj + 0x0Cu));  /* 0x12AAF8 */
        wr16(m, F3_A5 + 0x327Cu, rd16(m, obj + 0x0Eu));  /* 0x12AAFE */
        d0 = setw(d0, (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x12AB04..0x12AB08 */
    } else if ((uint16_t)d1 == 7u) {                     /* 0x12AAC4..0x12AAC8 */
        d0 = setw(d0, rd16(m, F3_A5 + 0x3288u));         /* 0x12AAD8 */
        d1 = setw(d1, rd16(m, F3_A5 + 0x328Au));         /* 0x12AADC */
        uint16_t d2w = rd16(m, F3_A5 + 0x328Cu);         /* 0x12AAE0 */
        wr32(m, F3_A5 + 0x328Eu, obj);                   /* 0x12AAE4 */
        sub_12ABFC(m, obj, &d0, &d1, d2w);               /* 0x12AAF4 */
        wr16(m, F3_A5 + 0x327Au, rd16(m, obj + 0x0Cu));  /* 0x12AAF8 */
        wr16(m, F3_A5 + 0x327Cu, rd16(m, obj + 0x0Eu));  /* 0x12AAFE */
        d0 = setw(d0, (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x12AB04..0x12AB08 */
    } else {
        sub_12B3F4(m, obj, &d0, &d1, &a1);               /* 0x12AAEC */
    }

    Obj_MoveByAngle(m, obj, (uint16_t)d0);               /* 0x12AB0A */
    sub_12B41E(m, obj, &d0, &d1, &a1);                   /* 0x12AB10 */
    wr8(m, obj + 0x35u, 0u);                             /* 0x12AB14 */
    Obj_11B06E(m, obj);                                  /* 0x12AB18 */

    a1 = rd32(m, obj + 0x08u);                           /* 0x12AB1E */
    a1 = rd32(m, a1 + 0x08u);                            /* 0x12AB22 */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));             /* 0x12AB26 */

    {
        int16_t seg = (int16_t)rd16(m, obj + 0x22u);
        if (seg >= 7 && seg <= 0x10) {                   /* 0x12AB2C..0x12AB3C */
            sub_12AB7C(m, obj);                          /* 0x12AB40 */
            if ((rd8(m, F3_A5 + 0x30EFu) & 0x04u) != 0u) /* 0x12AB44 bftst (0x30ee,A5){13:1} */
                d0 = Obj_Spawn(m, obj, 0x0012C126u, 5u, 2u, rd32(m, obj + 0x3Au)); /* 0x12AB4E..0x12AB66 */
        }
    }

    wr32(m, obj + 0x00u, 0x0012AA8Eu);                   /* 0x12AB6A -> 0x12AA86 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
