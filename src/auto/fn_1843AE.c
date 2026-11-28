/* ROM 0x1843AE: (recipe capture fnk_, work/zcap/recipes15.tsv): scripted boss-part object tick
 * (A0 = object). Clears +0x24 when A5-0x144C bit 4 (0x10) is set; while
 * +0x24 <= 0 it runs the vanish path (0x184482 -> 0x17C59A: freeze, shrink by
 * +0x1E/+0x1F in 0x17C662, or explode 0x0CBB2C). Otherwise: score/explode
 * branch when A5+0x1D58 bit 1 is set on-screen, steps the 10-byte script
 * (0x178982), frees via 0x17C78E when +0x50/+0x54 leave the 0x1849F8/0x1849FC
 * box, registers in hazard lists A5+0x906 and A5+0x8FE, syncs +0x14/+0x18 from
 * +0x50/+0x54, offsets by the sprite zoom byte +0x1F, ticks the 0x179A5C
 * animation, centres by 8 * sprite size, draws (0x001282) and re-arms
 * 0x1843AE. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* mulu.w D1w,D0 ; lsr.w #8,D0w ; lsr.w #1,D0w: the word ops keep only bits
 * 9..15 of the 32-bit product, so mask the product before shifting. */
static uint16_t zoom_step(uint8_t b, uint16_t sizew)
{
    uint32_t p = (uint32_t)b * (uint32_t)sizew;
    return (uint16_t)((uint16_t)(p & 0xFFFFu) >> 9);
}

/* cmp2.w (base)/(base+2),D0w + bcs: the word is outside the modular inclusive
 * (lower, upper) pair - the same model src/auto/fn_183A2C.c uses. */
static int outside(const f3mem_t *m, uint32_t base, uint16_t v)
{
    uint16_t lo = rd16(m, base);
    uint16_t hi = rd16(m, base + 2u);
    return (uint16_t)(v - lo) > (uint16_t)(hi - lo);
}

static int in_bounds_1843(const f3mem_t *m, uint32_t obj)
{
    if (outside(m, 0x001849F8u, rd16(m, obj + 0x50u)))
        return 0;
    if (outside(m, 0x001849FCu, rd16(m, obj + 0x54u)))
        return 0;
    return 1;
}

/* ROM 0x17B02E: score receiver: 1 -> (+0x36)->+8 (A1 = +0x36), 2 -> +0x36,
 * else -1 (A1 untouched). */
static uint32_t sub_17B02E(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);                                    /* 0x17B02E */
    if (k == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);                             /* 0x17B038 */
        r->a[1] = a1;
        return rd32(m, a1 + 0x08u);                                     /* 0x17B03C */
    }
    if (k == 2u)                                                        /* 0x17B042 */
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;                                                 /* 0x17B052 */
}

/* +0x10 frame header {dy.w, dx.w}: shift each by 3 and subtract from
 * +0x14/+0x18, leaving A1 = frame + 4. */
static void centre_by_frame(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);                                 /* 0x184464 */
    r->a[1] = a1;
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    r->d[1] = setw(r->d[1], d1);
    r->d[0] = setw(r->d[0], d0);
    r->a[1] = a1 + 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
}

/* Animation half of the 0x179A5C table: index 0 init/tick only, the sole
 * index the two script heads 0x17C79C / 0x17C7DC select. */
static int anim_init_1843(const f3mem_t *m, uint32_t obj)
{
    int16_t d0 = (int16_t)(uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
    uint32_t a1 = 0x0017C79Cu + (d0 > 8 ? 0x40u : 0u);                  /* 0x17C5C4..0x17C5D0 */
    uint16_t idx = rd16(m, a1);                                         /* 0x17C5DA */
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return Obj_AnimInit(m, obj, idx);                                   /* 0x17C5EA */
}

/* ROM 0x17C78E: off-screen / dead: free the slot. */
static void lab_17C78E(const f3mem_t *m, f3regs_t *r)
{
    batch_free_object(m, r, 0x0017C796u);
}

/* ROM 0x17C614: script step, position sync, animation tick, centre, draw. */
static int lab_17C614(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if (Obj_Script10(m, obj) < 0)                                       /* 0x17C614 */
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                         /* 0x17C61A */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                         /* 0x17C620 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)                 /* 0x17C626..0x17C634 */
        return -1;
    centre_by_frame(m, r, obj);                                         /* 0x17C636 */
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {                            /* 0x17C64A */
        lab_17C78E(m, r);
        return 0;
    }
    batch_draw_sprite(m, r, 0x0017C658u);                               /* 0x17C652 */
    wr32(m, obj, 0x0017C660u);                                          /* 0x17C658 */
    return 0;
}

/* ROM 0x17C662: shrink away by +0x1E/+0x1F (zoom bytes). */
static int lab_17C662(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if (rd8(m, obj + 0x1Fu) >= 0xF0u || rd8(m, obj + 0x1Eu) >= 0xF0u) { /* 0x17C662..0x17C67A */
        lab_17C78E(m, r);
        return 0;
    }
    if (Obj_Script10(m, obj) < 0)                                       /* 0x17C67E */
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                         /* 0x17C684 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                         /* 0x17C68A */

    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));        /* 0x17C690 */
    uint32_t a1 = rd32(m, obj + 0x10u);                                 /* 0x17C696 */
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 2u) << 4);
    uint16_t d0 = zoom_step(rd8(m, obj + 0x1Fu), d1);                   /* 0x17C69A..0x17C6AA */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));        /* 0x17C6AC */

    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));        /* 0x17C6B0 */
    a1 = rd32(m, obj + 0x10u);                                          /* 0x17C6B6 */
    d1 = (uint16_t)(rd16(m, a1) << 4);
    d0 = zoom_step(rd8(m, obj + 0x1Eu), d1);                            /* 0x17C6BA..0x17C6C8 */
    r->d[0] = setw(r->d[0], d0);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));        /* 0x17C6CA */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)                 /* 0x17C6CE..0x17C6DC */
        return -1;
    centre_by_frame(m, r, obj);                                         /* 0x17C6DE */
    batch_draw_sprite(m, r, 0x0017C6FEu);                               /* 0x17C6F2 */
    wr32(m, obj, 0x0017C700u);                                          /* 0x17C6F8 */
    return 0;
}

