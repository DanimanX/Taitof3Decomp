/* ROM 0x105898: multi-state enemy/wingman sub-object tick (A0 = object, A0+0x08 = parent).
 * On A5-0x144B bit 5, spawns 0x0DC340 explosion at (+0x20, +0x14) and calls
 * Obj_StartDying (0x0CEE74). When state (+0x06) == 0 and parent+0x06 == 0,
 * advances state to 1. Checks on-screen bomb/explode (0x0CBB2C). Dispatches
 * state via jump table 0x105954:
 *   0: tracks parent + (-0x10, -(+0x42 + 0x0E)), inherits parent+0x1D.
 *   1: detaches (flags7|=0x10, state=2, installs script 0x105A36 & behaviour
 *      0x105AC0, draws sprite).
 *   2: steps 0xCF1E4 if flags7 bit 1, steps 0xCF4B8 with 0x1059FE if +0x20!=0,
 *      flags offscreen (0x0CF85A), draws sprite at (+0x3A, +0x3E).
 * If flags7 bit 3 set, calls Obj_StartDying; else steps behaviour script
 * (0x0D0690) and appends to collision lists (0x0D0DC2). */
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

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
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
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x00649A with full D0 result. */
static uint32_t dist_full(const f3mem_t *m, uint16_t dx_in, uint16_t dy_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)dx_in * (int32_t)(int16_t)dx_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)dy_in * (int32_t)(int16_t)dy_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    return (d0 << 16) | (d0 >> 16);
}

/* ROM 0x0CBB2C with register tracking. */
static void explode_tick_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t off_y = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1)) >> 5;
    int16_t off_x = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1 + 2u)) >> 5;
    dx = (uint16_t)(dx - (uint16_t)off_x);
    dy = (uint16_t)(dy - (uint16_t)off_y);

    uint32_t d0_dist = dist_full(m, dx, dy);
    uint16_t d0w = (uint16_t)((uint16_t)d0_dist << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0w < (int16_t)thresh;
    uint16_t diff = (uint16_t)(d0w - thresh);

    Obj_ExplodeTick(m, obj);

    if (is_lt) {
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, diff);
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        uint16_t ang = Obj_Atan2(m, dx, dy);
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, (uint16_t)(ang << 8));
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    draw_sprite_regs(m, r);
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

int Fn_105898(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x105898 */
        uint32_t child = Obj_Spawn(m, obj, 0x000DC340u, 3u, 2u, 0u); /* 0x105908 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {                      /* 0x105926 */
            r->a[1] = child;
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0020u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0014u));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
        Obj_StartDying(m, obj);                         /* 0x0CEE74 */
        return 0;
    }

    if (rd8(m, obj + 0x06u) == 0u) {                    /* 0x1058A2 */
        uint32_t parent = rd32(m, obj + 0x08u);
        r->a[1] = parent;
        if (rd8(m, parent + 0x06u) != 0u)               /* 0x1058AE */
            goto dispatch;
        wr8(m, obj + 0x06u, 1u);                        /* 0x1058B6 */
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x1058BC */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x1058D4 */
            explode_tick_regs(m, r);                    /* 0x1058DA -> 0x0CBB2C */
            return 0;
        }
    }

dispatch: {
        uint8_t state = rd8(m, obj + 0x06u);            /* 0x1058E0 */
        r->d[0] = setw(r->d[0], state);
        switch (state) {
        case 0u: {                                      /* 0x105960 */
            uint32_t parent = rd32(m, obj + 0x08u);
            r->a[1] = parent;
            wr8(m, obj + 0x1Du, rd8(m, parent + 0x1Du));
            wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
            wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
            uint16_t d0w = (uint16_t)(rd16(m, parent + 0x42u) + 0x000Eu);
            r->d[0] = setw(r->d[0], d0w);
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
            break;
        }
        case 1u: {                                      /* 0x10598A */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x1Du, 0u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
            wr8(m, obj + 0x06u, 2u);
            Obj_StartScript(m, obj, 0x00105A36u);
            Obj_StartBehaviour(m, obj, 0x00105AC0u);
            draw_sprite_regs(m, r);
            break;
        }
        case 2u: {                                      /* 0x1059C2 */
            uint8_t f7 = rd8(m, obj + 0x07u);
            if ((f7 & 0x02u) != 0u) {
                wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
                if (ObjScript_CF1E4(m, obj) < 0)
                    return -1;
            }
            if (rd8(m, obj + 0x20u) != 0u) {
                if (ObjScript_CF4B8(m, obj, 0x001059FEu) < 0)
                    return -1;
            }
            Obj_FlagOffscreen(m, obj);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            draw_sprite_regs(m, r);
            break;
        }
        default:
            return -1;
        }
    }

    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x1058EE */
        Obj_StartDying(m, obj);                         /* 0x0CEE74 */
        return 0;
    }

    step_script_regs(m, obj, &r->d[0], &r->a[1]);       /* 0x1058FA (0x0D0690) */
    Obj_0D0DC2(m, obj);                                 /* 0x105900 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    return 0;
}
