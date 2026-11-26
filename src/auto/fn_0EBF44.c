/* ROM 0x0EBF44: destructible turret/part tick (A0 = object).
 * State 3 checks parent (+0x08)->+0x06 == 0 and retires via 0x0CEE74.
 * State 2 scrolls +0x3E, runs 0x0CF1E4/0x0CF4B8, syncs (+0x14, +0x18), steps
 * script (0x0D0690); while +0x4C != 0 draws (0x001282), otherwise spawns 2
 * debris pieces via 0x0CEFF8 (or decrements parent +0x22 by 2 and dies).
 * States 0..1 handle bomb-hit (A5+0x1D58 bit1), HP <= 0 or A5-0x144B bit5
 * -> state 2; otherwise scroll/script, check 0x0CF82C and 0x0CF85A (retiring
 * if offscreen), dispatch 0x0EC108[+0x06], step script, add to lists
 * (0x0D0DE4), blink (0x0CBE34), and draw (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CF82C: if +0x07 bit7 set and +0x3E.w in [-0x40, 0xF7], clear bit7 and set bit1. */
static void sub_0CF82C(const f3mem_t *m, uint32_t obj)
{
    uint8_t f = rd8(m, obj + 0x07u);
    if ((f & 0x80u) == 0u)
        return;
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (y < -0x40 || y > 0x00F7)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((f & ~0x80u) | 0x02u));
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0CF4B8 wrapper tracking D0/D1 left by sub_0D2D44 in modes 12..15. */
static int cf4b8_regs(const f3mem_t *m, uint32_t obj, uint32_t rec,
                      uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t op = rd8(m, obj + 0x20u);
    if (ObjScript_CF4B8(m, obj, rec) < 0)
        return -1;
    if (op >= 12u && op <= 15u)
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), d0_io, d1_io);
    return 0;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0EC00C: start death animation and enter state 2. */
static void enter_state2_0EC00C(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
    uint32_t a1 = 0x000EC2DAu;                          /* 0x0EC00C */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0EC010 */
    r->d[0] = setw(r->d[0], rd16(m, a1 + 2u));
    r->a[1] = a1 + 8u;
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));           /* 0x0EC016 */
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 8u)); /* 0x0EC01C */
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 8u)); /* 0x0EC020 */
    wr8(m, obj + 0x06u, 2u);                            /* 0x0EC024 */
}

