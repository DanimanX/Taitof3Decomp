/* ROM 0x0F1F48: turret/gun enemy tick (A0 = object).
 * Handles debug-kill check (A5+0x1D58 bit 1, X in [-0x0A, 0x11C]), HP <= 0 /
 * bomb-clear death (spawning 0x0DC56E and 0x117B44 when parent+0x22 hits 0),
 * vertical scroll (A5+0x4228), script steps (0x0CF1E4, 0x0CF4B8 with 0x0F21A0),
 * offscreen check (0x0CF85A), 2-state aim/fire machine (0x0F2104: arm on
 * screen, aim & spawn 0x0D9092 shot), then script/collision/flash/draw. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

#define REC_0F21A0 0x000F21A0u
#define REC_0F21D8 0x000F21D8u
#define REC_0F229A 0x000F229Au

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F1F48(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0F1F48: debug-kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0F1F60 */
            r->d[0] = 0x00000190u;                            /* 0x0F20A6 */
            Score_AddCurrent(m, 0x00000190u);
            uint32_t parent = rd32(m, obj + 0x08u);           /* 0x0F20B2 */
            r->a[1] = parent;
            uint8_t rem = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
            wr8(m, parent + 0x22u, rem);
            if (rem == 0u && (rd8(m, parent + 0x07u) & 0x01u) == 0u) {
                if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                    Obj_StartDying(m, obj);
                    return 0;
                }
                r->d[0] = 0x00000FA0u;
                Score_AddCurrent(m, 0x00000FA0u);
                r->d[0] = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
                Obj_DieWithBonusDbg(m, obj, r->d[0]);
                return 0;
            }
            Obj_DieWithBonusDbgChk(m, obj, r->d[0]);
            return 0;
        }
    }

    /* 0x0F1F6C: HP <= 0 or bomb-clear (A5-0x144B bit 5) */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        r->d[0] = 0x00000190u;                                /* 0x0F1FEC */
        Obj_0D0E04(m, obj, 0x00000190u);
        goto die_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)              /* 0x0F1F74 */
        goto die_spawn;

    /* 0x0F1F7E: vertical scroll and scripts */
    {
        uint32_t dy = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - dy);
        r->d[0] = dy;
    }
    {
        uint8_t f7 = rd8(m, obj + 0x07u);                     /* 0x0F1F86 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)                  /* 0x0F1F90 */
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {                          /* 0x0F1F96 */
        r->a[1] = REC_0F21A0;
        if (ObjScript_CF4B8(m, obj, REC_0F21A0) < 0)          /* 0x0F1FA2 */
            return -1;
    }

    Obj_FlagOffscreen(m, obj);                                /* 0x0F1FA8 -> 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {                /* 0x0F1FAE */
        uint32_t parent = rd32(m, obj + 0x08u);               /* 0x0F2092 */
        r->a[1] = parent;
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);                               /* 0x0F20A0 */
        return 0;
    }

    /* 0x0F1FB8: sync coords and dispatch state (jump table 0x0F2104) */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    {
        uint8_t st = rd8(m, obj + 0x06u);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | st;
        switch (st) {
        case 0:                                               /* 0x0F210C */
            r->a[1] = REC_0F21A0;
            Obj_AimAtPlayers(m, obj, REC_0F21A0);
            if (rd8(m, obj + 0x23u) != 0u) {
                r->a[1] = REC_0F229A;
                Obj_StartBehaviour(m, obj, REC_0F229A);
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
                wr8(m, obj + 0x06u, 1u);
            }
            break;
        case 1:                                               /* 0x0F2136 */
            if (rd8(m, obj + 0x23u) != 0u) {
                uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                wr8(m, obj + 0x23u, t);
                if (t != 0u)
                    break;
            }
            r->a[1] = REC_0F21D8;                             /* 0x0F2146 */
            Obj_AimAtPlayers(m, obj, REC_0F21D8);
            if (rd8(m, obj + 0x23u) != 0u) {
                uint32_t child = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
                r->d[0] = child;
                if ((int32_t)child >= 0) {
                    r->a[1] = child;
                    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u));
                    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
                    wr16(m, child + 0x58u, rd16(m, obj + 0x42u));
                    wr16(m, child + 0x10u, 0u);
                }
            }
            break;
        default:
            break;
        }
    }

    /* 0x0F1FD2: script, collision, flash, draw */
    step_script_regs(m, obj, &r->d[0], &r->a[1]);
    Obj_0D0DC2(m, obj);
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    Obj_HitFlash(m, obj);
    draw_sprite_regs(m, obj, r);
    return 0;

die_spawn:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC56Eu, 3u, 2u, 0u);
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
            wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
        }
        uint32_t parent = rd32(m, obj + 0x08u);               /* 0x0F2040 */
        r->a[1] = parent;
        uint8_t rem = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
        wr8(m, parent + 0x22u, rem);
        if (rem == 0u && (rd8(m, parent + 0x07u) & 0x01u) == 0u) {
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            r->d[0] = 0x00000FA0u;
            Obj_0D0E04(m, obj, 0x00000FA0u);
            r->d[0] = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
            Obj_DieWithBonus(m, obj, r->d[0]);
            return 0;
        }
        Obj_DieWithBonusChk(m, obj, r->d[0]);
        return 0;
    }
}
