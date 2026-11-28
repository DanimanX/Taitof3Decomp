/* ROM 0x130B16: (recipe capture fnk_, work/zcap/recipes15.tsv): 3D (z-depth) scaled object
 * tick. Integrates the (+0x56,+0x5A,+0x5E) position by the (+0x44,+0x46,+0x48)
 * velocities << 8, and when the depth goes negative frees the object (0x0105EC,
 * like the A5-0x144B bit 4 "bomb clear" flag which skips straight there).
 *
 * The per-frame projection 0x130BE4 projects the depth to screen:
 *   D2 = (z.w >> 1) + 0x64; x = z.x >> 8 / D2 + 0xA0; y = z.y >> 8 / D2 + 0x74;
 *   zoom = -(0x639C / D2 - 0xFF), stored at +0x1F/+0x1E.
 * Depth D2 outside the 0x12EDA4 bounds {0, 0x68} skips the list push. Otherwise
 * the debug on-screen kill (A5+0x1D58 bit 1, x in [-10, 0x11C]) explodes the
 * object through 0x0CBB2C and returns from this routine; else it is appended
 * to the list at A5+0x906. Finally the sprite def at +0x10 scales the x/y
 * offsets by (zoom - 0x100) >> 5 and 0x001282 draws it.
 * The shipped capture has one pair whose post state was sampled 678 frames
 * after the entry (the runner skips it), so this is checked against the bytes.
 *
 * 0x130C26 cmp2.w (0xE17A,PC),D2 compares D2 with the word pair at 0x12EDA4
 * {0x0000, 0x0068} = lower 0, upper 0x68 (bcs = out of range). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

/* divs.w Dm,Dn: signed 32/16, quotient low word, remainder high word; the
 * destination is left alone on overflow or a zero divisor. */
static uint32_t divs_w(uint32_t dreg, int16_t divisor)
{
    if (divisor == 0)
        return dreg;
    int32_t q = (int32_t)dreg / divisor;
    int32_t r = (int32_t)dreg % divisor;
    if (q < -32768 || q > 32767)
        return dreg;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

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

/* ROM 0x130BE4: project the depth position, scale and draw. Returns 0 when
 * the caller (0x130B24) continues, 1 when the 0x0CBB2C explosion took over and
 * the routine returns straight to its own caller. */
static int sub_130BE4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];

    d0 = (uint32_t)((int32_t)rd32(m, obj + 0x56u) >> 8);   /* 0x130BE4..0x130BE8 */
    d1 = (uint32_t)((int32_t)rd32(m, obj + 0x5Au) >> 8);   /* 0x130BEA..0x130BEE */
    d2 = setw(d2, (uint16_t)(rd16(m, obj + 0x5Eu) >> 1));  /* 0x130BF0..0x130BF4 */
    d2 = setw(d2, (uint16_t)(d2 + 0x64u));                 /* 0x130BF6 addi.w */
    d0 = divs_w(d0, (int16_t)(d2 & 0xFFFFu));              /* 0x130BFA */
    d1 = divs_w(d1, (int16_t)(d2 & 0xFFFFu));              /* 0x130BFC */
    d3 = setw(d3, (uint16_t)d0);                           /* 0x130BFE move.w D0w,D3w */
    d0 = setw(d0, (uint16_t)(d0 + 0xA0u));                 /* 0x130C00 */
    d1 = setw(d1, (uint16_t)(d1 + 0x74u));                 /* 0x130C04 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                    /* 0x130C08 */
    wr16(m, obj + 0x18u, (uint16_t)d1);                    /* 0x130C0C */

    d0 = divs_w(0x0000639Cu, (int16_t)(d2 & 0xFFFFu));     /* 0x130C10..0x130C16 */
    d0 = setw(d0, (uint16_t)(d0 - 0xFFu));                 /* 0x130C18 subi.w #0xff */
    d0 = setw(d0, (uint16_t)(-(int16_t)(d0 & 0xFFFFu)));   /* 0x130C1C neg.w */
    wr8(m, obj + 0x1Fu, (uint8_t)d0);                      /* 0x130C1E */
    wr8(m, obj + 0x1Eu, (uint8_t)d0);                      /* 0x130C22 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;

    int16_t scale = (int16_t)(d2 & 0xFFFFu);
    if (scale >= 0 && scale <= 0x68) {                     /* 0x130C26 cmp2.w, bcs 0x130C54 */
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&     /* 0x130C30 btst.b #1 */
            (int16_t)rd16(m, obj + 0x14u) >= -0x0A &&      /* 0x130C38 */
            (int16_t)rd16(m, obj + 0x14u) <= 0x11C) {      /* 0x130C40 */
            Obj_ExplodeTick(m, obj);                       /* 0x130C48 jmp 0x0CBB2C */
            return 1;
        }
        uint32_t lists = rd32(m, F3_A5 + 0x0906u);         /* 0x130C4E jsr 0x0CBDAE */
        wr32(m, lists, obj);
        wr32(m, F3_A5 + 0x0906u, lists + 4u);
    }

    uint32_t a1 = rd32(m, obj + 0x10u);                    /* 0x130C54 movea.l */
    d0 = setw(d0, (uint16_t)(d0 - 0x100u));                /* 0x130C58 subi.w #0x100 */
    d1 = setw(d1, (uint16_t)d0);                           /* 0x130C5C move.w D0w,D1w */
    d0 = (uint32_t)((int32_t)(int16_t)(d0 & 0xFFFFu)
                    * (int32_t)(int16_t)rd16(m, a1));      /* 0x130C5E muls.w (A1)+ */
    a1 += 2u;
    d0 = setw(d0, (uint16_t)((uint16_t)((int16_t)(d0 & 0xFFFFu) >> 5))); /* 0x130C60 asr.w #5 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x130C62 */
    d1 = (uint32_t)((int32_t)(int16_t)(d1 & 0xFFFFu)
                    * (int32_t)(int16_t)rd16(m, a1));      /* 0x130C66 muls.w (A1)+ */
    d1 = setw(d1, (uint16_t)((uint16_t)((int16_t)(d1 & 0xFFFFu) >> 5))); /* 0x130C68 asr.w #5 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1)); /* 0x130C6A */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                                /* 0x130C6E -> 0x001282 */
    return 0;                                              /* 0x130C74 rts */
}

int Fn_130B16(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)           /* 0x130B16 btst.b #4 */
        goto free_object;                                  /* 0x130B1C bne.w 0x130B52 */

    if (sub_130BE4(m, r) != 0)                             /* 0x130B20 bsr 0x130BE4 */
        return 0;                                          /* the explosion returned */

    r->d[0] = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x44u) << 8); /* 0x130B24..0x130B2A */
    wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) + r->d[0]);  /* 0x130B2C */
    r->d[0] = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x46u) << 8); /* 0x130B30..0x130B36 */
    wr32(m, obj + 0x5Au, rd32(m, obj + 0x5Au) + r->d[0]);  /* 0x130B38 */
    r->d[0] = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x48u) << 8); /* 0x130B3C..0x130B42 */
    wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) + r->d[0]);  /* 0x130B44 */

    if ((int16_t)rd16(m, obj + 0x5Eu) < 0)                 /* 0x130B48 tst.w/blt */
        goto free_object;                                  /* 0x130B4C */
    wr32(m, obj, 0x00130B16u);                             /* 0x130B0E re-arm */
    return 0;

free_object:
    Obj_Free(m, obj);                                      /* 0x130B52..0x130B5A */
    return 0;
}