int Fn_0EBF44(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t state = rd8(m, obj + 0x06u);

    if (state == 3u) {                                  /* 0x0EBF44 */
        uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x0EC0E4 */
        if (rd8(m, a1 + 0x06u) == 0u)                   /* 0x0EC0E8 */
            Obj_StartDying(m, obj);                     /* 0x0EC0EC */
        r->a[1] = a1;
        return 0;
    }

    if (state == 2u) {                                  /* 0x0EBF4E */
        uint32_t d0 = rd32(m, F3_A5 + 0x4228u);         /* 0x0EC02C */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0); /* 0x0EC030 */
        r->d[0] = d0;
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));    /* 0x0EC034 */
        if ((f7 & 0x02u) != 0u)                         /* 0x0EC03A */
            ObjScript_CF1E4(m, obj);                    /* 0x0EC03E */
        if (rd8(m, obj + 0x20u) != 0u)                  /* 0x0EC044 */
            cf4b8_regs(m, obj, 0x000EC136u, &r->d[0], &r->d[1]); /* 0x0EC04C..0x0EC050 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));     /* 0x0EC056 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));     /* 0x0EC05C */
        Obj_StepScript(m, obj);                         /* 0x0EC062 */
        if (rd16(m, obj + 0x4Cu) != 0u) {               /* 0x0EC068 */
            draw_sprite_regs(m, obj, r);                /* 0x0EC070 */
            return 0;
        }
        r->a[1] = 0x000EC352u;                          /* 0x0EC078 */
        r->d[1] = setw(r->d[1], 2u);                    /* 0x0EC07C */
        Fn_0CEFF8(m, r);                                /* 0x0EC07E */
        if ((int16_t)r->d[0] < 0) {                     /* 0x0EC084..0x0EC086 */
            uint32_t a1 = rd32(m, obj + 0x08u);         /* 0x0EC0D6 */
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 2u)); /* 0x0EC0DA */
            Obj_StartDying(m, obj);                     /* 0x0EC0DE */
            r->a[1] = a1;
            return 0;
        }
        uint32_t a1 = F3_A5 + 0x2862u;                  /* 0x0EC08A */
        uint32_t a2 = rd32(m, a1); a1 += 4u;            /* 0x0EC08E */
        wr32(m, a2 + 0x14u, rd32(m, obj + 0x14u));      /* 0x0EC090 */
        wr16(m, a2 + 0x14u, (uint16_t)(rd16(m, a2 + 0x14u) + 3u)); /* 0x0EC096 */
        wr32(m, a2 + 0x18u, rd32(m, obj + 0x18u));      /* 0x0EC09A */
        wr16(m, a2 + 0x18u, (uint16_t)(rd16(m, a2 + 0x18u) + 5u)); /* 0x0EC0A0 */
        wr32(m, a2 + 0x10u, 0x000EC58Eu);               /* 0x0EC0A4 */
        a2 = rd32(m, a1);                               /* 0x0EC0AC */
        wr32(m, a2 + 0x14u, rd32(m, obj + 0x14u));      /* 0x0EC0AE */
        wr16(m, a2 + 0x14u, (uint16_t)(rd16(m, a2 + 0x14u) + 0x0Eu)); /* 0x0EC0B4 */
        wr32(m, a2 + 0x18u, rd32(m, obj + 0x18u));      /* 0x0EC0BA */
        wr16(m, a2 + 0x18u, (uint16_t)(rd16(m, a2 + 0x18u) + 0x0Bu)); /* 0x0EC0C0 */
        wr32(m, a2 + 0x10u, 0x000EC5C4u);               /* 0x0EC0C6 */
        wr8(m, obj + 0x06u, 3u);                        /* 0x0EC0CE */
        r->a[1] = a1;
        r->a[2] = a2;
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0EBF58 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                /* 0x0EBF60..0x0EBF6E */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0EBF70 */
            enter_state2_0EC00C(m, obj, r);             /* 0x0EBF76 */
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0              /* 0x0EBF7C */
        || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {   /* 0x0EBF84 */
        enter_state2_0EC00C(m, obj, r);
        return 0;
    }

    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);             /* 0x0EBF8E */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0EBF92 */
    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));        /* 0x0EBF96 */
    if ((f7 & 0x02u) != 0u)                             /* 0x0EBF9C */
        ObjScript_CF1E4(m, obj);                        /* 0x0EBFA0 */
    if (rd8(m, obj + 0x20u) != 0u)                      /* 0x0EBFA6 */
        cf4b8_regs(m, obj, 0x000EC136u, &r->d[0], &r->d[1]); /* 0x0EBFAE..0x0EBFB2 */
    sub_0CF82C(m, obj);                                 /* 0x0EBFB8 */
    if ((int16_t)rd16(m, obj + 0x3Au) <= 0x013F) {      /* 0x0EBFBE */
        Obj_FlagOffscreen(m, obj);                      /* 0x0EBFC8 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {      /* 0x0EBFCE */
            uint32_t a1 = rd32(m, obj + 0x08u);         /* 0x0EC0F4 */
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 2u)); /* 0x0EC0F8 */
            wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u)); /* 0x0EC0FC */
            Obj_StartDying(m, obj);                     /* 0x0EC102 */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0EBFD8 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0EBFDE */
    uint8_t st = rd8(m, obj + 0x06u);                   /* 0x0EBFE6 */
    if (st == 0u) {                                     /* 0x0EBFEA -> 0x0EC118 */
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            Obj_StartBehaviour(m, obj, 0x000EC29Eu);    /* 0x0EC122..0x0EC126 */
            wr8(m, obj + 0x06u, 1u);                    /* 0x0EC12C */
        }
    }
    /* st == 1 -> 0x0EC134 (rts); st >= 2 handled above. */

    Obj_StepScript(m, obj);                             /* 0x0EBFF2 */
    Obj_0D0DE4(m, obj);                                 /* 0x0EBFF8 */
    Obj_BlinkColour(m, obj);                            /* 0x0EBFFE */
    draw_sprite_regs(m, obj, r);                        /* 0x0EC004 */
    return 0;
}
