/* ROM 0x13B498: verified capture: boss-part animation driver; each tick decrements
 * the +0x06 hold counter, and when it expires decrements the +0x22 frame count,
 * spawns the 0x13E4EE puff with sound 0x35, then rebuilds position and sprites
 * through 0x13B55A and re-arms the next state 0x13B4EA. When +0x22 runs below
 * zero it spawns a bigger puff (arg 5) with sound 0x16, plays the 0x138772
 * strip for descriptor 0x13870A, bumps A5+0x32DA and frees the object. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "fn_sound_frames.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle] with speed at +0x42; leaves the
 * two signed products in D1 and D2 and the table base in A3. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * angle);
    uint32_t dx = (uint32_t)((int32_t)speed * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->a[3] = 0x00027E40u;
    r->d[1] = dx;
    r->d[2] = dy;
}

/* ROM 0x001282 wrapper preserving its D0/D1/A1 return values. */
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
        r->d[1] = set_word(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x13B654: draw the sprite of the current +0x10 definition after shifting
 * the position back by the definition's size, then restore +0x14/+0x18. */
static void sub_13B654(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t sp = r->a[7];
    uint32_t saved_x = rd32(m, a0 + 0x14u);          /* 0x13B654 */
    uint32_t saved_y = rd32(m, a0 + 0x18u);          /* 0x13B658 */
    uint32_t a1 = rd32(m, a0 + 0x10u);               /* 0x13B65C */
    uint16_t w = rd16(m, a1);                        /* 0x13B660 */
    r->d[0] = set_word(r->d[0], (uint16_t)(w << 3)); /* 0x13B662 */
    wr16(m, a0 + 0x18u, (uint16_t)(rd16(m, a0 + 0x18u) - (uint16_t)r->d[0])); /* 0x13B664 */
    uint16_t h = rd16(m, a1 + 2u);                   /* 0x13B668 */
    r->d[0] = set_word(r->d[0], (uint16_t)(h << 3)); /* 0x13B66A */
    wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) - (uint16_t)r->d[0])); /* 0x13B66C */
    r->a[1] = a1 + 4u;                               /* (A1)+ twice before the call */
    draw_sprite_regs(m, r);                          /* 0x13B670 jsr 0x001282 */
    wr32(m, a0 + 0x18u, saved_y);                    /* 0x13B676 */
    wr32(m, a0 + 0x14u, saved_x);                    /* 0x13B67A */
    r->a[7] = sp;
}

/* ROM 0x138772: draw the strip of descriptor A1 at a scroll-relative tile
 * offset through 0x1385B2; D0.w gains the horizontal tile offset, D3 gets the
 * layer word and A1 the layer base. */
static void sub_138772(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = r->a[1];
    r->d[4] = set_word(r->d[4], (uint16_t)(rd16(m, a1 + 6u) << 4)); /* 0x138772..0x138776 */
    r->a[2] = rd32(m, 0x001385E2u                       /* 0x138778 lea -0x198(PC) */
                   + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[4]);
    int16_t scroll = (int16_t)((int16_t)rd16(m, r->a[2]) >> 7); /* 0x138780..0x138782 */
    r->d[5] = set_word(r->d[5], (uint16_t)scroll);      /* 0x138784 */
    int32_t q = (int32_t)scroll / 12;                   /* 0x138786..0x138788 divs.w */
    if (scroll < 0)                                     /* 0x13878C..0x138792 */
        --q;
    r->d[4] = (uint32_t)((int16_t)q * 12);              /* 0x138794 muls.w #0xC */
    r->d[3] = set_word(r->d[3], (uint16_t)(r->d[3] + (uint16_t)r->d[4])); /* 0x138798 */
    r->d[3] = set_word(r->d[3], (uint16_t)((uint16_t)r->d[3] & 3u));      /* 0x13879A */
    r->d[3] = set_word(r->d[3], (uint16_t)((uint16_t)r->d[3] << 3));      /* 0x13879E */
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] + (uint16_t)r->d[3])); /* 0x1387A0 */
    r->d[3] = set_word(r->d[3], rd16(m, a1));           /* 0x1387A2 */
    r->a[1] = rd32(m, 0x00138622u                       /* 0x1387A4 lea -0x184(PC) */
                   + 4u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[3]);
    Fn_1385B2(m, r);                                    /* 0x1387AC bsr 0x001385B2 */
}

