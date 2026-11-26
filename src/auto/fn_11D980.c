/* ROM 0x11D980: dying wing/part drift & explosion emitter tick (A0 = object).
 * Every 4 frames (A5+0x878 & 3 == 0), plays hooked sound 0x32, spawns explosion
 * 0x13E4EE (via 0x13E4D0) and debris 0x13EE08 at a random offset from Rng_NextMem.
 * Always drifts (-6, -2) in (+0x14, +0x18), draws sprite (0x001282), and re-arms
 * 0x11D980 while X > 0x0CC112 (-32), else frees via 0x0105EC. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
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

int Fn_11D980(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    d0 = setw(d0, rd16(m, F3_A5 + 0x0878u) & 0x0003u);   /* 0x11D980..0x11D984 */
    if ((uint16_t)d0 == 0u) {                            /* 0x11D988 */
        Sound_PlayHooked(m, 0x0032u);                    /* 0x11D98C..0x11D99C */
        d0 = Obj_13E4D0(m, obj, 2u);                     /* 0x11D9A0..0x11D9A2 */
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0); /* 0x11D9A8..0x11D9BE */

        d1 = 0u;                                         /* 0x11D9C2 */
        d0 = Rng_NextMem(m);                             /* 0x11D9C4 */
        d0 = setw(d0, (uint16_t)((d0 & 0x001Fu) + 0x0028u)); /* 0x11D9CA..0x11D9CE */
        d1 = (d1 & 0x00FFFFFFu) | ((d0 & 0xFFu) << 24);  /* 0x11D9D2 bfins D0,D1{0:8} */

        d0 = Rng_NextMem(m);                             /* 0x11D9D6 */
        d0 = setw(d0, (uint16_t)((d0 & 0x001Fu) + 0x000Cu)); /* 0x11D9DC..0x11D9E0 */
        d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0xFFu) << 16);  /* 0x11D9E4 bfins D0,D1{8:8} */

        d0 = Obj_Spawn(m, obj, 0x0013EE08u, 5u, 2u, d1); /* 0x11D9E8..0x11D9FE */
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 6u)); /* 0x11DA02 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u)); /* 0x11DA06 */
    r->d[0] = d0;
    r->d[1] = d1;
    draw_sprite_regs(m, r);                              /* 0x11DA0A */

    uint16_t x = rd16(m, obj + 0x14u);                   /* 0x11DA10 */
    r->d[0] = setw(r->d[0], x);
    if ((int16_t)x > (int16_t)rd16(m, 0x000CC112u))     /* 0x11DA14..0x11DA1A */
        wr32(m, obj, 0x0011D980u);                       /* 0x11D978 */
    else
        Obj_Free(m, obj);                                /* 0x11DA1E..0x11DA2A */

    return 0;
}
