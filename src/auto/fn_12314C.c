/* ROM 0x12314C: articulated boss/enemy segment tick (A0 = object, +0x08 = root,
 * +0x44 = anchor). Frees self (0x0105EC) if root->+0x59 != 0. Syncs HP from
 * root when A5+0x326F != 0; if HP <= 0 sets A5+0x326F = 2, +0x06 = 3 and
 * advances to 0x12325A. Otherwise interpolates angle +0x20 toward anchor via
 * 0x1235A0, moves along angle (0x0CBE12), optionally fires 0x0CCD04 when
 * root->+0x58 bit 0 is set and 0x0CC2BC(0x0E) == 0, updates sprite frame from
 * 0x1232B4[+0x3A], runs 0x11B06E if root->+0x24 > 0, draws sprite (0x001282),
 * and re-arms 0x12314C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_11B06E.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0w) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = (d1 << 16) | (d1 >> 16);
    d1w = (uint16_t)d1;
    if (d1w == 0u)
        d1 = setw(d1, 1u);
    *d1_out = d1;
    uint16_t div = (uint16_t)d1;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    return (uint16_t)(num % div);
}

/* ROM 0x1235D8: copy root (+0x08) HP into +0x24 if A5+0x326F != 0 */
static uint32_t sub_1235D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    if (rd8(m, F3_A5 + 0x326Fu) != 0u) {
        a1 = rd32(m, obj + 0x08u);
        wr32(m, obj + 0x24u, rd32(m, a1 + 0x24u));
    }
    return a1;
}

/* ROM 0x1235A0: copy anchor (+0x44) pos and ease angle +0x20 toward anchor+root+0x3E */
static void sub_1235A0(const f3mem_t *m, uint32_t obj, uint32_t *d0_out, uint32_t *d1_out)
{
    uint32_t a2 = rd32(m, obj + 0x44u);                  /* 0x1235A6 */
    uint32_t a3 = rd32(m, obj + 0x08u);                  /* 0x1235AA */
    uint16_t d0w = rd16(m, a3 + 0x3Eu);                  /* 0x1235AE */
    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));           /* 0x1235B2 */
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));           /* 0x1235B8 */
    uint16_t d1w = rd16(m, obj + 0x20u);                 /* 0x1235BE */
    d0w = (uint16_t)(d0w + rd16(m, a2 + 0x20u));         /* 0x1235C2 */
    d0w = (uint16_t)(d0w - d1w);                         /* 0x1235C6 */
    int32_t num = (int32_t)(int16_t)d0w;                 /* 0x1235C8 ext.l */
    int16_t div = (int16_t)rd16(m, obj + 0x0Cu);         /* 0x1235CA divs.w */
    uint32_t d0 = (uint32_t)num;
    if (div != 0) {
        int32_t q = num / div;
        int32_t rem = num % div;
        if (q >= -32768 && q <= 32767)
            d0 = ((uint32_t)(uint16_t)(int16_t)rem << 16) | (uint16_t)(int16_t)q;
    }
    d0w = (uint16_t)((uint16_t)d0 + d1w);                /* 0x1235CE */
    wr16(m, obj + 0x20u, d0w);                           /* 0x1235D0 */
    d0w = (uint16_t)(d0w >> 8);                          /* 0x1235D4 lsr.w #8 */
    *d0_out = setw(d0, d0w);
    *d1_out = setw(*d1_out, d1w);
}

static void draw_sprite_with_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                                  uint32_t d0, uint32_t d1)
{
    Obj_DrawSprite(m, obj);
    uint32_t a2 = rd32(m, obj + 0x10u);
    r->a[1] = rd32(m, SPR_CURSOR);
    if (rd32(m, a2) == 0x00010001u) {
        d0 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
        d0 &= 0x0FFF0FFFu;
        d0 = setw(d0, rd16(m, a2 + 6u));
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        d1 = setw(d1, (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u));
    } else {
        uint16_t cols_m1 = (uint16_t)(rd16(m, a2) - 1u);
        d0 = 0x0000FFFFu;
        d1 = ((uint32_t)cols_m1 << 16) | 0xFFFFu;
    }
    r->d[0] = d0;
    r->d[1] = d1;
}

