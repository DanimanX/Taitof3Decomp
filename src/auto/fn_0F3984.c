/* ROM 0x0F3984: multi-part enemy core/body tick (A0 = object).
 * State 0 counts +0x4E up to 9 every 3 frames (or dies via 0x0CEE74 if
 * +0x22 == 0). Active states handle bomb-clear (spawning 0x0F4060), HP <= 0
 * / screen-clear death (spawning 0x0DC486), Y-scroll, script/motion
 * (0x0CF1E4, 0x0CF4B8), off-screen check (+0x3A < -0x20), script step
 * (0x0D0690), collision (0x0D0DC2), blink (0x0CBE34), and draw (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
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

int Fn_0F3984(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) == 0u) {                    /* 0x0F3984 */
        if (rd8(m, obj + 0x22u) == 0u) {                /* 0x0F3AC8 */
            Obj_StartDying(m, obj);                     /* 0x0F3ACC */
        } else if (rd8(m, obj + 0x4Eu) != 9u) {         /* 0x0F3AD2 */
            uint8_t cd = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, cd);                    /* 0x0F3ADC */
            if (cd == 0u) {
                wr8(m, obj + 0x23u, 3u);                /* 0x0F3AE4 */
                wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) + 1u)); /* 0x0F3AEA */
            }
        }
        goto done;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F398C */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);      /* 0x0F3994 */
        if (x >= -0x000A && x <= 0x011C) {              /* 0x0F399C */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F39A4 */
            d0 = 0x00000BB8u;                           /* 0x0F3A1C */
            Obj_RelayTimer(m, d0);                      /* 0x0F3A22 */
            uint32_t sp = Obj_Spawn(m, obj, 0x000F4060u, 9u, 2u, 0u); /* 0x0F3A3A */
            if ((int32_t)sp >= 0) {                     /* 0x0F3A44 */
                wr32(m, sp + 0x14u, rd32(m, obj + 0x14u)); /* 0x0F3A4C */
                wr32(m, sp + 0x18u, rd32(m, obj + 0x18u)); /* 0x0F3A52 */
            }
            goto retire;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F39B0 */
        d0 = 0x00000BB8u;                               /* 0x0F3A5C */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F3A62 */
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F39B8 */
        goto death_spawn;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F39C2 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F39C6 */
    {
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F39CA */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if ((f7 & 0x02u) != 0u) {                       /* 0x0F39D0 */
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F39D4 */
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x0F39DA */
        a1 = 0x000F3AF0u;                               /* 0x0F39E2 */
        if (ObjScript_CF4B8(m, obj, a1) < 0)            /* 0x0F39E6 */
            return -1;
    }

    if ((int16_t)rd16(m, obj + 0x3Au) < -0x20)          /* 0x0F39EC */
        goto retire;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F39F6 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F39FC */
    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F3A02 */
    add_coll_regs(m, obj, &a1);                         /* 0x0F3A08 */
    Obj_BlinkColour(m, obj);                            /* 0x0F3A0E */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F3A14 */
    goto done;

death_spawn:
    {
        uint32_t sp = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u); /* 0x0F3A7A */
        if ((int32_t)sp >= 0) {                         /* 0x0F3A84 */
            wr32(m, sp + 0x14u, rd32(m, obj + 0x14u));  /* 0x0F3A8C */
            wr16(m, sp + 0x14u, (uint16_t)(rd16(m, sp + 0x14u) + 0x10u)); /* 0x0F3A92 */
            wr32(m, sp + 0x18u, rd32(m, obj + 0x18u));  /* 0x0F3A98 */
            wr16(m, sp + 0x18u, (uint16_t)(rd16(m, sp + 0x18u) + 0x10u)); /* 0x0F3A9E */
            wr16(m, sp + 0x58u, 0x00B4u);               /* 0x0F3AA4 */
            wr16(m, sp + 0x60u, rd16(m, obj + 0x60u));  /* 0x0F3AAA */
        }
    }

retire:
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x0F3AB0 */
        Obj_StartDying(m, obj);                         /* 0x0F3AB6 */
    } else {
        wr8(m, obj + 0x23u, 1u);                        /* 0x0F3ABC */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0F3AC2 */
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