/* ROM 0x13B55A: refresh the +0x1C flip byte, place the object from the
 * descriptor at +0x0C and the A5 camera, then draw +0x22 animation frames at
 * +0x50 onward and re-arm the +0x42 tile base. */
static void sub_13B55A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    Obj_11C32C(m, a0);                                   /* 0x13B55A jsr 0x11C32C */
    uint32_t a1 = rd32(m, a0 + 0x0Cu);                   /* 0x13B560 */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, a1)));
    wr16(m, a0 + 0x14u, (uint16_t)r->d[0]);              /* 0x13B56A */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, a1 + 2u)));
    wr16(m, a0 + 0x18u, (uint16_t)r->d[0]);              /* 0x13B576 */
    r->d[1] = 0;                                         /* 0x13B57A moveq #0,D1 */
    uint32_t frame = a0 + 0x50u;                         /* 0x13B57C lea 0x50(A0),A1 */
    r->a[2] = 0x0013B680u;                               /* 0x13B580 lea 0xFE(PC),A2 */
    r->a[3] = 0x000CC10Eu;                               /* 0x13B584 lea 0xCC10E */
    wr16(m, a0 + 0x42u, 0x0020u);                        /* 0x13B58A */

    while ((int8_t)r->d[1] < (int8_t)rd8(m, a0 + 0x22u)) {   /* 0x13B590 */
        uint32_t d1_saved = r->d[1], a1_saved = frame;   /* 0x13B598 movem */
        uint32_t a2_saved = r->a[2], a3_saved = r->a[3];

        r->d[0] = set_word(r->d[0], rd8(m, frame));      /* 0x13B59C..0x13B59E */
        r->d[1] = set_word(d1_saved, (uint16_t)((uint16_t)r->d[0] + 4u)); /* 0x13B5A0 */
        r->d[2] = (r->d[1] >> 6) & 3u;                   /* 0x13B5A4 bfextu {24:2} */
        wr8(m, a0 + 0x1Cu, (uint8_t)(rd8(m, 0x000CC10Eu + r->d[2]) ^ 2u)); /* 0x13B5A8 */
        r->d[2] = (r->d[1] >> 3) & 0x0Fu;                /* 0x13B5B4 bfextu {25:4} */
        wr32(m, a0 + 0x10u, rd32(m, 0x0013B680u + 4u * r->d[2])); /* 0x13B5B8 */

        push_long(m, r, r->d[0]);                        /* 0x13B5BE movem {D0} */
        move_by_angle_regs(m, r, (uint16_t)r->d[0]);     /* 0x13B5C2 */
        sub_13B654(m, r);                                /* 0x13B5C8 */
        r->d[0] = rd32(m, r->a[7]);                      /* 0x13B5CC */
        r->a[7] += 4u;
        move_by_angle_regs(m, r, (uint16_t)r->d[0]);     /* 0x13B5D0 */
        r->d[1] = d1_saved;                              /* 0x13B5D6 movem */
        frame = a1_saved;
        r->a[2] = a2_saved;
        r->a[3] = a3_saved;
        r->d[1] = set_word(r->d[1], (uint16_t)((uint16_t)r->d[1] + 1u)); /* 0x13B5DA */
        frame += 1u;                                     /* 0x13B5DC */
    }
    r->a[1] = frame;                                     /* A1 = +0x50 + count */

    if ((int32_t)rd32(m, a0 + 0x24u) <= 0)               /* 0x13B5E0 */
        return;                                          /* 0x13B652 rts */

    if ((int8_t)rd8(m, a0 + 0x22u) > 1) {                /* 0x13B5E8 */
        r->d[1] = set_word(0u, rd8(m, frame));           /* 0x13B60C..0x13B60E */
        r->d[1] = set_word(r->d[1], (uint16_t)((uint16_t)r->d[1] + 0x44u)); /* 0x13B610 */
        /* 0x13B614 bfexts {24:1}, then andi.b #3 / eori.b #1 on the byte. */
        wr8(m, a0 + 0x1Cu, ((r->d[1] >> 7) & 1u) ? 2u : 1u);
        r->d[2] = (r->d[1] >> 3) & 0x0Fu;                /* 0x13B624 bfextu {25:4} */
        wr32(m, a0 + 0x10u, rd32(m, 0x0013B6C0u + 4u * r->d[2])); /* 0x13B628 */
    } else {
        r->d[0] = (uint32_t)(((rd8(m, a0 + 0x22u) & 1u) << 2)
                             | (rd8(m, a0 + 0x23u) >> 6)); /* 0x13B5F2 {7:3} */
        uint32_t a2 = rd32(m, a0 + 0x0Cu);               /* 0x13B5F8 */
        wr8(m, a0 + 0x1Cu, rd8(m, a2 + 9u));             /* 0x13B5FC */
        wr32(m, a0 + 0x10u, rd32(m, a2 + 0x0Eu + 4u * r->d[0])); /* 0x13B602 */
    }

    r->d[0] = rd8(m, a0 + 0x23u) >> 2;                   /* 0x13B632 bfextu {8:6} */
    uint32_t a2 = rd32(m, a0 + 0x0Cu);                   /* 0x13B638 */
    r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] + rd16(m, a2 + 4u)));
    wr16(m, a0 + 0x42u, (uint16_t)r->d[0]);              /* 0x13B640 */
    r->d[0] = set_word(0u, rd8(m, frame));               /* 0x13B644..0x13B646 */
    move_by_angle_regs(m, r, (uint16_t)r->d[0]);         /* 0x13B648 */
    sub_13B654(m, r);                                    /* 0x13B64E -> 0x13B652 rts */
}

