/* ROM 0x0E4560: (injected capture fnj_, NOIRQ): scripted flying enemy tick
 * (A0 = object). The only captured pair spans frames 0x06DD->0x0992, so the
 * harness skips it and the translation is verified against the bytes only. While
 * +0x06 == 2 it integrates the vertical scroll into +0x3A; always integrates
 * A5+0x4228 into +0x3E and mirrors both into +0x14/+0x18. A bomb hit off the
 * +0x14 window ([-0xA..0x11C]) hands over to 0x0CEF0E. Otherwise, with HP
 * +0x24 > 0: runs the scripts (0x0CF1E4 / 0x0CF4B8 / 0x0CF7FE / 0x0CF85A),
 * aims (0x0D07B4), turns (0x0D0BBC), picks the animation by heading
 * (0x0D005A), spawns 0x0E35BE once per +0x23 timer, registers collision
 * (0x0D0DC2), blinks (0x0CBE34) and draws (0x001282). HP <= 0 awards 0x12C
 * (0x0D0E04), spawns the 0x0DC328 burst and dies (0x0CEE74); a bomb hit on HP
 * > 0 just dies at 0x0CEF0E. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

#define REC_0E3054 0x000E3054u /* lea (-0x158C,PC),A1 at 0x0E45DE / 0x0E461C */
#define TBL_0E470E 0x000E470Eu /* lea (0xE0,PC),A1 at 0x0E462C */

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07B4: aim at nearest live player within bounds/angle window. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t p68 = rd32(m, obj + 0x68u);
    int16_t cx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, p68));
    int16_t cy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, p68 + 2u));
    if (cx < 0x28 || cx > 0x0127 || cy < 0x10 || cy > 0xDF)
        return;

    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    int16_t best_ang[2] = { -1, -1 };
    uint16_t max_dx[2] = { 0u, 0u };
    uint16_t max_dy[2] = { 0u, 0u };

    uint16_t r_min = rd16(m, a1 + 0x08u);
    uint16_t r_max = rd16(m, a1 + 0x0Au);
    uint16_t a_min = rd16(m, a1 + 0x0Cu);
    uint16_t a_max = rd16(m, a1 + 0x0Eu);

    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t sdy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u)
                                + rd16(m, p68 + 2u) - 0x0F);
        uint16_t ady = (uint16_t)(sdy < 0 ? -sdy : sdy);
        if (ady > r_max)
            continue;
        int16_t sdx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u)
                                + rd16(m, p68) - 0x17);
        uint16_t adx = (uint16_t)(sdx < 0 ? -sdx : sdx);
        if (adx > r_max)
            continue;
        if (sdx < 0) {
            if (adx < r_min && ady < r_min)
                continue;
        } else {
            if ((int16_t)adx < (int16_t)(r_min - 0x1E) && ady < r_min)
                continue;
        }
        uint16_t ang = Angle360(m, (uint16_t)sdx, (uint16_t)sdy);
        if (a_min > a_max) {
            if (ang < a_min && ang > a_max)
                continue;
        } else {
            if (ang < a_min || ang > a_max)
                continue;
        }
        best_ang[i] = (int16_t)ang;
        max_dx[i] = adx;
        max_dy[i] = ady;
    }

    int16_t chosen = best_ang[1];
    if (best_ang[0] >= 0) {
        if (best_ang[1] < 0) {
            chosen = best_ang[0];
        } else {
            uint16_t d0 = (max_dx[0] >= max_dy[0]) ? max_dx[0] : max_dy[0];
            uint16_t d1 = (max_dx[1] >= max_dy[1]) ? max_dx[1] : max_dy[1];
            if (d1 >= d0)
                chosen = best_ang[0];
        }
    } else if (best_ang[1] < 0) {
        return;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t thr = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    if ((int16_t)thr > (int16_t)Rng_Mix1D60(m))
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0D0BBC: turn obj+0x08 toward obj+0x42. */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d1w = rd16(m, obj + 0x42u);
    int16_t diff = (int16_t)(d1w - rd16(m, obj + 0x08u));
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);
    if ((uint16_t)diff > 0x00B4u) {
        diff = (int16_t)(0x0168 - diff);
        if ((uint16_t)diff <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, d1w);
            return;
        }
        int32_t v = (int32_t)rd32(m, obj + 0x08u) - (int32_t)rd32(m, a1 + 0x04u);
        wr32(m, obj + 0x08u, (uint32_t)v);
        if (v < 0)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
    } else {
        if ((uint16_t)diff <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, d1w);
            return;
        }
        uint32_t v = rd32(m, obj + 0x08u) + rd32(m, a1 + 0x04u);
        wr32(m, obj + 0x08u, v);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
    }
}


