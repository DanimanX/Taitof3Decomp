/* ROM 0x0DE96C: orbiting-subpart enemy tick (A0 = object).
 * On bomb-hit (A5+0x1D58 bit 1 with X in [-10, 284]) awards 0x12C timer via
 * 0x0D0E30, clears +0x06, and tail-calls 0x0CEF0E. On HP <= 0 awards 0x12C
 * via 0x0D0E04; on death or A5-0x144B bit 5 spawns 0x0DC328, clears +0x06,
 * and calls Obj_StartDying (0x0CEE74). Otherwise scrolls +0x3E by A5+0x4228,
 * steps ObjScript_CF1E4 / ObjScript_CF4B8 (aim 0x0DEAF4), checks on/offscreen
 * (0x0CF7FE / 0x0CF85A), syncs (+0x14, +0x18), dispatches +0x06 via 0x0DEA86,
 * adds 0x0CF898 D1 offset to +0x18, adds to collision lists (0x0D0DC2), and
 * blinks colour (0x0CBE34). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"

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

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

int Fn_0DE96C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0DE96C */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0DE984 */
            d0 = 0x0000012Cu;                           /* 0x0DEA70 */
            Obj_RelayTimer(m, d0);                      /* 0x0DEA76 */
            wr8(m, obj + 0x06u, 0u);                    /* 0x0DEA7C */
            Obj_0CEF0E(m, obj);                         /* 0x0DEA80 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0DE990 */
        d0 = 0x0000012Cu;                               /* 0x0DEA14 */
        Obj_0D0E04(m, obj, d0);                         /* 0x0DEA1A */
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0DE998 */
        goto death_spawn;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0DE9A2 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0DE9A6 */

    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));    /* 0x0DE9AA */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0DE9B4 */
                return -1;
        }
    }

    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x0DE9BA */
        a1 = 0x000DEAF4u;                               /* 0x0DE9C2 */
        if (ObjScript_CF4B8(m, obj, a1) < 0)            /* 0x0DE9C6 */
            return -1;
    }

    Obj_ArmOnScreen(m, obj);                            /* 0x0DE9CC */
    Obj_FlagOffscreen(m, obj);                          /* 0x0DE9D2 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0DE9D8 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0DEA66 */
        Obj_StartDying(m, obj);                         /* 0x0DEA6A */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0DE9E2 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0DE9E8 */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0DE9EE..0x0DE9F0 */
    switch ((uint16_t)d0) {                             /* 0x0DE9F4 -> 0x0DEA86 */
    case 1u:
    case 5u:
        step_script_regs(m, obj, &d0, &a1);             /* 0x0DEA9E */
        break;
    case 2u:
        a1 = 0x000DEC7Au;                               /* 0x0DEAA6 */
        Obj_StartBehaviour(m, obj, a1);                 /* 0x0DEAAA */
        a1 += 8u;
        wr8(m, obj + 0x06u, 3u);                        /* 0x0DEAB0 */
        break;
    case 3u:
        step_script_regs(m, obj, &d0, &a1);             /* 0x0DEAB8 */
        if (rd16(m, obj + 0x4Cu) == 0u) {               /* 0x0DEABE */
            a1 = 0x000DEC56u;                           /* 0x0DEAC6 */
            Obj_StartBehaviour(m, obj, a1);             /* 0x0DEACA */
            a1 += 8u;
            wr8(m, obj + 0x06u, 1u);                    /* 0x0DEAD0 */
        }
        break;
    case 4u:
        a1 = 0x000DEC44u;                               /* 0x0DEAD8 */
        Obj_StartScript(m, obj, a1);                    /* 0x0DEADC */
        a1 = 0x000DEC56u;                               /* 0x0DEAE2 */
        Obj_StartBehaviour(m, obj, a1);                 /* 0x0DEAE6 */
        a1 += 8u;
        wr8(m, obj + 0x06u, 5u);                        /* 0x0DEAEC */
        break;
    default:
        break;
    }

    if (Obj_0CF898(m, obj, &d0, &d1) < 0)               /* 0x0DE9FC */
        return -1;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);    /* 0x0DEA02 */
    add_coll_regs(m, obj, &a1);                         /* 0x0DEA06 */
    Obj_BlinkColour(m, obj);                            /* 0x0DEA0C */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_spawn:
    d0 = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u);    /* 0x0DEA32 */
    if ((int32_t)d0 >= 0) {
        a1 = d0;                                        /* 0x0DEA42 */
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));      /* 0x0DEA44 */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u)); /* 0x0DEA4A */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));      /* 0x0DEA50 */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0008u)); /* 0x0DEA56 */
        wr16(m, a1 + 0x58u, 0x00B4u);                   /* 0x0DEA5A */
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));      /* 0x0DEA60 */
    }
    wr8(m, obj + 0x06u, 0u);                            /* 0x0DEA66 */
    Obj_StartDying(m, obj);                             /* 0x0DEA6A */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
