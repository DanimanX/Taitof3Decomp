/* ROM 0x0E48FA: (recipe capture fnk_, work/zcap/recipes15.tsv): scripted flying enemy tick
 * (A0 = object). The only captured pair spans frames 0x06DD->0x0992, so the
 * harness skips it and the translation is verified against the bytes only;
 * it is the twin of
 * 0x0E4560 with a different death burst and a conditional collision push.
 * While +0x06 == 2 it integrates the vertical scroll into +0x3A; always
 * integrates A5+0x4228 into +0x3E and mirrors both into +0x14/+0x18. A bomb
 * hit inside the +0x14 window ([-0xA..0x11C]) hands over to 0x0CEF0E with
 * 0x12C relayed. Otherwise, with HP +0x24 > 0: runs the scripts (0x0CF1E4 /
 * 0x0CF4B8 / 0x0CF7FE / 0x0CF85A), aims (0x0D07B4), turns (0x0D0BBC), picks
 * the animation by heading (0x0D005A), spawns 0x0E35BE once per +0x23 timer,
 * then steps the script (0x0D0690), blinks (0x0CBE34), pushes to the collision
 * lists (0x0D0DC2) only while +0x1E is zero, and draws (0x001282). HP <= 0
 * awards 0x12C (0x0D0E04), spawns the 0x0DC6D0 burst - placed by 0x0D4A12 -
 * and dies (0x0CEE74). */
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

#define REC_0E3054 0x000E3054u /* lea (-0x1926,PC),A1 at 0x0E4978 / 0x0E49B6 */
#define TBL_0E470E 0x000E470Eu /* lea (-0x2BA,PC),A1 at 0x0E49C6 */

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


/* ROM 0x0D4A12 */
static int sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t *out_d0)
{
    uint32_t d0 = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)d1w * 4));
    if (d0w > 50u) {
        /* unimplemented: ROM 0x0D4E5A */
        return -1;
    }
    /* Jump table 0x0D4E5A [0..50] -> 0x0D4FF4..0x0D5396: exact D0 * k mod 2^32 */
    d0 = (uint32_t)(d0 * (uint32_t)d0w);
    *out_d0 = (uint16_t)(d0 >> 16);
    return 0;
}


int Fn_0E48FA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd8(m, obj + 0x06u) == 0x02u) {                         /* 0x0E48FA */
        uint32_t d0 = rd32(m, F3_A5 + 0x4224u);                 /* 0x0E4904 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);         /* 0x0E4908 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));              /* 0x0E490C */
    }
    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0E4912 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);             /* 0x0E4916 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));                  /* 0x0E491A */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u                 /* 0x0E4920 btst #1 */
        && (int16_t)rd16(m, obj + 0x14u) >= (int16_t)-0x000A     /* 0x0E4928 blt */
        && (int16_t)rd16(m, obj + 0x14u) <= (int16_t)0x011C) {   /* 0x0E4930 bgt */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));                /* 0x0E4938 */
        r->d[0] = 0x0000012Cu;                                   /* 0x0E4AAA */
        Obj_RelayTimer(m, 0x0000012Cu);                          /* 0x0E4AB0 jsr 0x0D0E30 */
        wr8(m, obj + 0x06u, 0x00u);                              /* 0x0E4AB6 */
        Obj_0CEF0E(m, obj);                                      /* 0x0E4ABA bra.l 0x0CEF0E */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                    /* 0x0E4944 ble */
        r->d[0] = 0x0000012Cu;                                   /* 0x0E4A3C */
        Obj_0D0E04(m, obj, 0x0000012Cu);                         /* 0x0E4A42 jsr 0x0D0E04 */
        goto spawn_burst;                                        /* 0x0E4A48 */
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)                 /* 0x0E494C btst #5 */
        goto spawn_burst;
    if (rd8(m, obj + 0x06u) == 0x02u)                            /* 0x0E4956 */
        goto tail_common;
    if ((rd8(m, obj + 0x07u) & 0x02u) != 0u) {                   /* 0x0E4960 bclr #1 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)                         /* 0x0E496A */
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u) {                             /* 0x0E4970 */
        if (ObjScript_CF4B8(m, obj, REC_0E3054) < 0)             /* 0x0E497C */
            return -1;
    }
    Obj_ArmOnScreen(m, obj);                                     /* 0x0E4982 jsr 0x0CF7FE */
    Obj_FlagOffscreen(m, obj);                                   /* 0x0E4988 jsr 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)                     /* 0x0E498E btst #3 */
        goto die_dying;                                          /* 0x0E4AA0 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));                  /* 0x0E4998 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));                  /* 0x0E499E */
    if (rd8(m, obj + 0x23u) != 0u)                               /* 0x0E49A4 */
        goto tick_23;
    if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)                     /* 0x0E49AC btst #5 */
        goto tail_common;
    sub_0D07B4(m, obj, REC_0E3054);                              /* 0x0E49BA */
    sub_0D0BBC(m, obj, REC_0E3054);                              /* 0x0E49C0 */
    r->a[1] = TBL_0E470E;                                        /* 0x0E49C6 lea */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x08u);     /* 0x0E49CA */
    Obj_SetAnimByDir(m, obj, TBL_0E470E, (uint16_t)r->d[0]);      /* 0x0E49CE jsr 0x0D005A */
    if (rd8(m, obj + 0x06u) == 0x03u)                            /* 0x0E49D4 */
        wr8(m, obj + 0x23u, 0x00u);                              /* 0x0E49DE */
    if (rd8(m, obj + 0x23u) == 0u)                               /* 0x0E49E2 */
        goto tail_common;

