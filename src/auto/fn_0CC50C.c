/* ROM 0x0CC50C: verified on an injected F3_NOIRQ capture (fnq_0CC50C): shot/effect object tick.
 * With A5+0x1D58 bit 1 set and X in [-10, 0x11C] the object is on screen and
 * goes straight to the 0x0CD3D6 conversion: sound entry 0x0CD444[1], the
 * record after it into +0x0C, the animation fields +0x3A/+0x3E/+0x1E/+0x1C
 * cleared, next state 0x0CD412 (D0w = 1 selects the table entry).
 * Otherwise, when HP (+0x24 long) is still positive and the A5-0x144B bit 4
 * flag is clear, it moves by angle +0x20 (0x0CBE12), appends itself to the
 * list at A5+0x906 and draws the sprite (0x001282); X still greater than the
 * 0x0CC112 limit re-arms 0x0CC50C, else it converts like the on-screen case.
 * Note: the shipped capture has a single pair whose post state was sampled
 * 678 frames after the entry (the runner skips it as VBlank-interrupted), so
 * this translation is checked against the bytes only. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle]. */
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
    r->d[1] = dx;
    r->a[3] = 0x00027E40u;
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

int Fn_0CC50C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    /* 0x0CC50C..0x0CC524: on-screen window test on X */
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u ||     /* btst.b #1 */
        x < -0x0A || x > 0x11C) {
        if ((int32_t)rd32(m, obj + 0x24u) <= 0)         /* 0x0CC52A tst.l/ble */
            goto convert;
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)    /* 0x0CC532 btst.b #4 */
            goto convert;

        /* 0x0CC53C bsr 0x0CC554 */
        uint16_t angle = rd16(m, obj + 0x20u);          /* 0x0CC554 move.w */
        d0 = setw(d0, angle);
        move_by_angle_regs(m, r, angle);                /* 0x0CC558 -> 0x0CBE12 */
        d1 = r->d[1];
        a1 = rd32(m, F3_A5 + 0x0906u);                  /* 0x0CC55E */
        wr32(m, a1, obj);                               /* 0x0CC562 */
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);                   /* 0x0CC564 */
        draw_sprite_regs(m, r);                         /* 0x0CC568 -> 0x001282 */
        d0 = r->d[0];
        d1 = r->d[1];
        a1 = r->a[1];

        int16_t x2 = (int16_t)rd16(m, obj + 0x14u);     /* 0x0CC540 */
        d1 = setw(d1, (uint16_t)x2);
        if (x2 > (int16_t)rd16(m, 0x000CC112u)) {       /* 0x0CC544 cmp.w/bgt */
            wr32(m, obj, 0x000CC50Cu);                  /* 0x0CC504 re-arm */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

convert:
    /* 0x0CC54C: D0w = 1 then bra 0x0CD3D6 */
    a1 = rd32(m, 0x000CD444u + 4u);                     /* table entry for D0w = 1 */
    uint16_t snd = rd16(m, a1);                         /* 0x0CD3DE */
    a1 += 2u;
    d0 = setw(1u, snd);
    if (snd != 0u)                                      /* 0x0CD3E0 beq */
        Sound_PlayHooked(m, snd);                       /* 0x0CD3E4..0x0CD3F2 */
    wr32(m, obj + 0x0Cu, a1);                           /* 0x0CD3F6 */
    wr16(m, obj + 0x3Au, 0u);                           /* 0x0CD3FA */
    wr16(m, obj + 0x3Eu, 0u);                           /* 0x0CD3FE */
    wr16(m, obj + 0x1Eu, 0u);                           /* 0x0CD402 */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x0CD406 */
    wr32(m, obj, 0x000CD412u);                          /* 0x0CD40A */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
