#include "obj_109BB6.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* Aiming record handed to 0x0D09B8 / 0x0CF4B8 (lea 0x1AE,PC / lea 0x194,PC
 * from 0x109CD2 / 0x109CEC). The callers overwrite its {dx,dy} aim offset
 * before the jmp 0x0CEEC4 tail runs. */
static const uint8_t k_aim_rec[0x1A] = { 0 };

/* ROM 0x0D0DC2: append obj to A5+0x8FE, then Coll_AddHazard (0x0D0DCC) */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    Coll_AddEnemyHazard(m, obj);
}

/* ROM 0x109BB6 */
int Obj_109BB6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x109BB6: on screen -> tail at 0x109E24 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            uint32_t a1;
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x109E24 */
            Score_AddCurrent(m, 0x0000012Cu);
            a1 = rd32(m, obj + 0x08u);
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
            if (rd8(m, a1 + 0x22u) != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0u);   /* 0x0CEE9A */
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);               /* 0x0CEE74 */
                return 0;
            }
            Score_AddCurrent(m, 0x00000BB8u);
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000BB8u);
            Obj_DieWithBonusDbg(m, obj, 0x00000BB8u); /* 0x0CEEA2 */
            return 0;
        }
    }

    /* 0x109BDA */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x109D6A */
        Score_OnHit(m, obj, 0x0000012Cu);
        goto dying;                                    /* 0x109D76 */
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto dying;                                    /* 0x109D76 */

    /* 0x109BEC */
    if ((int16_t)rd16(m, obj + 0x3Au) < -0x20) {
        /* 0x109E10 */
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u)); /* bset.b #0 */
        Obj_StartDying(m, obj);                        /* 0x0CEE74 */
        return 0;
    }

    /* 0x109BF6 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    /* 0x109BFE */
    {
        uint32_t a1 = 0x0041B190u;
        int16_t dy;      /* D0 */
        int16_t dx;      /* D0 */

        if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
            dy = (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a1 + 0x18u) + 1u);
            if (dy < 0)
                dy = (int16_t)-dy;
            if ((uint16_t)dy <= 0x000Au)
                goto lock_x;                           /* 0x109C9E */
            dx = (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a1 + 0x14u) - 7u);
            if (dx < 0)
                dx = (int16_t)-dx;
            if ((uint16_t)dx <= 0x0019u)
                goto lock_y;                           /* 0x109C82 */
        }

        /* 0x109C40 */
        a1 = 0x0041B21Au;
        if ((int16_t)rd16(m, a1 + 0x4Cu) < 0)
            goto no_lock;                              /* 0x109CBA */
        dy = (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a1 + 0x18u) + 1u);
        if (dy < 0)
            dy = (int16_t)-dy;
        if ((uint16_t)dy <= 0x000Au)
            goto lock_x;                               /* 0x109C9E */
        dx = (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a1 + 0x14u) - 7u);
        if (dx < 0)
            dx = (int16_t)-dx;
        if ((uint16_t)dx > 0x0019u)
            goto no_lock;                              /* 0x109CBA */

        /* 0x109C82: step Y towards this player by 2 */
    lock_y: {
        int16_t step = 2;
        int16_t d = (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a1 + 0x18u) + 1u);
        if (d < 0)
            step = (int16_t)-step;
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - (uint16_t)step));
        goto ready;                                    /* 0x109D44 */
    }

        /* 0x109C9E: step X towards this player by 2 */
    lock_x: {
        int16_t step = 2;
        int16_t d = (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a1 + 0x14u) - 7u);
        if (d < 0)
            step = (int16_t)-step;
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - (uint16_t)step));
        goto ready;                                    /* 0x109D44 */
    }

        /* 0x109CBA */
    no_lock: {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));   /* bclr.b #1 */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        /* 0x109CCA */
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, (uint32_t)(uintptr_t)k_aim_rec) < 0)
                return -1;
        }
        /* 0x109CDC */
        if (rd8(m, obj + 0x23u) != 0u) {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                goto ready;                            /* 0x109D44 */
        }
        /* 0x109CEC */
        Obj_AimAtPlayers(m, obj, (uint32_t)(uintptr_t)k_aim_rec);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint32_t a2 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)a2 >= 0) {
                wr32(m, a2 + 0x14u, rd32(m, obj + 0x3Au));
                wr16(m, a2 + 0x14u, (uint16_t)(rd16(m, a2 + 0x14u) + 0x10u));
                wr32(m, a2 + 0x18u, rd32(m, obj + 0x3Eu));
                wr16(m, a2 + 0x18u, (uint16_t)(rd16(m, a2 + 0x18u) + 0x10u));
                wr16(m, a2 + 0x58u, rd16(m, obj + 0x42u));
                wr16(m, a2 + 0x10u, 0u);
            }
        }
    }

    /* 0x109D44 */
ready:
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        Obj_StepScript(m, obj);
        sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

dying:
    /* 0x109D76: pea 0 / move.w #2,-(SP) / move.w #3,-(SP) / move.l #0xDC486,-(SP) */
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x10u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x10u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
    }

    /* 0x109DBE */
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
        Score_OnHit(m, obj, 0x00000BB8u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000BB8u);
        Obj_DieWithBonus(m, obj, 0x00000BB8u);         /* 0x0CEE52 */
    }
    return 0;
}
