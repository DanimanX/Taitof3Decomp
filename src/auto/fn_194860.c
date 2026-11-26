/* ROM 0x194860: boss sub-part animation & hit-flash tick (A0 = object).
 * Syncs position from parent (+0x08) + (+0x16, +0x1A), ticks 0x179A5C
 * animation (+0x40), centers (+0x14, +0x18) by sprite dimensions * 8, runs
 * 0x1948FE (re-arming anim from +0x0C and spawning 0x183DBA when A5+0x4B9C
 * matches +0x22), cycles blink colour +0x1D (6/8/A5+0x4B36), clears +0x35,
 * draws sprite (0x001282), and re-arms 0x194860 unless A5+0x4B38 == -1. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x1948FE: phase-matched anim switch and 0x183DBA spawn. */
static int sub_1948FE(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x4B9Cu) != rd8(m, obj + 0x22u)) /* 0x1948FE..0x194906 */
        return 0;

    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {             /* 0x19490A */
        uint32_t a1 = rd32(m, rd32(m, obj + 0x0Cu));     /* 0x194912..0x194916 */
        uint16_t idx = rd16(m, a1);                      /* 0x194918 */
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);                       /* 0x19491A */
        wr32(m, obj + 0x3Au, a1);                        /* 0x19491E */
        if (Obj_AnimInit(m, obj, idx) < 0)               /* 0x194922..0x19492C */
            return -1;
    }

    {
        uint32_t a1 = rd32(m, obj + 0x0Cu);              /* 0x19492E */
        if (rd32(m, a1 + 4u) == rd32(m, obj + 0x3Au)) {  /* 0x194932..0x19493A */
            (void)Obj_Spawn(m, obj, 0x00183DBAu, 9u, 2u, 0x0000001Eu); /* 0x19493E..0x194958 */
            wr16(m, F3_A5 + 0x4B9Cu, 0xFFFFu);           /* 0x19495C */
        }
    }
    return 0;
}

int Fn_194860(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t anim_idx = rd16(m, obj + 0x40u);
    if (anim_idx == 3u || anim_idx == 11u || anim_idx == 13u || anim_idx > 16u)
        return -1;

    uint32_t parent = rd32(m, obj + 0x08u);              /* 0x194860 */
    uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);                             /* 0x194874 */
    wr16(m, obj + 0x54u, y);                             /* 0x194878 */
    wr16(m, obj + 0x14u, x);                             /* 0x19487C */
    wr16(m, obj + 0x18u, y);                             /* 0x194880 */

    if (Obj_AnimTick(m, obj, anim_idx) < 0)              /* 0x194884..0x194892 */
        return -1;

    {
        uint32_t spr = rd32(m, obj + 0x10u);             /* 0x194894 */
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);    /* 0x194898..0x19489E */
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3); /* 0x19489A..0x19489C */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x1948A0 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w)); /* 0x1948A4 */
    }

    if (sub_1948FE(m, obj) < 0)                          /* 0x1948A8 */
        return -1;

    if (rd8(m, obj + 0x1Du) == 0x06u)                    /* 0x1948AC..0x1948B2 */
        wr8(m, obj + 0x1Du, 0x08u);                      /* 0x1948D2 */
    else if (rd8(m, obj + 0x35u) != 0u)                  /* 0x1948B6..0x1948BA */
        wr8(m, obj + 0x1Du, 0x06u);                      /* 0x1948C8 */
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));    /* 0x1948BE */

    wr8(m, obj + 0x35u, 0u);                             /* 0x1948DC */
    draw_sprite_regs(m, r);                              /* 0x1948E0 */

    if (rd16(m, F3_A5 + 0x4B38u) != 0xFFFFu)             /* 0x1948E6 */
        wr32(m, obj, 0x00194860u);                       /* 0x194858 */
    else
        Obj_Free(m, obj);                                /* 0x1948F0..0x1948FC */

    return 0;
}
