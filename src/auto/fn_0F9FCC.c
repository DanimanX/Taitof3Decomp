/* ROM 0x0F9FCC: spawner/turret base tick (A0 = object).
 * When +0x06 != 2, checks HP <= 0 (0x0D0E04) or bomb-clear (0x0D0E30),
 * playing sound 0x1A, installing behaviour 0x0FA170 (0x0D0678), and entering
 * state 2. Scrolls +0x14/+0x18, retires via 0x0CEE74 on screen-clear or
 * X < -0x28, updates +0x0C, periodically spawns 0x0FA1C4 (0x01057C), steps
 * script (0x0D0690), blinks (0x0CBE34), draws (0x001282), and adds collision
 * (0x0D0DC2) when +0x06 != 2. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

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

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F9FCC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) != 2u) {                    /* 0x0F9FCC */
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {       /* 0x0F9FD6 */
            d0 = 0x000000C8u;                           /* 0x0FA0C8 */
            Obj_0D0E04(m, obj, d0);                     /* 0x0FA0CE */
            goto enter_state2;
        }
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {  /* 0x0F9FDE */
            int16_t x = (int16_t)rd16(m, obj + 0x14u);  /* 0x0F9FE6 */
            if (x >= -0x000A && x <= 0x011C) {          /* 0x0F9FEE */
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu)); /* 0x0F9FF6 */
                d0 = 0x000000C8u;                       /* 0x0FA0FC */
                Obj_RelayTimer(m, d0);                  /* 0x0FA102 */
enter_state2:
                Sound_PlayHooked(m, 0x001Au);           /* 0x0FA0D4..0x0FA0E4 */
                a1 = 0x000FA170u;                       /* 0x0FA0E8 */
                Obj_StartBehaviour(m, obj, a1);         /* 0x0FA0EC */
                a1 += 8u;
                wr8(m, obj + 0x06u, 2u);                /* 0x0FA0F2 */
            }
        }
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x0FA002 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0FA10A */
        Obj_StartDying(m, obj);                         /* 0x0FA10E */
        goto done;
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0FA00E */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0FA012 */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0FA016 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0FA01A */

    if ((int16_t)rd16(m, obj + 0x14u) < -0x28) {        /* 0x0FA01E */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0FA10A */
        Obj_StartDying(m, obj);                         /* 0x0FA10E */
        goto done;
    }

    d0 = (d0 & 0xFFFFFF00u) | (rd8(m, obj + 0x20u) != 0u ? 0x07u : 0x49u); /* 0x0FA028..0x0FA032 */
    wr16(m, obj + 0x0Cu, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x0FA034..0x0FA03A */

    {
        uint8_t cd = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x0FA03E */
        wr8(m, obj + 0x23u, cd);
        if (cd == 0u) {
            d0 = (d0 & 0xFFFFFF00u) | (rd8(m, obj + 0x06u) == 2u ? 0x0Fu : 0x28u); /* 0x0FA046..0x0FA052 */
            wr8(m, obj + 0x23u, (uint8_t)d0);           /* 0x0FA054 */
            uint32_t sp = Obj_Spawn(m, obj, 0x000FA1C4u, 3u, 2u, 0u); /* 0x0FA06A */
            if ((int32_t)sp >= 0) {                     /* 0x0FA074 */
                a1 = sp;                                /* 0x0FA07A */
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u)); /* 0x0FA07C */
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x10u)); /* 0x0FA082 */
                d0 = (d0 & 0xFFFFFF00u) | (rd8(m, obj + 0x20u) != 0u ? 0x49u : 0x07u); /* 0x0FA088..0x0FA092 */
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u)); /* 0x0FA094 */
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + (uint16_t)d0)); /* 0x0FA09A */
                wr8(m, a1 + 0x10u, rd8(m, obj + 0x20u)); /* 0x0FA09E */
            }
        }
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0FA0A4 */
    Obj_BlinkColour(m, obj);                            /* 0x0FA0AA */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0FA0B0 */
    if (rd8(m, obj + 0x06u) != 2u)                      /* 0x0FA0B6 */
        add_coll_regs(m, obj, &a1);                     /* 0x0FA0C0 */

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