/* ROM 0x13B4F2: +0x22 expired - spawn the bigger puff, draw the death strip
 * and return the object to its pool (bumping A5+0x32DA). */
static void death_13B4F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) < 2) {         /* 0x13B4F2..0x13B4F8 */
        call_spawn(m, r, 0x0013B516u, 0x0013E4EEu, 5u, 2u, 5u); /* 0x13B4FC */
        call_hooked_sound(m, r, 0x0016u, 0x0013B524u, 0x0013B52Au); /* 0x13B51A */
        uint32_t a1 = rd32(m, a0 + 0x0Cu);               /* 0x13B52E */
        a1 = rd32(m, a1 + 0x0Au);                        /* 0x13B532 */
        r->d[0] = set_word(r->d[0], rd16(m, a1));        /* 0x13B536 */
        r->d[1] = set_word(r->d[1], rd16(m, a1 + 2u));   /* 0x13B538 */
        r->d[2] = rd32(m, a1 + 4u);                      /* 0x13B53A */
        r->d[3] = set_word(r->d[3], 0u);                 /* 0x13B53C */
        r->a[1] = 0x0013870Au;                           /* 0x13B540 lea -0x2E38(PC) */
        sub_138772(m, r);                                /* 0x13B544 */
    }
    wr8(m, F3_A5 + 0x32DAu, (uint8_t)(rd8(m, F3_A5 + 0x32DAu) + 1u)); /* 0x13B548 */
    call_free(m, r, 0x0013B554u);                        /* 0x13B54C..0x13B554 */
}

int Fn_13B498(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint16_t counter = (uint16_t)(rd16(m, a0 + 6u) - 1u); /* 0x13B498 */
    wr16(m, a0 + 6u, counter);
    if ((int16_t)counter > 0) {                          /* 0x13B49C -> 0x13B48C */
        sub_13B55A(m, r);
        wr32(m, a0, 0x0013B498u);                        /* 0x13B490 */
        return 0;                                        /* 0x13B496 */
    }

    int8_t frames = (int8_t)(rd8(m, a0 + 0x22u) - 1u);    /* 0x13B49E */
    wr8(m, a0 + 0x22u, (uint8_t)frames);
    if (frames < 0) {                                    /* 0x13B4A2 */
        death_13B4F2(m, r);
        return 0;                                        /* 0x13B558 */
    }

    call_spawn(m, r, 0x0013B4C0u, 0x0013E4EEu, 5u, 2u, 1u); /* 0x13B4A6..0x13B4BA */
    call_hooked_sound(m, r, 0x0035u, 0x0013B4CEu, 0x0013B4D4u); /* 0x13B4C4..0x13B4D4 */
    wr16(m, a0 + 6u, 0x0004u);                           /* 0x13B4D8 */
    sub_13B55A(m, r);                                    /* 0x13B4DE */
    wr32(m, a0, 0x0013B4EAu);                            /* 0x13B4E2 */
    return 0;                                            /* 0x13B4E8 */
}
