#include "obj_10A484.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* Movement record handed to 0x0CF4B8 (lea 0x24A,PC at 0x10A4DA). */
static const uint8_t k_move_rec[0x1A] = { 0 };

/* ROM 0x0D0DC2: append obj to A5+0x8FE, then Coll_AddHazard (0x0D0DCC) */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    Coll_AddEnemyHazard(m, obj);
}

/* Jump table 0x10A650 indexed by obj+0x6:
 *   0 -> 0x10A6C0 (nothing)  1 -> 0x10A664 (arm)   2 -> 0x10A6C2 (shot)
 *   3 -> 0x10A700 (switch to anim 2)  4/5 -> 0x10A6FE (nothing)
 * Returns the new obj+0x6 value to request for the next frame. */

/* ROM 0x10A664: within 0x96 of a live player -> arm and go to state 3 */
static uint8_t sub_10A664(const f3mem_t *m, uint32_t obj)
{
    int16_t d0;
    uint32_t a1 = 0x0041B190u;
    if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
        d0 = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) - rd16(m, a1 + 0x14u) - 1u);
        if (d0 < 0)
            d0 = (int16_t)-d0;
        if ((uint16_t)d0 <= 0x0096u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x02u)); /* bclr.b #1 */
            return 3u;                                   /* 0x10A6BA */
        }
    }
    /* 0x10A68C */
    a1 = 0x0041B21Au;
    if ((int16_t)rd16(m, a1 + 0x4Cu) < 0)
        return rd8(m, obj + 0x06u);
    d0 = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) - rd16(m, a1 + 0x14u) - 1u);
    if (d0 < 0)
        d0 = (int16_t)-d0;
    if ((uint16_t)d0 > 0x0096u)
        return rd8(m, obj + 0x06u);
    /* 0x10A6B4 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x02u)); /* bclr.b #1 */
    return 3u;
}

/* ROM 0x10A6C2: the child detonated -> spawn 0x0DB26C */
static void sub_10A6C2(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        return;
    wr8(m, obj + 0x22u, 0u);
    uint32_t a1 = Obj_Spawn(m, obj, 0x000DB26Cu, 9u, 2u, 0u);
    if ((int32_t)a1 < 0)
        return;
    wr16(m, a1 + 0x10u, 0x0010u);
    wr16(m, a1 + 0x36u, 0x0014u);
}

/* ROM 0x10A700: switch to animation 2 (0x10EAFC) */
static void sub_10A700(const f3mem_t *m, uint32_t obj)
{
    Obj_SetAnim(m, obj, 0x0010EAFCu);
    wr8(m, obj + 0x06u, 5u);
}

/* ROM 0x10A484 */
int Obj_10A484_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10A484: on screen -> tail at 0x10A5EA */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x10A5EA */
            wr8(m, obj + 0x06u, 0u);
            Score_AddCurrent(m, 0x00000FA0u);
            uint32_t a1 = rd32(m, obj + 0x08u);
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
            if (rd8(m, a1 + 0x22u) != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0u);    /* 0x0CEE9A */
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);                /* 0x0CEE74 */
                return 0;
            }
            Score_AddCurrent(m, 0x00009C40u);
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00009C40u);
            Obj_DieWithBonusDbg(m, obj, 0x00009C40u); /* 0x0CEEA2 */
            return 0;
        }
    }

    /* 0x10A4A8 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x10A528 */
        Score_OnHit(m, obj, 0x00000FA0u);
        goto dying;                                    /* 0x10A534 */
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto dying;                                    /* 0x10A534 */

    /* 0x10A4BA */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    /* 0x10A4C2 */
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));   /* bclr.b #1 */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
    }
    /* 0x10A4D2 */
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, (uint32_t)(uintptr_t)k_move_rec) < 0)
            return -1;
    }

    /* 0x10A4E4 */
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        /* 0x10A5D2 */
        wr8(m, obj + 0x06u, 0u);
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u)); /* bset.b #0 */
        Obj_StartDying(m, obj);                        /* 0x0CEE74 */
        return 0;
    }

    /* 0x10A4F4 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    /* 0x10A500: jsr (0x10A650, D0w*4) with D0b = obj+0x6 */
    switch (rd8(m, obj + 0x06u)) {
    case 1u:  (void)sub_10A664(m, obj); break;
    case 2u:  sub_10A6C2(m, obj); break;
    case 3u:  sub_10A700(m, obj); break;
    case 0u: case 4u: case 5u: default: break;
    }

    /* 0x10A50E */
    Obj_StepScript(m, obj);
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;

dying:
    /* 0x10A534 */
    wr8(m, obj + 0x06u, 0u);
    {
        /* pea 0 / move.w #2,-(SP) / move.w #3,-(SP) / move.l #0xDC56E,-(SP) */
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC56Eu, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x16u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x10u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
    }
    /* 0x10A580 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        if (rd8(m, a1 + 0x22u) != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
            Obj_DieWithBonusChk(m, obj, 0u);           /* 0x0CEE48 */
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);                    /* 0x0CEE74 */
            return 0;
        }
        Score_OnHit(m, obj, 0x00009C40u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00009C40u);
        Obj_DieWithBonus(m, obj, 0x00009C40u);         /* 0x0CEE52 */
    }
    return 0;
}