tick_23:
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));    /* 0x0E49EA subq.b */
    if (rd8(m, obj + 0x23u) != 0u)                               /* 0x0E49EE */
        goto tail_common;
    r->d[0] = Obj_Spawn(m, obj, 0x000E35BEu, 9u, 2u, 0u);        /* 0x0E49F2..0x0E4A0A */
    if ((int32_t)r->d[0] < 0)                                    /* 0x0E4A0E bmi */
        goto tail_common;
    wr8(m, obj + 0x06u, 0x02u);                                  /* 0x0E4A14 */

tail_common:
    Obj_StepScript(m, obj);                                      /* 0x0E4A1A jsr 0x0D0690 */
    Obj_BlinkColour(m, obj);                                     /* 0x0E4A20 jsr 0x0CBE34 */
    if (rd8(m, obj + 0x1Eu) == 0u)                               /* 0x0E4A26 tst.b */
        Obj_0D0DC2(m, obj);                                      /* 0x0E4A2E jsr 0x0D0DC2 */
    batch_draw_sprite(m, r, 0x000E4A3Au);                        /* 0x0E4A34 jsr 0x001282 */
    return 0;                                                    /* 0x0E4A3A rts */

spawn_burst:
    r->d[0] = Obj_Spawn(m, obj, 0x000DC6D0u, 3u, 2u, 0u);        /* 0x0E4A48..0x0E4A60 */
    if ((int32_t)r->d[0] >= 0) {                                 /* 0x0E4A64 bmi */
        uint32_t a1 = r->d[0];                                   /* 0x0E4A6A movea.l */
        r->a[1] = a1;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));               /* 0x0E4A6C */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));               /* 0x0E4A72 */
        wr16(m, a1 + 0x58u, 0x00B4u);                            /* 0x0E4A78 */
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));               /* 0x0E4A7E */
        wr16(m, a1 + 0x10u, rd16(m, obj + 0x1Eu));               /* 0x0E4A84 */
        uint16_t dz = 0x0010u;                                   /* 0x0E4A8A moveq #0x10 */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | rd8(m, obj + 0x1Eu); /* 0x0E4A8C..0x0E4A8E */
        r->d[0] = 0x00000010u;                                   /* moveq: whole register */
        uint16_t off = 0u;
        if (sub_0D4A12(m, dz, (uint16_t)(r->d[1] & 0xFFu), &off) < 0)
            return -1;                                           /* 0x0E4A92 (index > 50) */
        /* D0 = swap(product); the helper yields only its low word, which is
         * all 0x0E4A98/0x0E4A9C read. */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | off;
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + off)); /* 0x0E4A98 */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + off)); /* 0x0E4A9C */
    }

die_dying:
    wr8(m, obj + 0x06u, 0x00u);                                  /* 0x0E4AA0 */
    Obj_StartDying(m, obj);                                      /* 0x0E4AA4 bra.l 0x0CEE74 */
    return 0;
}