int Fn_12314C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12314C */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x123150..0x123154 */
        Obj_Free(m, obj);                                /* 0x1232A8 (preserves D0,A0,A1) */
        return 0;
    }

    a1 = sub_1235D8(m, obj, a1);                         /* 0x123158 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x12315C..0x123160 */
        wr8(m, F3_A5 + 0x326Fu, 2u);                     /* 0x123244 */
        wr16(m, obj + 0x06u, 3u);                        /* 0x12324A (0x123250 beq folded) */
        wr32(m, obj + 0x00u, 0x0012325Au);               /* 0x123252 */
        r->a[1] = a1;
        return 0;
    }

    sub_1235A0(m, obj, &d0, &d1);                        /* 0x123164 */
    {
        /* 0x123168 jsr 0x000CBE12: leaves D1 = speed * cos[angle] */
        int16_t speed = (int16_t)rd16(m, obj + 0x42u);
        int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * (uint16_t)d0);
        d1 = (uint32_t)((int32_t)speed * (int32_t)cos_v);
        Obj_MoveByAngle(m, obj, (uint16_t)d0);
    }
    wr8(m, obj + 0x4Fu, 0u);                             /* 0x12316E */
    a1 = rd32(m, obj + 0x08u);                           /* 0x123172 */

    if ((rd8(m, a1 + 0x58u) & 0x01u) != 0u) {            /* 0x123176..0x12317C */
        uint16_t w = (uint16_t)((rd16(m, obj + 0x22u) - 1u) & 3u);
        d0 = setw(d0, w);
        if (w == 0u) {
            uint16_t r_mod = sub_0CC2BC(m, 0x000Eu, &d1); /* 0x123192 */
            d0 = setw(d0, r_mod);
            if (r_mod == 0u) {
                d1 = 0x08080000u | rd16(m, obj + 0x20u); /* 0x12319E..0x1231A4 */
                d0 = rd32(m, obj + 0x0Cu);               /* 0x1231A8 */
                if ((d0 & 2u) == 0u)
                    d1 = setw(d1, (uint16_t)(d1 - 0x4000u)); /* 0x1231B4 */
                else
                    d1 = setw(d1, (uint16_t)(d1 + 0x4000u)); /* 0x1231BC */
                d0 = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x1231D0 */
                wr32(m, obj + 0x3Au, 0u);                /* 0x1231DA */
            }
        }
    }

    {
        uint16_t idx = rd16(m, obj + 0x3Au);             /* 0x1231DE */
        d0 = setw(d0, idx);
        if ((int16_t)idx < 0) {                          /* 0x1231E2 */
            a1 = rd32(m, obj + 0x08u);                   /* 0x1231E6 */
            wr32(m, obj + 0x10u, rd32(m, a1 + 0x10u));   /* 0x1231EA */
        } else {
            /* 0x1231F4: table at 0x1231F6 + 0xBE = 0x1232B4 */
            wr32(m, obj + 0x10u, rd32(m, 0x001232B4u + (uint32_t)(int32_t)(int16_t)idx * 4u));
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x8000u); /* 0x1231FE */
            if ((int16_t)rd16(m, obj + 0x3Au) > 7)       /* 0x123206..0x12320C */
                wr16(m, obj + 0x3Au, 0xFFFFu);           /* 0x123210 */
        }
    }

    if ((int32_t)rd32(m, a1 + 0x24u) > 0) {              /* 0x123216..0x12321A */
        Obj_11B06E_Tick(m, obj);                         /* 0x12321E */
    } else {
        uint16_t fl = rd16(m, obj + 0x22u) & 3u;         /* 0x123228..0x12322C */
        d0 = setw(d0, fl);
        wr8(m, obj + 0x1Cu, (uint8_t)fl);                /* 0x123230 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x123234 */
    }

    draw_sprite_with_regs(m, r, obj, d0, d1);            /* 0x12323A */
    wr32(m, obj + 0x00u, 0x0012314Cu);                   /* 0x123144 */
    return 0;
}