/* ROM 0x17C59A: death / vanish state entry. */
static int lab_17C59A(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u)                            /* 0x17C59A btst #1 */
        return lab_17C662(m, r, obj);

    uint16_t d1 = rd16(m, obj + 0x62u);                                 /* 0x17C5A4 bfextu {0:16} */
    r->d[1] = d1;
    uint32_t d0 = sub_17B02E(m, r, obj);                                /* 0x17C5AA */
    r->d[0] = d0;
    if ((int32_t)d0 >= 0) {                                             /* 0x17C5B2 */
        uint32_t a1 = d0;
        r->a[1] = a1;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d1);                  /* 0x17C5B8 */
    }

    wr16(m, obj + 0x1Eu, 0u);                                           /* 0x17C5BC */
    wr16(m, obj + 0x1Cu, 0u);                                           /* 0x17C5C0 */
    return anim_init_1843(m, obj) == 0 ? lab_17C614(m, r, obj) : -1;    /* 0x17C5C4..0x17C614 */
}

int Fn_1843AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)                        /* 0x1843AE btst #4 */
        wr32(m, obj + 0x24u, 0u);                                       /* 0x1843B8 */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                           /* 0x1843BC */
        /* 0x184482: falling apart - spawn the three 0x184322 fragments
         * (arg byte = +0x20 - 0x20, +0x20, +0x60) unless the zoom has
         * already run past 0x80, then the 0x17C59A death state. */
        uint16_t d1 = (uint16_t)(rd8(m, obj + 0x1Eu) + 0x40u);          /* 0x184482..0x184488 */
        if ((int16_t)d1 > 0x0080)                                       /* 0x18448C..0x184490 */
            return lab_17C59A(m, r, obj);

        uint32_t arg = ((uint32_t)d1 << 8) & 0xFFFFFF00u;               /* 0x184494 lsl.l #8 */
        arg = (arg & 0xFFFFFF00u) | (uint8_t)(rd8(m, obj + 0x20u) - 0x20u);
        r->d[1] = arg;
        r->d[0] = Obj_Spawn(m, obj, 0x00184322u, 9u, 1u, r->d[1]);      /* 0x18449E..0x1844B4 */
        arg = (arg & 0xFFFFFF00u) | (uint8_t)(rd8(m, obj + 0x20u));
        r->d[1] = arg;
        r->d[0] = Obj_Spawn(m, obj, 0x00184322u, 9u, 1u, r->d[1]);      /* 0x1844B8..0x1844D2 */
        arg = (arg & 0xFFFFFF00u) | (uint8_t)(rd8(m, obj + 0x20u) + 0x20u);
        r->d[1] = arg;
        r->d[0] = Obj_Spawn(m, obj, 0x00184322u, 9u, 1u, r->d[1]);      /* 0x1844D6..0x1844F0 */
        return lab_17C59A(m, r, obj);                                   /* 0x1844F4 bra */
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {                      /* 0x1843C4 btst #1 */
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);                         /* 0x1843CC */
        r->a[1] = a1;
        uint32_t d0 = rd16(m, obj + 0x62u);                             /* 0x1843D0 bfextu {0:16} */
        r->d[0] = d0;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);                  /* 0x1843D6 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));                       /* 0x1843DA */
        Obj_ExplodeTick(m, obj);                                        /* 0x1843E0 jmp 0x0CBB2C */
        return 0;
    }

    if (Obj_Script10(m, obj) < 0)                                       /* 0x1843E6 */
        return -1;
    if (!in_bounds_1843(m, obj)) {                                      /* 0x1843EC..0x184408 */
        lab_17C78E(m, r);
        return 0;
    }

    uint32_t a1 = rd32(m, F3_A5 + 0x906u);                              /* 0x18440C */
    r->a[1] = a1;
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x906u, a1 + 4u);                                   /* 0x184412 */
    a1 = rd32(m, F3_A5 + 0x8FEu);                                       /* 0x184416 */
    r->a[1] = a1;
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);                                   /* 0x18441C */

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                         /* 0x184420 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                         /* 0x184426 */
    /* 0x18442C/0x184432: addi.b #0 on +0x1F / +0x1E - a same-value write. */

    a1 = rd32(m, obj + 0x10u);                                          /* 0x184438 */
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 4);
    uint16_t d0 = zoom_step(rd8(m, obj + 0x1Fu), d1);                   /* 0x18443C..0x18444A */
    r->d[0] = setw(r->d[0], d0);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));        /* 0x18444C */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));        /* 0x184450 */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)                 /* 0x184454..0x184462 */
        return -1;
    centre_by_frame(m, r, obj);                                         /* 0x184464 */
    batch_draw_sprite(m, r, 0x0018447Eu);                               /* 0x184478 */
    wr32(m, obj, 0x001843AEu);                                          /* 0x1843A6 */
    return 0;
}
