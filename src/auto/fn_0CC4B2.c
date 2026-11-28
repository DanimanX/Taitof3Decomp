/* ROM 0x0CC4B2: (injected capture fnj_, NOIRQ): enemy bullet/missile tick. When the
 * "bullet shield" byte A5+0x1D58 has bit 1 and the object X is within
 * [-0x0A, 0x11C] it is retired immediately; the same happens when its lifetime
 * +0x24 has run out or the global blast flag A5-0x144B bit 4 is set. Otherwise
 * +0x1F loses 0x18, +0x2E gains 3, and 0x0CC554 moves the object along its
 * +0x20 angle, appends it to the A5+0x906 list and draws its sprite. The +0x06
 * countdown re-arms 0x0CC4B2 while positive; at zero the speed +0x42 is
 * recomputed by 0x0CBEB0 from the difficulty level for 0x18 units and the next
 * state 0x0CC50C is installed. Retiring plays the sound of table 0x0CD444[1]
 * (through 0x13F238 + 0x0039D8, empty here), clears +0x1C/+0x1E/+0x3A/+0x3E,
 * points +0x0C past that table word and installs 0x0CD412.
 * The only capture set (1 pair) has its pre and post dumps 0x2A6 frames apart,
 * so the runner skips it as unobservable. Run offline on that pair the body
 * reproduces the captured D0 (0x6C000012), D1 (0x00400C48) and A0 exactly; only
 * A1 (the sprite-RAM write cursor) differs, and the post dump is 678 frames
 * later there. Every path is read from the bytes. */
#include "fn_auto.h"
#include "fn_sound_frames.h"
#include "../video/sprite.h"

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle] with speed at +0x42; D1 and D2
 * get the two signed products and A3 the table base 0x27E40. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * (uint16_t)(int16_t)angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * (uint16_t)(int16_t)angle);
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CBEB0: speed word from the difficulty level A5+0x87E, full D0
 * (MULS.W, ASL.L #4, SWAP); D1 is left holding that scaled level word. */
static uint32_t sub_0CBEB0(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x87Eu) << 3) + 0x0C00u);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | d1;         /* 0x0CBEB2..0x0CBEB6 */
    uint32_t p = (uint32_t)((int32_t)(int16_t)(uint16_t)r->d[0]
                            * (int32_t)(int16_t)d1) << 4; /* 0x0CBEBA muls.w / asl */
    return (p << 16) | (p >> 16);                   /* 0x0CBEBE swap D0 */
}

/* ROM 0x0CC554: move along the +0x20 angle, append A0 to the A5+0x906 list and
 * draw the sprite. */
static void sub_0CC554(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a0 + 0x20u); /* 0x0CC554 */
    move_by_angle_regs(m, r, (uint16_t)r->d[0]);             /* 0x0CC558 */
    uint32_t a1 = rd32(m, F3_A5 + 0x906u);                   /* 0x0CC55E */
    wr32(m, a1, a0);                                         /* 0x0CC562 */
    wr32(m, F3_A5 + 0x906u, a1 + 4u);                        /* 0x0CC564 */
    draw_sprite_regs(m, r);                                  /* 0x0CC568 */
}

/* ROM 0x0CC54C -> 0x0CD3D6: retire the object with the sound of table
 * 0x0CD444[1], then install 0x0CD412 with +0x0C past that sound word. */
static void retire(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x0001u;            /* 0x0CC54C move.w #1,D0 */
    uint32_t a1 = rd32(m, 0x000CD444u                     /* 0x0CD3D6 (0x6C,PC,D0w*4) */
                       + 4u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]);
    r->a[1] = a1 + 2u;
    uint16_t snd = rd16(m, a1);                              /* 0x0CD3DE move.w (A1)+,D0 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | snd;
    if (snd != 0u)                                           /* 0x0CD3E0 beq */
        call_hooked_sound(m, r, snd, 0x000CD3E6u, 0x000CD3ECu);

    wr32(m, a0 + 0x0Cu, a1 + 2u);                            /* 0x0CD3F6 */
    wr16(m, a0 + 0x3Au, 0u);                                 /* 0x0CD3FA */
    wr16(m, a0 + 0x3Eu, 0u);                                 /* 0x0CD3FE */
    wr16(m, a0 + 0x1Eu, 0u);                                 /* 0x0CD402 */
    wr16(m, a0 + 0x1Cu, 0u);                                 /* 0x0CD406 */
    wr32(m, a0, 0x000CD412u);                                /* 0x0CD40A */
}

int Fn_0CC4B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {            /* 0x0CC4B2 btst #1 */
        int16_t x = (int16_t)rd16(m, a0 + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {                       /* 0x0CC4BA / 0x0CC4C2 */
            retire(m, r);                                     /* 0x0CC4CA jmp 0x0CC54C */
            return 0;
        }
    }
    if ((int32_t)rd32(m, a0 + 0x24u) <= 0) {                  /* 0x0CC4D0 tst.l / ble */
        retire(m, r);
        return 0;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {            /* 0x0CC4D8 btst #4 */
        retire(m, r);
        return 0;
    }

    wr8(m, a0 + 0x1Fu, (uint8_t)(rd8(m, a0 + 0x1Fu) - 0x18u)); /* 0x0CC4E2 */
    wr16(m, a0 + 0x2Eu, (uint16_t)(rd16(m, a0 + 0x2Eu) + 3u)); /* 0x0CC4E8 */
    sub_0CC554(m, r);                                         /* 0x0CC4EC */

    uint16_t counter = (uint16_t)(rd16(m, a0 + 6u) - 1u);      /* 0x0CC4F0 */
    wr16(m, a0 + 6u, counter);
    if ((int16_t)counter > 0) {                               /* 0x0CC4F4 bgt 0x0CC4AA */
        wr32(m, a0, 0x000CC4B2u);
        return 0;                                             /* 0x0CC4B0 rts */
    }

    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x0018u;               /* 0x0CC4F6 move.w #0x18,D0 */
    r->d[0] = sub_0CBEB0(m, r);                              /* 0x0CC4FA jsr 0x0CBEB0 */
    wr16(m, a0 + 0x42u, (uint16_t)r->d[0]);                    /* 0x0CC500 */
    wr32(m, a0, 0x000CC50Cu);                                  /* 0x0CC504 */
    return 0;                                                  /* 0x0CC50A rts */
}
