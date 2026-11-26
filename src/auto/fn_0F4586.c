/* ROM 0x0F4586: enemy object tick (A0 = obj) with 3D-orbit offset (0x0CF898).
 * Handles debug on-screen kill, HP <= 0 / bomb-clear death (spawning 0x0DC486),
 * scroll Y, optional scripts (0x0CF1E4, 0x0CF4B8), off-screen flag (0x0CF85A),
 * state machine (+0x06: 1=arm, 2=aim/fire, 3=idle, 4=re-anim), orbit step
 * (0x0CF898), script step (0x0D0690), collision (0x0D0DC2) and hit flash (0x0CBE34). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"

#define REC_0F471A 0x000F471Au /* lea (0x13c,PC),A1 at 0x0F45DC & 0x0F46E0 */
#define REC_0F47A6 0x000F47A6u /* lea (0x9c,PC),A1 at 0x0F4708 */
#define REC_0F4812 0x000F4812u /* lea (0x11e,PC),A1 at 0x0F46F2 */

int Fn_0F4586(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    /* 0x0F4586: debug on-screen kill */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));         /* 0x0F459E */
            d0 = 0x00000BB8u;                                 /* 0x0F4696 */
            Score_AddCurrent(m, d0);                          /* 0x0F469C */
            a1 = rd32(m, F3_A5 + 0x1D50u);
            wr8(m, obj + 0x06u, 0u);                          /* 0x0F46A2 */
            Obj_DieWithBonusDbgChk(m, obj, d0);               /* 0x0F46A6 -> 0x0CEE9A */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }
    }

    /* 0x0F45AA: HP <= 0 or bomb-clear */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        d0 = 0x00000BB8u;                                     /* 0x0F462E */
        Score_OnHit(m, obj, d0);                              /* 0x0F4634 */
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {     /* 0x0F45B2 */
        d0 = rd32(m, F3_A5 + 0x4228u);                        /* 0x0F45BC */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);      /* 0x0F45C0 */

        {
            uint8_t f7 = rd8(m, obj + 0x07u);                 /* 0x0F45C4 bclr #1 */
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if ((f7 & 0x02u) != 0u) {
                if (ObjScript_CF1E4(m, obj) < 0)              /* 0x0F45CE */
                    return -1;
            }
        }

        if (rd8(m, obj + 0x20u) != 0u) {                      /* 0x0F45D4 */
            a1 = REC_0F471A;                                  /* 0x0F45DC */
            if (ObjScript_CF4B8(m, obj, a1) < 0)              /* 0x0F45E0 */
                return -1;
        }

        Obj_FlagOffscreen(m, obj);                            /* 0x0F45E6 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {            /* 0x0F45EC */
            wr8(m, obj + 0x06u, 0u);                          /* 0x0F468C */
            Obj_StartDying(m, obj);                           /* 0x0F4690 -> 0x0CEE74 */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));           /* 0x0F45F6 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));           /* 0x0F45FC */

        switch (rd8(m, obj + 0x06u)) {                        /* 0x0F4608 -> table 0x0F46AC */
        case 1:                                               /* 0x0F46C0 */
            Obj_ArmOnScreen(m, obj);
            if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
                wr8(m, obj + 0x06u, 2u);
            break;
        case 2:                                               /* 0x0F46D8 */
            if (rd32(m, obj + 0x68u) != 0u) {
                a1 = REC_0F471A;
                Obj_AimAtPlayers(m, obj, a1);
                if (rd8(m, obj + 0x23u) != 0u) {
                    a1 = REC_0F4812;
                    Obj_SetAnim(m, obj, a1);
                    wr8(m, obj + 0x23u, 0u);
                    wr8(m, obj + 0x06u, 3u);
                }
            }
            break;
        case 3:                                               /* 0x0F4706 */
            break;
        case 4:                                               /* 0x0F4708 */
            a1 = REC_0F47A6;
            Obj_SetAnim(m, obj, a1);
            wr8(m, obj + 0x06u, 2u);
            break;
        default:
            break;
        }

        if (Obj_0CF898(m, obj, &d0, &d1) < 0)                 /* 0x0F4610 */
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);      /* 0x0F4616 */
        {
            uint16_t old_cnt = rd16(m, obj + 0x4Cu);
            Obj_StepScript(m, obj);                           /* 0x0F461A */
            if (old_cnt == 1u) {
                uint32_t scr = rd32(m, obj + 0x48u);
                uint16_t w = rd16(m, scr);
                if ((int16_t)w < 0)
                    d0 = (d0 & 0xFFFF0000u) | w;
                else
                    d0 = (d0 & 0xFFFF0000u) | rd16(m, scr + 2u);
            }
        }
        Coll_AddEnemyHazard(m, obj);                          /* 0x0F4620 */
        a1 = rd32(m, F3_A5 + 0x0906u);
        Obj_HitFlash(m, obj);                                 /* 0x0F4626 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    /* 0x0F463A: death spawn */
    d0 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));            /* 0x0F465E */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x001Cu)); /* 0x0F4664 */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));            /* 0x0F466A */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u)); /* 0x0F4670 */
        wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));            /* 0x0F4676 */
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));            /* 0x0F467C */
    }
    wr8(m, obj + 0x06u, 0u);                                  /* 0x0F4682 */
    Obj_DieWithBonusChk(m, obj, d0);                          /* 0x0F4686 -> 0x0CEE48 */
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
