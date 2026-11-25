#include "obj_D6242.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* cmp2.w (addr),Dn; bcs -> outside the signed bounds {lo.w, hi.w} at addr */
static int out_of_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr);
    int16_t hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x0CC2BC: stage-speed-scaled modulo tick gate. Returns (rem, quot). */
static void gate_0CC2BC(const f3mem_t *m, uint16_t d0, uint16_t *rem_out, uint16_t *quot_out)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0 * (uint32_t)d1;
    uint16_t div = (uint16_t)((prod << 8) >> 16);
    if (div == 0u)
        div = 1u;
    uint32_t num = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    *quot_out = (uint16_t)(num / div);
    *rem_out = (uint16_t)(num % div);
}

static void kill_enemy(const f3mem_t *m, uint32_t obj)
{
    int16_t cmd = Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u));
    Sound_PlayCmd(m, cmd);                         /* 0x0D63C4 / 0x0D63CA */
    uint32_t arg = Obj_13E4D0(m, obj, 2u);         /* 0x0D63D6 */
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);   /* 0x0D63EC */
    wr32(m, obj + 0x00u, 0x000D63FEu);             /* 0x0D63F6 */
}

static int bomb_explode_check(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return 0;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -0x0A || x > 0x11C)
        return 0;
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr32(m, obj + 0x10u, 0x0005D84Cu);             /* 0x0D640C */
    Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
    Obj_ExplodeTick(m, obj);
    return 1;
}

/* ROM 0x0D6242: enemy animation, firing, and death/bomb handling. */
int Obj_D6242_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);          /* 0x0D63AC */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        kill_enemy(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        kill_enemy(m, obj);                        /* 0x0D6250 */
        return 0;
    }

    Obj_AnimFrameStep(m, obj, 0x000D6426u);        /* 0x0D625A */
    if ((int16_t)rd16(m, 0x000D6426u + rd16(m, obj + 0x3Au)) > 0) {
        if (bomb_explode_check(m, obj))
            return 0;
        Obj_SubScrollVel(m, obj);                  /* 0x0D628C */
        Obj_0D0DC2(m, obj);                        /* 0x0D6292 */
        Obj_BlinkColour(m, obj);                   /* 0x0D6298 */
        Obj_DrawSprite(m, obj);                    /* 0x0D629E */
        wr32(m, obj + 0x00u, 0x000D6242u);         /* 0x0D62A4 -> 0x0D623A */
        return 0;
    }

    /* Phase transition into 0x0D62C8 */
    wr16(m, obj + 0x2Cu, 0x000Au);                 /* 0x0D62A6 */
    wr16(m, obj + 0x30u, 0x000Cu);                 /* 0x0D62AC */
    wr16(m, obj + 0x2Eu, 0x0025u);                 /* 0x0D62B2 */
    wr16(m, obj + 0x32u, 0x001Fu);                 /* 0x0D62B8 */
    wr16(m, obj + 0x3Au, 0x000Cu);                 /* 0x0D62BE */
    wr16(m, obj + 0x3Eu, 0u);                      /* 0x0D62C4 */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        kill_enemy(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        kill_enemy(m, obj);
        return 0;
    }

    Obj_AnimFrameStep(m, obj, 0x000D6464u);        /* 0x0D62E0 */
    if ((int16_t)rd16(m, 0x000D6464u + rd16(m, obj + 0x3Au)) <= 0) {
        if (bomb_explode_check(m, obj))
            return 0;
        uint16_t rem, quot;
        gate_0CC2BC(m, 8u, &rem, &quot);           /* 0x0D6316 */
        if (rem == 0u && (quot & 7u) < 4u) {
            Obj_Spawn(m, obj, 0x000D9056u, 9u, 2u, 0x0B139B00u); /* 0x0D6346 */
            Obj_Spawn(m, obj, 0x000D9056u, 9u, 2u, 0x2513E500u); /* 0x0D6366 */
            wr16(m, obj + 0x3Au, 0u);
            wr16(m, obj + 0x3Eu, 0u);
        }
    }

    Obj_SubScrollVel(m, obj);                      /* 0x0D6378 */
    Obj_0D0DC2(m, obj);                            /* 0x0D637E */
    Obj_BlinkColour(m, obj);                       /* 0x0D6384 */
    Obj_DrawSprite(m, obj);                        /* 0x0D638A */
    if (out_of_bounds(m, 0x000CC112u, rd16(m, obj + 0x14u))) {
        Obj_Free(m, obj);                          /* 0x0D639C -> 0x0D63FE */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x000D63A8u);             /* 0x0D63A0 */
    return 0;
}
