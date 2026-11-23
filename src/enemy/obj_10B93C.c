#include "obj_10B93C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x10B93C (BossCore): core object of a multi-part boss.
 *
 *   state 0 (0x10BB3A)  aim, then spawn the next part as a child of A0; once
 *                       its counter A0+0x23 is set switch to state 2.
 *   state 1 (0x10B9D0)  tail only.
 *   state 2 (0x10BB62)  hold a part that is about to open, then fire.
 *   state 3 (0x10BC10)  rts.
 *
 * Jump table 0x10BB2A, byte-exact from work/maincpu.bin:
 *   0 -> 0x10B9D0 (tail), 1 -> 0x10BB3A, 2 -> 0x10BB62, 3 -> 0x10BC10 (rts).
 *
 * Object +0x6 in pair 0 of the capture is 0x01 (0x41954C), so the dispatch at
 * 0x10B9C8 selects 0x10BB3A -- consistent with the ROM. */

#define BOSS_BUDGET_ALIVE 0x000001F4u /* D0 for 0x0D0E04 at 0x10BA06 */
#define BOSS_BUDGET_DIE   0x00001388u /* D0 for 0x0D0E30 at 0x10BA84/0x10BAFE */
#define BOSS_TARGET_LIST  (F3_A5 + 0x08FEu)

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0CF1D8 */
static void sub_0CF1D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44u, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
}

/* ROM 0x0D0678: load the {timer.w, flip.b, pad.b, sprdef.l, moveptr.l}
 * record at A1 and restart the script from it. */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DD8 */
static void sub_0D0DD8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, BOSS_TARGET_LIST);
    wr32(m, a1, obj);
    wr32(m, BOSS_TARGET_LIST, a1 + 4u);
}

/* ROM 0x0D09E8: aim at player 1. The captured path always stores the raw
 * angle in obj+0x58 (obj+0x1D == 0, single target), appends the object to the
 * target list and draws it. */
static void sub_0D09E8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    const uint32_t p = 0x41B190u;
    uint32_t a4;
    uint16_t t, d3, d2, ang;
    int16_t d0;

    if ((int16_t)rd16(m, p + 0x4Cu) < 0)
        return;                                  /* player 1 gone: 0x0D0AA4 */

    a4 = rd32(m, obj + 0x68u) + 2u;
    a1 += 2u;                                    /* 0x0D09FA */

    /* 0x0D09FE: |obj.y - p1.y + tbl - 0xF| */
    t = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u));
    t = (uint16_t)(t + rd16(m, a4));
    t = (uint16_t)(t - 0x0Fu);
    if ((int16_t)t < 0)
        t = (uint16_t)(0u - t);
    d3 = t;

    /* 0x0D0A20: |obj.x - p1.x + tbl - 0x17| */
    t = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u));
    t = (uint16_t)(t + rd16(m, a4));
    t = (uint16_t)(t - 0x17u);
    if ((int16_t)t < 0)
        t = (uint16_t)(0u - t);
    d2 = t;

    /* 0x0D0A3E: Obj_Atan2(dy, dx) -> byte angle, minus (A1).w, wrapped */
    ang = Obj_Atan2(m, d3, d2);
    d0 = (int16_t)(uint16_t)((uint16_t)(ang & 0x00FFu) - rd16(m, a1));
    d0 = (int16_t)(uint8_t)d0;
    if (d0 < 0)
        d0 = (int16_t)(d0 + 0x0168);

    if (rd8(m, a1 + 4u) == 0u) {                 /* 0x0D0AC2: single target */
        if (rd8(m, obj + 0x1Du) == 0u)
            wr16(m, obj + 0x58u, (uint16_t)d0);
        sub_0D0DD8(m, obj);                      /* 0x0D0AE2 */
        Obj_DrawSprite(m, obj);
        return;
    }
    /* 0x0D0AD6 mirror branch: never taken by this capture. */
}

/* ROM 0x10BAC8: the core is released; kill the shared part counter. */
static void boss_dying(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    wr8(m, obj + 0x06u, 0u);
    Obj_RelayTimer(m, BOSS_BUDGET_DIE);          /* ROM 0x0D0E30 */
    a1 = rd32(m, obj + 0x08u);
    {
        uint8_t c = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
        wr8(m, a1 + 0x22u, c);
        if (c != 0u)
            goto done;
    }
    if ((rd8(m, a1 + 0x07u) & 0x01u) != 0u)      /* 0x10BA6C */
        goto done;
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) /* 0x10BA78 */
        goto done;
    Obj_RelayTimer(m, BOSS_BUDGET_DIE);          /* 0x10BAFE */
    (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, BOSS_BUDGET_DIE);
done:
    Obj_StartDying(m, obj);                      /* ROM 0x0CEE74 */
}