int Fn_0E4560(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd8(m, obj + 0x06u) == 0x02u) {                         /* 0x0E4560 */
        uint32_t d0 = rd32(m, F3_A5 + 0x4224u);                 /* 0x0E456A */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);         /* 0x0E456E */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));              /* 0x0E4572 */
    }
    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0E4578 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);             /* 0x0E457C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));                  /* 0x0E4580 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u                 /* 0x0E4586 btst #1 */
        && (int16_t)rd16(m, obj + 0x14u) >= (int16_t)-0x000A             /* 0x0E458E blt */
        && (int16_t)rd16(m, obj + 0x14u) <= (int16_t)0x011C) {           /* 0x0E4596 bgt */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));                /* 0x0E459E */
        r->d[0] = 0x0000012Cu;                                   /* 0x0E46F8 */
        Obj_RelayTimer(m, 0x0000012Cu);                          /* 0x0E46FE jsr 0x0D0E30 */
        wr8(m, obj + 0x06u, 0x00u);                              /* 0x0E4704 */
        Obj_0CEF0E(m, obj);                                      /* 0x0E4708 bra.l 0x0CEF0E */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                    /* 0x0E45AA ble */
        r->d[0] = 0x0000012Cu;                                   /* 0x0E469A */
        Obj_0D0E04(m, obj, 0x0000012Cu);                         /* 0x0E46A0 jsr 0x0D0E04 */
        goto spawn_burst;                                        /* 0x0E46A6 */
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)                 /* 0x0E45B2 btst #5 */
        goto spawn_burst;
    if (rd8(m, obj + 0x06u) == 0x02u)                            /* 0x0E45BC */
        goto tail_common;
    if ((rd8(m, obj + 0x07u) & 0x02u) != 0u) {                   /* 0x0E45C6 bclr #1 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)                         /* 0x0E45D0 */
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u) {                             /* 0x0E45D6 */
        if (ObjScript_CF4B8(m, obj, REC_0E3054) < 0)             /* 0x0E45E2 */
            return -1;
    }
    Obj_ArmOnScreen(m, obj);                                     /* 0x0E45E8 jsr 0x0CF7FE */
    Obj_FlagOffscreen(m, obj);                                   /* 0x0E45EE jsr 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)                     /* 0x0E45F4 btst #3 */
        goto die_dying;                                          /* 0x0E46EE */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));                  /* 0x0E45FE */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));                  /* 0x0E4604 */
    if (rd8(m, obj + 0x23u) != 0u)                               /* 0x0E460A */
        goto tick_23;
    if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)                     /* 0x0E4612 btst #5 */
        goto tail_common;
    sub_0D07B4(m, obj, REC_0E3054);                              /* 0x0E4620 */
    sub_0D0BBC(m, obj, REC_0E3054);                              /* 0x0E4626 */
    r->a[1] = TBL_0E470E;                                        /* 0x0E462C lea */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x08u);     /* 0x0E4630 */
    Obj_SetAnimByDir(m, obj, TBL_0E470E, (uint16_t)r->d[0]);      /* 0x0E4634 jsr 0x0D005A */
    if (rd8(m, obj + 0x06u) == 0x03u)                            /* 0x0E463A */
        wr8(m, obj + 0x23u, 0x00u);                              /* 0x0E4644 */
    if (rd8(m, obj + 0x23u) == 0u)                               /* 0x0E4648 */
        goto tail_common;

tick_23:
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));    /* 0x0E4650 subq.b */
    if (rd8(m, obj + 0x23u) != 0u)                               /* 0x0E4654 */
        goto tail_common;
    r->d[0] = Obj_Spawn(m, obj, 0x000E35BEu, 9u, 2u, 0u);        /* 0x0E4658..0x0E4670 */
    if ((int32_t)r->d[0] < 0)                                    /* 0x0E4674 bmi */
        goto tail_common;
    wr8(m, obj + 0x06u, 0x02u);                                  /* 0x0E467A */

tail_common:
    Obj_StepScript(m, obj);                                      /* 0x0E4680 jsr 0x0D0690 */
    Obj_0D0DC2(m, obj);                                          /* 0x0E4686 jsr 0x0D0DC2 */
    Obj_BlinkColour(m, obj);                                     /* 0x0E468C jsr 0x0CBE34 */
    batch_draw_sprite(m, r, 0x000E4698u);                        /* 0x0E4692 jsr 0x001282 */
    return 0;                                                    /* 0x0E4698 rts */

spawn_burst:
    r->d[0] = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u);        /* 0x0E46A6..0x0E46BE */
    if ((int32_t)r->d[0] >= 0) {                                 /* 0x0E46C2 bmi */
        uint32_t a1 = r->d[0];                                   /* 0x0E46C8 movea.l */
        r->a[1] = a1;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));               /* 0x0E46CA */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u)); /* 0x0E46D0 */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));               /* 0x0E46D6 */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u)); /* 0x0E46DC */
        wr16(m, a1 + 0x58u, 0x00B4u);                            /* 0x0E46E2 */
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));               /* 0x0E46E8 */
    }

die_dying:
    wr8(m, obj + 0x06u, 0x00u);                                  /* 0x0E46EE / 0x0E46F2 */
    Obj_StartDying(m, obj);                                      /* 0x0E46F2 bra.l 0x0CEE74 */
    return 0;
}
