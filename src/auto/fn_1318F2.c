/* ROM 0x1318F2: homing missile / sight tick (A0 = object).
 * A bomb-clear flag (A5-0x144B bit 4) or HP <= 0 (crediting +0x4C to the
 * 0x0CBDDE receiver) hands the object to 0x0CD3D6 with D0 = 2 (its 0x0CD444
 * animation slot) and heading 0x80. Otherwise 0x0CBD52 walks the animation at
 * 0x1319B0; the bomb bit plus -0x0A <= X <= 0x11C goes straight to the
 * explosion tick (0x0CBD12 relay + 0x0CBB2C) after colouring +0x1D from +0x4F.
 * The normal frame moves along +0x20 with speed +0x42 (decaying to 2) via
 * 0x0CBE12, subtracts 0x40*+0x22 from X while mode bit 24 (A5+0x30EE) is set,
 * adds itself to both collision lists (0x0CBDA4), updates the flash colour
 * (0x11C32C) and draws (0x001282); X <= -0x10 frees it, else 0x1318F2 re-arms. */
#include "fn_auto.h"
#include "../objects/obj_common.h"      /* Obj_Animate (0x0CBD52), Obj_Free */
#include "../objects/obj_shared.h"       /* Obj_ExplodeTick (0x0CBB2C) */
#include "../objects/obj_pool.h"        /* Obj_Free */
#include "../enemy/obj_E8B72.h"         /* Sound_PlayHooked */
#include "../objects/obj_shared_ext.h"   /* Obj_0CBDDE, Obj_0D0DC2, Obj_11C32C */
#include "../video/sprite.h"

#define ANIM_1319B0 0x001319B0u
#define CVT_TBL     0x000CD444u          /* 0x0CD3D6's per-shot animation list */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

/* ROM 0x00649A with the full 32-bit D0 result (D1 is preserved there). */
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

/* ROM 0x0CBB2C with register tracking (see fn_0D63A8.c). */
static void explode_tick_regs(const f3mem_t *m, uint32_t obj,
                              uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d1_in = *d1_io;
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
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int atan_exg = (((int16_t)dx < 0) != ((int16_t)dy < 0));
    uint32_t ang_hi = atan_exg ? (d1_in & 0xFFFF0000u) : (d0_dist & 0xFFFF0000u);

    Obj_ExplodeTick(m, obj);

    if (is_lt) {
        *d0_io = setw(d0_dist, diff);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        *d0_io = (ang_hi & 0xFFFF0000u) | (uint16_t)(ang << 8);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    *d1_io = rd32(m, obj + 0x14u);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
}

/* ROM 0x0CD3D6: switch the object to the 0x0CD412 animation slot selected by
 * D0.w (its first word is a hooked sound), then arm 0x0CD412. */
static void sub_0CD3D6(const f3mem_t *m, uint32_t obj, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, CVT_TBL + (uint32_t)(int32_t)(int16_t)d0w * 4u); /* 0x0CD3D6 */
    uint16_t snd = rd16(m, a1);                         /* 0x0CD3DE */
    a1 += 2u;
    if (snd != 0u)                                      /* 0x0CD3E0 beq */
        Sound_PlayHooked(m, snd);                       /* 0x0CD3E4..0x0CD3F2 */
    wr32(m, obj + 0x0Cu, a1);                           /* 0x0CD3F6 */
    wr16(m, obj + 0x3Au, 0u);                           /* 0x0CD3FA */
    wr16(m, obj + 0x3Eu, 0u);                           /* 0x0CD3FE */
    wr16(m, obj + 0x1Eu, 0u);                           /* 0x0CD402 */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x0CD406 */
    wr32(m, obj + 0x00u, 0x000CD412u);                  /* 0x0CD40A */
    *d0_io = setw(*d0_io, snd);                         /* 0x0CD3DE move.w (A1)+,D0w */
    *a1_io = a1;
}

int Fn_1318F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x1318F2 btst #4 */
        d0 = setw(d0, 0x0002u);                         /* 0x131992 move.w #2 */
        goto kill;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x1318FC tst.l/ble */
        a1 = Obj_0CBDDE(m, obj);                        /* 0x131982 */
        d0 = setw(d0, 0x0000u);                         /* 0x131988 moveq #0 */
        d0 = setw(d0, rd16(m, obj + 0x4Cu));            /* 0x13198A move.w */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);  /* 0x13198E */
        d0 = setw(d0, 0x0002u);                         /* 0x131992 */
        goto kill;
    }

    Obj_Animate(m, obj, ANIM_1319B0);                   /* 0x131904..0x13190A */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x131910 btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {              /* 0x131918..0x131926 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x131928 */
            a1 = rd32(m, F3_A5 + 0x1D50u);              /* 0x0CBD12 */
            d0 = setw(d0, 0x0000u);                     /* 0x0CBD16 moveq #0 */
            d0 = setw(d0, rd16(m, obj + 0x4Cu));
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            explode_tick_regs(m, obj, &d0, &d1, &a1);   /* 0x13192E jmp 0x0CBD12 */
            goto out;
        }
    }

    d0 = setw(d0, rd16(m, obj + 0x20u));                /* 0x131934 */
    {
        /* 0x0CBE12: move along +0x20 with +0x42 as the speed. */
        uint16_t speed = rd16(m, obj + 0x42u);
        uint16_t ang = (uint16_t)d0;
        int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
        int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
        uint32_t p1 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)c);
        uint32_t p2 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)s);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + p1); /* 0x0CBE24 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + p2); /* 0x0CBE2E */
        d1 = p1;                                        /* D1 = cos*speed */
    }
    if ((int16_t)rd16(m, obj + 0x42u) > 2)              /* 0x13193E cmpi/ble */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u)); /* 0x131948 */
    /* 0x13194C bftst (A5+0x30EE){7:1} = bit 24. */
    if ((rd32(m, F3_A5 + 0x30EEu) & 0x01000000u) != 0u) {
        d0 = setw(d0, 0x0000u);                         /* 0x131956 moveq #0 */
        d0 = setw(d0, rd16(m, obj + 0x22u));            /* 0x131958 */
        d0 <<= 6;                                       /* 0x13195C asl.l #6 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0); /* 0x13195E */
    }

    Obj_0D0DC2(m, obj);                                 /* 0x131962 -> 0x0CBDA4 */
    a1 = rd32(m, F3_A5 + 0x0906u);                       /* A1 left by 0x0CBDA4 */
    Obj_11C32C(m, obj);                                 /* 0x131968 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x13196E -> 0x001282 */

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0010) {     /* 0x131974 cmpi/ble */
        Obj_Free(m, obj);                               /* 0x1319A2 */
        goto out;
    }
    wr32(m, obj + 0x00u, 0x001318F2u);                  /* 0x13197E -> 0x1318EA */
    goto out;

kill:
    wr16(m, obj + 0x20u, 0x0080u);                      /* 0x131996 */
    sub_0CD3D6(m, obj, (uint16_t)d0, &d0, &a1);         /* 0x13199C jmp 0x0CD3D6 */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
