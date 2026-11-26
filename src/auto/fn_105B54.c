/* ROM 0x105B54: boss/enemy articulated wing/pod attachment tick (A0 = object,
 * +0x08 = parent). When A5-0x144B bit 5 is set, spawns 0x0DC340 at (+0x20, +0x0B)
 * and starts dying (0x0CEE74). When +0x06 == 0 and grandparent +0x06 == 0,
 * advances +0x06 to 1. Handles bomb-clear (A5+0x1D58 bit 1 -> 0x0CBB2C).
 * Dispatches +0x06 via table 0x105C14: 0 copies grandparent pos/colour and
 * draws grandparent, parent, and self; 1 arms +0x07 bit 4, sets state 2,
 * starts script 0x105D16 and behaviour 0x105DA0, draws self; 2 runs
 * 0x0CF1E4/0x0CF4B8 (aim 0x105CDE), 0x0CF85A offscreen check, and draws self.
 * Tail checks +0x07 bit 3 (0x0CEE74) and runs 0x0D0690 + 0x0D0DC2. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x001282 register outputs after Obj_DrawSprite(m, obj) */
static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, SPR_CURSOR);
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

/* ROM 0x00649A with full 32-bit D0 output (high word from swap D0) */
static uint32_t dist_full(const f3mem_t *m, int16_t dx, int16_t dy)
{
    uint32_t d0 = (uint32_t)((int32_t)dx * (int32_t)dx)
                + (uint32_t)((int32_t)dy * (int32_t)dy);
    d0 += d0;
    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }
    uint16_t v = rd16(m, 0x00028340u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 2u);
    uint32_t r = (uint32_t)v << (shift & 63u);
    return (r << 16) | (r >> 16);
}

/* ROM 0x0D0690 with exact D0/A1 register outputs when script advances */
static void step_script_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0w < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0w);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0w);
    }
    d0w = rd16(m, a1);
    wr8(m, obj + 0x1Cu, (uint8_t)d0w); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *d0_io = setw(*d0_io, d0w);
    *a1_io = a1;
}

int Fn_105B54(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x105B54..0x105B5A */
        d0 = Obj_Spawn(m, obj, 0x000DC340u, 3u, 2u, 0u); /* 0x105BC8..0x105BE0 */
        if ((int32_t)d0 >= 0) {                          /* 0x105BE4..0x105BE6 */
            a1 = d0;                                     /* 0x105BEC */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));   /* 0x105BEE */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0020u)); /* 0x105BF4 */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));   /* 0x105BFA */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x000Bu)); /* 0x105C00 */
            wr16(m, a1 + 0x58u, 0u);                     /* 0x105C06 */
            wr16(m, a1 + 0x60u, 0u);                     /* 0x105C0A */
        }
        Obj_StartDying(m, obj);                          /* 0x105C0E -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    if (rd8(m, obj + 0x06u) == 0u) {                     /* 0x105B5E..0x105B62 */
        a1 = rd32(m, obj + 0x08u);                       /* 0x105B66 */
        a1 = rd32(m, a1 + 0x08u);                        /* 0x105B6A */
        if (rd8(m, a1 + 0x06u) != 0u)                    /* 0x105B6E..0x105B72 */
            goto dispatch;
        wr8(m, obj + 0x06u, 1u);                         /* 0x105B76 */
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x105B7C */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                 /* 0x105B84..0x105B92 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));    /* 0x105B94 */
            Obj_ExplodeTick(m, obj);                     /* 0x105B9A -> 0x0CBB2C */
            if (rd32(m, obj + 0x00u) == 0x000CBC78u) {
                draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
            } else {
                int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
                int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - 0x0074u);
                uint32_t df = dist_full(m, dx, dy);
                uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
                d0 = setw(df, (uint16_t)(((uint16_t)df << 2) - thresh));
                d1 = setw(d1, (uint16_t)dy);
                a1 = rd32(m, obj + 0x10u) + 4u;
            }
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

dispatch:
    {
        uint16_t st = rd8(m, obj + 0x06u);               /* 0x105BA0..0x105BA2 */
        d0 = setw(d0, st);
        switch (st) {                                    /* 0x105BA6 table 0x105C14 */
        case 0u:
            a1 = rd32(m, obj + 0x08u);                   /* 0x105C20 */
            a1 = rd32(m, a1 + 0x08u);                    /* 0x105C24 */
            wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));     /* 0x105C28 */
            wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));   /* 0x105C2E */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u)); /* 0x105C34 */
            wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));   /* 0x105C3A */
            d0 = setw(d0, (uint16_t)(rd16(m, a1 + 0x42u) + 0x000Eu)); /* 0x105C40..0x105C44 */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x105C48 */
            Obj_DrawSprite(m, a1);                       /* 0x105C50 */
            Obj_DrawSprite(m, rd32(m, obj + 0x08u));     /* 0x105C5A */
            Obj_DrawSprite(m, obj);                      /* 0x105C62 */
            draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
            break;

        case 1u:
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x105C6A */
            wr8(m, obj + 0x1Du, 0u);                     /* 0x105C70 */
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));  /* 0x105C74 */
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));  /* 0x105C7A */
            wr8(m, obj + 0x06u, 2u);                     /* 0x105C80 */
            Obj_StartScript(m, obj, 0x00105D16u);        /* 0x105C86..0x105C8A */
            Obj_StartBehaviour(m, obj, 0x00105DA0u);     /* 0x105C90..0x105C94 */
            Obj_DrawSprite(m, obj);                      /* 0x105C9A */
            draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
            break;

        case 2u: {
            uint8_t f7 = rd8(m, obj + 0x07u);            /* 0x105CA2 */
            if ((f7 & 0x02u) != 0u) {
                wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
                if (ObjScript_CF1E4(m, obj) < 0)         /* 0x105CAC */
                    return -1;
            }
            if (rd8(m, obj + 0x20u) != 0u) {             /* 0x105CB2..0x105CB6 */
                if (ObjScript_CF4B8(m, obj, 0x00105CDEu) < 0) /* 0x105CBA..0x105CBE */
                    return -1;
            }
            Obj_FlagOffscreen(m, obj);                   /* 0x105CC4 */
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));  /* 0x105CCA */
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));  /* 0x105CD0 */
            Obj_DrawSprite(m, obj);                      /* 0x105CD6 */
            draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
            break;
        }

        default:
            return -1;
        }
    }

    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {           /* 0x105BAE..0x105BB4 */
        Obj_StartDying(m, obj);                          /* 0x0CEE74 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    step_script_regs(m, obj, &d0, &a1);                  /* 0x105BBA jsr 0x0D0690 */
    Obj_0D0DC2(m, obj);                                  /* 0x105BC0 */
    a1 = rd32(m, F3_A5 + 0x0906u);
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
