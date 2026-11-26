/* ROM 0x10C2F4: enemy tick (A0 = object). While on screen (A5+0x1D58 bit 1)
 * and inside the playfield it resets the colour from +0x4F and jumps to the
 * off-screen handler tail 0x0CEF0E; otherwise it integrates +0x50/+0x54 into
 * +0x3A/+0x3E, runs +0x23 idle frames (checking two terrain points at the end)
 * and steps the script / hit flash / sprite. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D0E30: score += D0 to the object at A5+0x1D50 */
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x0CEF0E: pause -> die; on screen -> colour from +0x4F then the
 * explosion tick 0x0CBB2C; else park on 0x0CEF44. */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {          /* 0x0CEF0E */
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x0CEF18 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                    /* 0x0CEF20..0x0CEF2E */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x0CEF30 */
            Obj_ExplodeTick(m, obj);                        /* 0x0CEF36 jmp 0x0CBB2C */
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);                      /* 0x0CEF3C */
}

/* ROM 0x0D0DC2: append obj to A5+0x8FE, then Coll_AddHazard (0x0D0DCC) */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    Coll_AddEnemyHazard(m, obj);
}

/* ROM 0x0D2D44: (D0, D1) = angle speed tables scaled by the 0..50 count */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    /* jump table 0x0D4F26 [0..50] -> (D0*k, D1*k) */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

int Fn_10C2F4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x10C2F4 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x10C30C */
            sub_0D0E30(m, 0x00C8u);                         /* 0x10C476 jmp 0x10C476 */
            sub_0CEF0E(m, obj);                             /* bra 0x0CEF0E */
            return 0;
        }
    }

    /* 0x10C318: dying/dead objects still on the hit list */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* tst.l / ble */
        Score_OnHit(m, obj, 0x00C8u);
        /* 0x10C41E falls through to 0x10C42A */
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        /* 0x10C32A: integrate the velocity (scroll-relative) into +0x3A/+0x3E */
        uint32_t d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);

        Obj_FlagOffscreen(m, obj);                          /* 0x10C342 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x10C348 bne 0x0CEE74 */
            Obj_StartDying(m, obj);
            return 0;
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x10C354 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x10C35A */

        /* 0x10C360: idle counter in +0x23; while it runs, only the tail runs */
        if (rd8(m, obj + 0x23u) != 0u) {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, c);
            if (c != 0u)
                goto tail;                                  /* 0x10C36C bne 0x10C404 */
        }

        /* 0x10C370: two terrain probes decide whether the heading flips */
        {
            int16_t d0 = (int16_t)(rd16(m, obj + 0x14u) + 0x000Fu);
            int16_t d1 = (int16_t)(rd16(m, obj + 0x18u) + 0x0018u);
            if ((uint16_t)rd16(m, obj + 0x58u) < 0x00B4u)    /* 0x10C380 bcc 0x10C38E */
                d1 = (int16_t)(d1 - 0x0010);
            if (Terrain_TestPoint(m, d0, d1) == 0) {        /* 0x10C38E */
                d0 = (int16_t)(rd16(m, obj + 0x14u) + 0x0017u);
                d1 = (int16_t)(rd16(m, obj + 0x18u) + 0x0010u);
                uint16_t a = rd16(m, obj + 0x58u);
                if (!(a < 0x005Au || a >= 0x010Eu))         /* 0x10C3AA bcs / 0x10C3B4 bcc */
                    d0 = (int16_t)(d0 - 0x0010);
                if (Terrain_TestPoint(m, d0, d1) == 0)      /* 0x10C3C2 */
                    goto tail;                              /* 0x10C3CA beq 0x10C404 */
                /* 0x10C3CE: heading = 360 - (heading - 180) */
                d0 = (int16_t)((uint16_t)(rd16(m, obj + 0x58u) - 0x00B4u));
                d0 = (int16_t)(-(uint16_t)d0);
                if (d0 < 0)                                 /* 0x10C3D8 bpl 0x10C3EE */
                    d0 = (int16_t)(d0 + 0x0168);
            } else {
                /* 0x10C3E4: heading = 360 - heading */
                d0 = (int16_t)(-(uint16_t)rd16(m, obj + 0x58u));
                d0 = (int16_t)(d0 + 0x0168);
            }
            wr16(m, obj + 0x58u, (uint16_t)d0);             /* 0x10C3EE */
            {
                uint32_t d0l, d1l;
                sub_0D2D44(m, (uint16_t)d0, rd16(m, obj + 0x60u), &d0l, &d1l);
                wr32(m, obj + 0x50u, d0l);                  /* 0x10C3FC */
                wr32(m, obj + 0x54u, d1l);                  /* 0x10C400 */
            }
        }

tail:
        Obj_StepScript(m, obj);                             /* 0x10C404 jsr 0x0D0690 */
        sub_0D0DC2(m, obj);                                 /* 0x10C40A jsr 0x0D0DC2 */
        Obj_HitFlash(m, obj);                               /* 0x10C410 jsr 0x0CBE34 */
        Obj_DrawSprite(m, obj);                             /* 0x10C416 jsr 0x001282 */
        return 0;                                           /* 0x10C41C rts */
    }

    /* 0x10C42A: spawn a 1-frame score/effect object (fn 0x0DC43E, group 3,
     * 2 groups, arg 0) at this object's position and hand it +0x58/+0x60. */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
        if ((int32_t)child < 0) {                           /* 0x10C448 bmi 0x0CEE74 */
            Obj_StartDying(m, obj);
            return 0;
        }
        r->a[1] = child;                                    /* movea.l D0,A1 */
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));       /* 0x10C450 */
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x000Fu));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));       /* 0x10C45C */
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
        wr16(m, child + 0x58u, 0u);                         /* 0x10C468 */
        wr16(m, child + 0x60u, 0u);                         /* 0x10C46C */
    }
    Obj_StartDying(m, obj);                                 /* 0x10C470 bra 0x0CEE74 */
    return 0;
}
