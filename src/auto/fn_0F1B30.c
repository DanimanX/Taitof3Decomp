/* ROM 0x0F1B30: turret/gun enemy tick (A0 = object).
 * Handles debug-kill check (A5+0x1D58 bit 1, X in [-10, 0x11C]), HP/bomb
 * death (spawning 0x0DC486 child + 0x0CEE74), vertical scroll (A5+0x4228),
 * script steps (0x0CF1E4, 0x0CF4B8 with record 0x0F16AC), offscreen check
 * (0x0CF85A), 3-state firing machine (0x0F1C44: arm on screen, aim & spawn
 * 0x0D9092 shot, cooldown), then animation/collision/flash/draw. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

#define REC_0F16AC 0x000F16ACu  /* lea (-0x4DC,PC),A1 at 0x0F1B86 / (-0x5C6,PC) at 0x0F1C70 */

int Fn_0F1B30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0F1B30: debug kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0F1B48 */
            r->d[0] = 0x000001F4u;                            /* 0x0F1C32 */
            Score_AddCurrent(m, 0x000001F4u);                 /* 0x0F1C38 -> 0x0D0E30 */
            /* 0x0F1C3E bra.l 0x0CEF0E */
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {    /* 0x0CEF0E */
                Obj_StartDying(m, obj);                       /* 0x0CEE74 */
                return 0;
            }
            if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
                x >= -0x0A && x <= 0x011C) {                  /* 0x0CEF18..0x0CEF2E */
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));    /* 0x0CEF30 */
                Obj_OrbitDebug(m, obj);                       /* 0x0CEF36 -> 0x0CBB2C */
                return 0;
            }
            wr32(m, obj, 0x000CEF44u);                        /* 0x0CEF3C */
            return 0;
        }
    }

    /* 0x0F1B54: HP <= 0 or bomb flag (A5-0x144B bit 5) */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        r->d[0] = 0x000001F4u;                                /* 0x0F1BD6 */
        Score_OnHit(m, obj, 0x000001F4u);                     /* 0x0F1BDC -> 0x0D0E04 */
        goto die_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {            /* 0x0F1B5C */
        goto die_spawn;
    }

    /* 0x0F1B66: scroll Y and script steps */
    {
        uint32_t d0 = rd32(m, F3_A5 + 0x4228u);               /* 0x0F1B66 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);      /* 0x0F1B6A */
        r->d[0] = d0;
    }

    {
        uint8_t f7 = rd8(m, obj + 0x07u);                     /* 0x0F1B6E bclr #1 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)                  /* 0x0F1B78 */
                return -1;
        }
    }

    if (rd8(m, obj + 0x20u) != 0u) {                          /* 0x0F1B7E */
        r->a[1] = REC_0F16AC;                                 /* 0x0F1B86 */
        if (ObjScript_CF4B8(m, obj, REC_0F16AC) < 0)          /* 0x0F1B8A */
            return -1;
    }

    Obj_FlagOffscreen(m, obj);                                /* 0x0F1B90 -> 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {                /* 0x0F1B96 */
        Obj_StartDying(m, obj);                               /* 0x0CEE74 */
        return 0;
    }

    /* 0x0F1BA2: sync coords and run state machine (jump table 0x0F1C44) */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {                            /* 0x0F1BB0..0x0F1BB4 */
    case 0:                                                   /* 0x0F1C50 */
        Obj_ArmOnScreen(m, obj);                              /* 0x0CF7FE */
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)              /* 0x0F1C56 */
            wr8(m, obj + 0x06u, 1u);                          /* 0x0F1C60 */
        break;
    case 1:                                                   /* 0x0F1C68 */
        if (rd8(m, obj + 0x23u) == 0u) {
            Obj_AimAtPlayers(m, obj, REC_0F16AC);             /* 0x0F1C70..0x0F1C74 */
            if (rd8(m, obj + 0x23u) == 0u)                    /* 0x0F1C7A */
                break;
        }
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);  /* 0x0F1C82 */
            wr8(m, obj + 0x23u, t);
            if (t == 0u) {
                uint32_t child = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u); /* 0x0F1C8A..0x0F1C9C */
                if ((int32_t)child >= 0) {                    /* 0x0F1CA6 */
                    wr32(m, child + 0x14u, rd32(m, obj + 0x14u)); /* 0x0F1CAE */
                    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u)); /* 0x0F1CB4 */
                    wr32(m, child + 0x18u, rd32(m, obj + 0x18u)); /* 0x0F1CBA */
                    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u)); /* 0x0F1CC0 */
                    wr16(m, child + 0x58u, rd16(m, obj + 0x42u)); /* 0x0F1CC6 */
                    wr16(m, child + 0x10u, 0u);               /* 0x0F1CCC */
                    wr8(m, obj + 0x23u, 0x1Eu);               /* 0x0F1CD0 */
                    wr8(m, obj + 0x06u, 2u);                  /* 0x0F1CD6 */
                }
            }
        }
        break;
    case 2:                                                   /* 0x0F1CDE */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);  /* 0x0F1CDE */
            wr8(m, obj + 0x23u, t);
            if (t == 0u)
                wr8(m, obj + 0x06u, 1u);                      /* 0x0F1CE6 */
        }
        break;
    default:
        return -1;
    }

    /* 0x0F1BBC..0x0F1BD4 */
    Obj_StepScript(m, obj);                                   /* 0x0D0690 */
    Coll_AddEnemyHazard(m, obj);                              /* 0x0D0DC2 */
    Obj_HitFlash(m, obj);                                     /* 0x0CBE34 */
    Obj_DrawSprite(m, obj);                                   /* 0x001282 */
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
    r->d[0] = 0x0000FFFFu;
    r->d[1] = 0x0001FFFFu;
    return 0;

die_spawn:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u); /* 0x0F1BE2..0x0F1BF4 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;                                  /* 0x0F1C06 */
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));     /* 0x0F1C08 */
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u)); /* 0x0F1C0E */
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));     /* 0x0F1C14 */
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u)); /* 0x0F1C1A */
            wr16(m, child + 0x58u, rd16(m, obj + 0x58u));     /* 0x0F1C20 */
            wr16(m, child + 0x60u, rd16(m, obj + 0x60u));     /* 0x0F1C26 */
        }
        Obj_StartDying(m, obj);                               /* 0x0F1C2C -> 0x0CEE74 */
        return 0;
    }
}