/* ROM 0x10BA12: the core is removed; spawn the wreck and release the part. */
static void boss_despawn(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    wr8(m, obj + 0x06u, 0u);
    a1 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
    if ((int32_t)a1 >= 0) {
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x18u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x10u));
        wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    }

    a1 = rd32(m, obj + 0x08u);                   /* 0x10BA5E */
    {
        uint8_t c = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
        wr8(m, a1 + 0x22u, c);
        if (c != 0u)
            goto done;
    }
    if ((rd8(m, a1 + 0x07u) & 0x01u) != 0u)      /* 0x10BA6C */
        goto done;
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) /* 0x10BA78 */
        goto done;
    sub_0D0E04(m, obj, BOSS_BUDGET_DIE);         /* 0x10BA84 */
    (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, BOSS_BUDGET_DIE);
done:
    Obj_StartDying(m, obj);                      /* 0x0CEE74 */
}

/* ROM 0x10B960..0x10B9E8: the per-frame body. Returns 0. */
static int boss_body(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {  /* 0x10B968 */
        boss_despawn(m, obj);
        return 0;
    }

    /* 0x10B972: follow the camera scroll (long subtract, word read later) */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    if (rd8(m, obj + 0x06u) != 2u) {                /* 0x10B97A */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u)
            (void)ObjScript_CF1E4(m, obj);          /* 0x10B98E */
        if (rd8(m, obj + 0x20u) != 0u)              /* 0x10B994 */
            (void)ObjScript_CF4B8(m, obj, 0x0010BC12u);
    }

    Obj_FlagOffscreen(m, obj);                      /* 0x10B9A6 (0x0CF85A) */

    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {      /* 0x10B9AC -> 0x10BAB0 */
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x06u, 0u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10B9B6: draw at the lagged position and run the state */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0: /* 0x10BB3A */
        sub_0D09E8(m, obj, 0x0010BC12u);
        if (rd8(m, obj + 0x23u) != 0u) {            /* 0x10BB44 */
            uint32_t a2 = rd32(m, obj + 0x0Cu);
            sub_0D0678(m, obj, 0x0010BCA4u);
            wr32(m, obj + 0x0Cu, a2);
            wr8(m, obj + 0x06u, 2u);
        }
        break;

    case 2: /* 0x10BB62: hold the part at (dx-3, dy-5) until it opens */
        {
            uint32_t a2 = rd32(m, obj + 0x0Cu);
            uint8_t c;

            wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 3u));
            wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));

            c = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x10BB7A */
            wr8(m, obj + 0x23u, c);
            if (c != 0u)
                break;

            {   /* 0x10BB82: firing angle = Next4Way()&0x3F - 0x20, wrapped */
                uint16_t r = Obj_Next4Way(m);
                int16_t d0 = (int16_t)(uint16_t)((uint16_t)(r & 0x003Fu) -
                                                 (uint16_t)(r & 0x0030u) * 2u -
                                                 0x20u);
                if (d0 < 0)
                    d0 = (int16_t)(d0 + 0x0168);
                wr16(m, obj + 0x58u, (uint16_t)d0);
            }

            sub_0CF1D8(m, obj, 0x0010BC44u);        /* 0x10BB9C */
            sub_0D0678(m, obj, 0x0010BCC8u);        /* 0x10BBA6 */
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
            wr32(m, obj + 0x0Cu, 0u);
            wr8(m, obj + 0x06u, 3u);

            {
                uint16_t d0 = rd16(m, a2 + 0x4Eu);  /* 0x10BBC6 */
                if (d0 == 0u)
                    break;
                wr32(m, obj + 0x0Cu, rd32(m, obj + 0x24u));
                wr16(m, obj + 0x24u, d0);
                wr16(m, a2 + 0x4Eu, 0u);
                {
                    uint32_t a1 = Obj_Spawn(m, obj, 0x0010BD20u, 9u, 2u, 0u);
                    if ((int32_t)a1 < 0)
                        break;
                    wr8(m, a1 + 0x10u,
                        rd8(m, 0x0C8548u + (uint32_t)rd16(m, a2 + 0x22u)));
                }
            }
        }
        break;

    default: /* 1 -> 0x10B9D0, 3 -> 0x10BC10 (rts) */
        break;
    }

    /* 0x10B9D0 tail */
    Obj_StepScript(m, obj);                         /* ROM 0x0D0690 */
    sub_0D0DD8(m, obj);                             /* ROM 0x0D0DD8 */
    Obj_BlinkColour(m, obj);                        /* ROM 0x0CBE34 */
    Obj_DrawSprite(m, obj);                         /* ROM 0x001282 */
    return 0;
}

/* ROM 0x10B93C */
int Obj_10B93C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10B93C: core inside the screen window -> straight to the dying path */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            boss_dying(m, obj);
            return 0;
        }
    }

    /* 0x10B960: no child budget left */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        if (rd8(m, obj + 0x06u) == 3u) {            /* 0x10B9EA */
            wr32(m, obj + 0x24u, rd32(m, obj + 0x0Cu));
            if ((int32_t)rd32(m, obj + 0x24u) != 0) {
                wr32(m, obj + 0x0Cu, 0u);
                return boss_body(m, obj);           /* 0x10BA02 */
            }
        }
        sub_0D0E04(m, obj, BOSS_BUDGET_ALIVE);      /* 0x10BA06 */
        boss_despawn(m, obj);
        return 0;
    }

    return boss_body(m, obj);
}
