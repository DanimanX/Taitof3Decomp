#include "obj_F1432.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "obj_E8B72.h" /* Score_OnHit, Score_AddCurrent, Obj_AimAtPlayers */

/* ROM 0x0CEE52: drop the carried bonus 0x117C28 (class byte +0x4E staged in
 * D0.b) and die */
static void sub_0CEE52(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | (uint32_t)b;
        (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    Obj_StartDying(m, obj); /* 0x0CEE74 */
}

/* ROM 0x0CEE48 */
static void sub_0CEE48(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj); /* 0x0CEE74 */
        return;
    }
    sub_0CEE52(m, obj, d0);
}

/* ROM 0x0CEE9A + 0x0CEEA2: drop the bonus, then either arm the screen-wide
 * explode tick (0x0CEEF0) or park the object off screen. */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        return; /* 0x0CEE74 */

    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | (uint32_t)b;
        (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj); /* 0x0CBB2C */
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEEF0u);
}

/* ROM 0x0CF7FE: re-arm once the sprite's +0x3A re-enters [-0x40,0x14F] */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x0040 || x > 0x014F)
        return;
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x80u)); /* bclr.b #7 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));           /* bset.b #1 */
}

/* ROM 0x0F15FA (jump table 0x0F15EE slot 0) */
static void sub_0F15FA(const f3mem_t *m, uint32_t obj)
{
    sub_0CF7FE(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x80u) != 0u)
        return;
    Obj_StartBehaviour(m, obj, 0x000F1768u); /* 0x0F160E: lea (0x156,PC),A1 */
    wr8(m, obj + 0x06u, 0x01u);
}

/* ROM 0x0F161C (slot 1): aim through the behaviour's +0xC box, fire 0x0D9092
 * once the +0x23 delay runs out, then hold for 0x1E frames. */
static void sub_0F161C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x23u) == 0u) {
        if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
            return;
        Obj_AimAtPlayers(m, obj, rd32(m, obj + 0x68u) + 0x0Cu); /* 0x0F162E: adda.w #0xC,A1 */
        if (rd8(m, obj + 0x23u) == 0u)
            return;
    }
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) != 0u)
        return;

    uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u); /* 0x0F1648 */
    if ((int32_t)a1 < 0)
        return;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
    wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));
    wr16(m, a1 + 0x10u, 0u);
    wr8(m, obj + 0x23u, 0x1Eu);
    wr8(m, obj + 0x06u, 0x02u);
}

/* ROM 0x0F169C (slot 2) */
static void sub_0F169C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) == 0u)
        wr8(m, obj + 0x06u, 0x01u);
}

/* 0x0F14B4: jsr ([0x138,PC],D0w*4) over table 0x0F15EE; exactly three slots */
static void sub_0F15EE(const f3mem_t *m, uint32_t obj, uint8_t state)
{
    switch (state) {
    case 0u: sub_0F15FA(m, obj); break;
    case 1u: sub_0F161C(m, obj); break;
    case 2u: sub_0F169C(m, obj); break;
    default: break;
    }
}

/* ROM 0x0F1432: the 0x117B44 gun turret's tick. Damage byte +0x24 decides
 * between the live body (0x0F145E) and the death path (0x0F14D6). */
int Obj_F1432_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0F1432: the screen-wide explode branch, turret re-drawn in place */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj); /* 0x0F1450 jmp 0x0F1590 */
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x0F14D6 */
        Score_OnHit(m, obj, 0x000001F4u);                          /* 0x0D0E04 */
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u); /* 0x0F14E2 */
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }

        /* 0x0F152A: hand the slot back to the parent */
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        if (rd8(m, parent + 0x22u) != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
            sub_0CEE48(m, obj, 0u); /* 0x0F1532 / 0x0F153E */
        } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            /* 0x0CEE74 reached directly */
        } else {
            Score_AddCurrent(m, 0x00001388u); /* 0x0F1556 */
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00001388u);
            sub_0CEE52(m, obj, 0u); /* 0x0F1576 bra 0x0CEE52 */
        }

        /* 0x0F157C tail is only entered from the 0x0F14A2 arm path; the
         * branches above all leave through 0x0CEExx which does not return
         * here, so the tick stays as it was. */
        return 0;
    }

    /* 0x0F145E */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        /* bra 0x0F14E2: drop the bonus first */
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
        sub_0CEE9A(m, obj, 0u); /* 0x0F152A tail -> 0x0CEF9A */
        return 0;
    }

    /* 0x0F1468 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u)); /* bclr.b #1 */
        if ((f7 & 0x02u) != 0u)
            (void)ObjScript_CF1E4(m, obj); /* 0x0CF1E4 */
    }
    if (rd8(m, obj + 0x20u) != 0u)
        (void)ObjScript_CF4B8(m, obj, 0x000F16B0u); /* 0x0F1488: lea (0x222,PC),A1 */

    Obj_FlagOffscreen(m, obj); /* 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        /* 0x0F157C: the parent lets go of this slot */
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u)); /* bset.b #0 */
        Obj_StartDying(m, obj); /* 0x0CEE74 */
        return 0;
    }

    /* 0x0F14A2 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    sub_0F15EE(m, obj, rd8(m, obj + 0x06u)); /* 0x0F14B4 */
    Obj_StepScript(m, obj);                  /* 0x0F14BC */
    Obj_DrawSprite(m, obj);                  /* 0x0F14CE */
    return 0;
}
