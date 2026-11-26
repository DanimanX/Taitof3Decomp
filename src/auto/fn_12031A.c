/* ROM 0x12031A: sub-part enemy tick (A0 = object).
 * Frees immediately when parent (+0x08)->+0x59 is non-zero. Otherwise checks
 * (-0x144B,A5) bit 4 for random state switch to 0x1203FE (+0x42=-0x20, +0x1D=0);
 * while +0x24 > 0, animates (+0x0C+4) via 0x0CBD52, loads hitbox from +0x10 via
 * 0x12049C, registers in collision lists and updates flash (+0x1D) via 0x11B06E,
 * draws sprite via 0x11B09A, and re-arms 0x12031A. When +0x24 <= 0, decrements
 * A5+0x3261, adds 0x12C5F to A5+0x0886, awards +0x4C to score receiver (0x0CBDDE),
 * plays hooked sound 0x16, spawns explosion 0x13E4EE (group 5, count 2), and
 * advances to 0x1203C0. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x12049C: unpack next sprite pointer and 4 signed hitbox bytes from A1. */
static uint32_t sub_12049C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint32_t d0 = *d0_io;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Cu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)d0);
    *d0_io = d0;
    return a1;
}

/* ROM 0x120474: animate from (+0x0C)+4 and load hitbox record from +0x10. */
static void sub_120474(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu) + 4u;              /* 0x120474..0x120478 */
    Obj_Animate(m, obj, a1);                              /* 0x12047C */
    *d0_io = setw(*d0_io, rd16(m, obj + 0x3Au));
    *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
    a1 = rd32(m, obj + 0x10u);                            /* 0x120482 */
    *a1_out = sub_12049C(m, obj, a1, d0_io);              /* 0x120486 */
}

/* ROM 0x11B06E: register in collision lists if active & hitbox, then Obj_11C32C. */
static void sub_11B06E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        *a1_io = p2 + 4u;
        wr32(m, F3_A5 + 0x0906u, *a1_io);
    }
    Obj_11C32C(m, obj);                                   /* 0x11B092 */
}

/* ROM 0x11B09A: compute screen position, draw sprite if visible, restore offset. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, d0w);                            /* 0x11B0A6 */
    d0w = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, d0w);                            /* 0x11B0B6 */
    r->d[0] = setw(r->d[0], d0w);

    if ((int16_t)rd16(m, obj + 0x18u) < 0x00E8 && (int16_t)rd16(m, obj + 0x14u) < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);               /* 0x11B0CE */
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->d[0] = setw(r->d[0], (uint16_t)h);
        if ((int16_t)(h + (int16_t)rd16(m, obj + 0x18u)) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->d[0] = setw(r->d[0], (uint16_t)w);
            r->a[1] = a1;
            if ((int16_t)(w + (int16_t)rd16(m, obj + 0x14u)) > 0)
                draw_sprite_regs(m, r);                   /* 0x11B0EA */
        } else {
            r->a[1] = a1;
        }
    }

    uint16_t sx = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - sx)); /* 0x11B0F4 */
    uint16_t sy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy)); /* 0x11B0FC */
    r->d[0] = setw(r->d[0], sy);
}

int Fn_12031A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                   /* 0x12031A */
    r->a[1] = a1;
    if (rd8(m, a1 + 0x59u) != 0u) {                       /* 0x12031E..0x120322 */
        Obj_Free(m, obj);                                 /* 0x12045A */
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {        /* 0x120326..0x12032C */
        uint32_t d0 = Rng_NextMem(m);                     /* 0x120330 */
        d0 = setw(d0, (uint16_t)(d0 & 0x007Fu));          /* 0x120336 */
        r->d[0] = d0;
        if ((uint16_t)d0 == 0u) {                         /* 0x12033A */
            wr16(m, obj + 0x42u, 0xFFE0u);                /* 0x1203EC */
            wr8(m, obj + 0x1Du, 0u);                      /* 0x1203F2 */
            wr32(m, obj, 0x001203FEu);                    /* 0x1203F6 */
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) > 0) {              /* 0x12033E..0x120342 */
        sub_120474(m, obj, &r->d[0], &r->d[1], &r->a[1]); /* 0x120346 */
        sub_11B06E(m, obj, &r->a[1]);                     /* 0x12034A */
        sub_11B09A(m, r);                                 /* 0x120350 */
        wr32(m, obj, 0x0012031Au);                        /* 0x120312 */
        return 0;
    }

    /* 0x120358..0x1203BE */
    wr8(m, F3_A5 + 0x3261u, (uint8_t)(rd8(m, F3_A5 + 0x3261u) - 1u));
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
    a1 = rd32(m, obj + 0x0Cu);                            /* 0x120364 */
    wr32(m, obj + 0x10u, rd32(m, a1));                    /* 0x120368 */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));             /* 0x12036C */
    a1 = Obj_0CBDDE(m, obj);                              /* 0x120372 */
    r->a[1] = a1;
    uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);         /* 0x120378..0x12037A */
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);        /* 0x12037E */
    Sound_PlayHooked(m, 0x0016u);                         /* 0x120382..0x120392 */
    d0 = Obj_13E4D0(m, obj, 3u);                          /* 0x120396..0x120398 */
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0); /* 0x12039E..0x1203B4 */
    wr32(m, obj, 0x001203C0u);                            /* 0x1203B8 */
    return 0;
}
