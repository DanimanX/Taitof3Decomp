/* ROM 0x12EFFE: stage boss core / tentacle controller tick (A0 = object).
 * Draws 4 articulated tentacle/limb sprites + body/weapon parts via 0x12F356
 * (inheriting state from +0x0C, mirroring 4 angle/speed pairs from +0x4A,
 * drawing weapon phase <= 0x20 or dispatching A5+0x329E mode 0/1/2 via 0x12F494,
 * and drawing base 0x315A8). Advances +0x48/+0x46/+0x44, tracks player Y
 * (A5+0x30E2) into the 4 +0x4A angles with [-512, 512] deadband, and checks
 * A5+0x30F2 bit 0 (free) or A5+0x30EE bit 10 from MSB (countdown +0x48 to
 * frame-step death anim 0x12F1D8 and transition to 0x12F13C or 0x12EF40). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle], D2 = speed * sin[angle]. */
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
    r->d[2] = dy;
    r->a[3] = 0x00027E40u;
}

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0w)>>16). */
static uint16_t sub_0CC2BC(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    uint16_t diff = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)diff * (uint32_t)d0w) << 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t div_in = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t d1w = (uint16_t)d1;
    uint32_t q = div_in / d1w;
    uint32_t rem = div_in % d1w;
    uint32_t d0 = (q <= 0xFFFFu) ? ((q << 16) | rem) : div_in;
    r->d[0] = d0;
    r->d[1] = d1;
    return (uint16_t)d0;
}

/* ROM 0x12FF36: sync flash colour (+0x1D) with A5+0x32AA object. */
static void sub_12FF36(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);
    uint8_t d0b = rd8(m, a1 + 0x1Du);
    *d0_io = (*d0_io & 0xFFFFFF00u) | d0b;
    if (d0b == rd8(m, a1 + 0x4Fu))
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    else
        wr8(m, obj + 0x1Du, d0b);
    *a1_out = a1;
}

/* ROM 0x12F242: copy transform/colour from +0x0C owner and sync flash via 0x12FF36. */
static void sub_12F242(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x1Cu, rd8(m, a1 + 0x1Cu));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x1Eu));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr8(m, obj + 0x4Fu, 0x40u);
    sub_12FF36(m, obj, d0_io, a1_out);
}

/* ROM 0x12F4A0: mode 0 weapon draw + 0x130EE2 spawner. */
static void sub_12F4A0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x000Cu));
    wr32(m, obj + 0x10u, 0x000315C4u);
    draw_sprite_regs(m, r);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
    wr32(m, obj + 0x10u, 0x000315BCu);
    draw_sprite_regs(m, r);

    r->d[0] = Rng_NextMem(m);
    if (rd16(m, obj + 0x3Au) == 0u) {
        uint16_t d0w = rd16(m, obj + 0x3Eu);
        r->d[0] = setw(r->d[0], d0w);
        if (d0w == 0u) {
            Sound_PlayHooked(m, 0x0027u);
            wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));
        }
        d0w &= 0x0003u;
        r->d[0] = setw(r->d[0], d0w);
        if (d0w == 0u)
            r->d[0] = Obj_Spawn(m, obj, 0x00130EE2u, 9u, 2u, 0xE40E8000u);
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0x003C) {
        wr16(m, obj + 0x3Au, (uint16_t)~rd16(m, obj + 0x3Au));
        wr16(m, obj + 0x3Eu, 0u);
    }
}

/* ROM 0x12F53A: mode 1 weapon anim + 0x0CCD04 random bullet spawner. */
static void sub_12F53A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Cu));
    Obj_Animate(m, obj, 0x0012F5ACu);
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
    r->a[1] = 0x0012F5ACu;
    draw_sprite_regs(m, r);

    if (sub_0CC2BC(m, r, 4u) == 0u) {
        uint32_t d0 = Rng_NextMem(m);
        d0 = (d0 & 0x00001F00u) + 0x7000u;
        uint32_t d1 = d0 | 0xE4000000u;
        d0 = Rng_NextMem(m);
        d0 = setw(d0, (uint16_t)((d0 & 7u) + 0x0Au));
        d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0xFFu) << 16);
        r->d[1] = d1;
        r->d[0] = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);
    }
}

/* ROM 0x12F5C6: mode 2 weapon anim step + 6-shot 0x0CCF72 spread. */
static void sub_12F5C6(const f3mem_t *m, f3regs_t *r)
{
    static const uint32_t args[6] = {
        0xECFE8000u, 0xEC1D8000u, 0xF1067000u,
        0xF1169000u, 0xF50D6000u, 0xF50FA000u
    };
    uint32_t obj = r->a[0];
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x000Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Cu));

    for (;;) {
        Obj_AnimFrameStep(m, obj, 0x0012F6DCu);
        uint16_t off = rd16(m, obj + 0x3Au);
        r->d[0] = setw(r->d[0], off);
        r->a[1] = 0x0012F6DCu;
        int16_t dur = (int16_t)rd16(m, 0x0012F6DCu + (uint32_t)(int32_t)(int16_t)off);
        if (dur > 0)
            break;
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
        wr32(m, obj + 0x10u, rd32(m, 0x0012F6DCu + (uint32_t)(int32_t)(int16_t)off - 4u));
        if (sub_0CC2BC(m, r, 0x000Au) == 0u) {
            for (int i = 0; i < 6; i++)
                r->d[0] = Obj_Spawn(m, obj, 0x000CCF72u, 5u, 2u, args[i]);
            Sound_PlayHooked(m, 0x003Bu);
        }
        break;
    }
    draw_sprite_regs(m, r);
}

/* ROM 0x12F356: draw 4 tentacles + core/weapon + base sprite. */
static void sub_12F356(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    sub_12F242(m, obj, &r->d[0], &r->a[1]);
    wr8(m, obj + 0x1Cu, 0u);

    uint32_t owner = rd32(m, obj + 0x0Cu);
    if ((rd8(m, owner + 0x1Cu) & 0x01u) == 0u)
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0019u));
    else
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0046u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0078u));

    uint32_t d1_base = (uint32_t)rd8(m, obj + 0x44u);
    uint32_t a1 = obj + 0x4Au;
    for (int i = 3; i >= 0; i--) {
        uint16_t idx = (uint16_t)d1_base;
        if ((i & 1) != 0)
            idx = (uint16_t)(-(int16_t)idx + 7);
        wr32(m, obj + 0x10u, rd32(m, 0x0012F45Cu + (uint32_t)(int32_t)(int16_t)idx * 4u));
        uint16_t ang = rd16(m, a1) >> 8;
        wr16(m, obj + 0x42u, rd16(m, a1 + 2u));
        move_by_angle_regs(m, r, ang);
        draw_sprite_regs(m, r);
        a1 += 4u;
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x42u, 0x0044u);
    move_by_angle_regs(m, r, 0x00C9u);

    uint16_t save_x = rd16(m, obj + 0x14u);
    uint16_t save_y = rd16(m, obj + 0x18u);
    uint16_t mode = rd16(m, F3_A5 + 0x329Eu);
    r->d[0] = setw(r->d[0], mode);

    if ((int16_t)rd16(m, obj + 0x48u) <= 0x0020) {
        a1 = 0x0012F47Cu + (uint32_t)(int32_t)(int16_t)mode * 8u;
        int16_t d1w = (int16_t)(rd16(m, obj + 0x48u) - 3u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)d1w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 6u));
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
        r->d[1] = setw(r->d[1], (uint16_t)d1w);
        r->a[1] = a1;
        draw_sprite_regs(m, r);
        if (d1w > 0x000D) {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
            wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
            r->a[1] = a1;
            draw_sprite_regs(m, r);
        }
    } else {
        uint32_t target = rd32(m, 0x0012F494u + (uint32_t)(int32_t)(int16_t)mode * 4u);
        r->a[1] = target;
        if (mode == 0u)
            sub_12F4A0(m, r);
        else if (mode == 1u)
            sub_12F53A(m, r);
        else if (mode == 2u)
            sub_12F5C6(m, r);
    }

    wr16(m, obj + 0x18u, save_y);
    wr16(m, obj + 0x14u, save_x);
    wr32(m, obj + 0x10u, 0x000315A8u);
    draw_sprite_regs(m, r);
}

static int free_and_clear_owner(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                   /* 0x12F160 */
    wr8(m, a1 + 0x3Eu, 0u);
    Obj_Free(m, obj);
    r->a[1] = a1;
    return 0;
}

int Fn_12EFFE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    sub_12F356(m, r);                                    /* 0x12EFFE */

    if ((int16_t)rd16(m, obj + 0x48u) <= 0x0020)         /* 0x12F002 */
        wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) + 2u));

    uint16_t spd = rd16(m, obj + 0x46u);                 /* 0x12F010 */
    if ((int16_t)spd < 0x0080)
        wr16(m, obj + 0x46u, (uint16_t)(spd + 1u));

    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + spd)); /* 0x12F020 */
    wr8(m, obj + 0x44u, (uint8_t)(rd8(m, obj + 0x44u) & 0x07u));  /* 0x12F024 */

    uint32_t tgt = rd32(m, F3_A5 + 0x30E2u);             /* 0x12F02A */
    int16_t dy = (int16_t)((int16_t)(rd16(m, tgt + 0x18u) - rd16(m, obj + 0x18u)) << 2);
    if (dy < -0x0040) dy = -0x0040;
    if (dy >  0x0040) dy =  0x0040;
    int16_t d1w = (int16_t)(dy << 8);                    /* 0x12F050 */
    int16_t d2w = (int16_t)(-(int16_t)(d1w - (int16_t)(-0x8000))); /* 0x12F054..0x12F058 */

    uint32_t a1 = obj + 0x4Au;                           /* 0x12F05A */
    uint16_t d3w = (uint16_t)r->d[3];
    for (int d0w = 3; d0w >= 0; d0w--) {
        int16_t v = (int16_t)rd16(m, a1);
        v = (int16_t)(v - ((d0w >= 1) ? d1w : d2w));
        d3w = (uint16_t)v;
        if (v < -0x0200 || v > 0x0200) {                 /* 0x12F074 cmp2.w [-512, +512] */
            int16_t s = (int16_t)(v >> 5);
            d3w = (uint16_t)s;
            wr16(m, a1, (uint16_t)(rd16(m, a1) - (uint16_t)s));
        }
        a1 += 4u;
    }
    r->d[0] = setw(r->d[0], 0xFFFFu);
    r->d[1] = setw(r->d[1], (uint16_t)d1w);
    r->d[2] = setw(r->d[2], (uint16_t)d2w);
    r->d[3] = setw(r->d[3], d3w);
    r->a[1] = a1;

    /* 0x12EFE2..0x12F174 */
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u)
        return free_and_clear_owner(m, r, obj);

    uint8_t b = rd8(m, F3_A5 + 0x30EFu);                 /* 0x12EFEC bfclr (0x30EE,A5){0xA:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(b & ~0x20u));
    if ((b & 0x20u) == 0u) {
        wr32(m, obj, 0x0012EFFEu);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u)         /* 0x12F08E */
        return free_and_clear_owner(m, r, obj);

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x48u) - 1u); /* 0x12F098 */
    wr16(m, obj + 0x48u, cnt);
    if ((int16_t)cnt >= 0) {                             /* 0x12F09C */
        wr32(m, obj, 0x0012F0A8u);
        return 0;
    }

    wr16(m, obj + 0x3Au, 0u);                            /* 0x12F0FE */
    wr16(m, obj + 0x3Eu, 0u);
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u)         /* 0x12F106 */
        return free_and_clear_owner(m, r, obj);

    Obj_AnimFrameStep(m, obj, 0x0012F1D8u);              /* 0x12F110..0x12F116 */
    uint16_t off = rd16(m, obj + 0x3Au);
    r->d[0] = setw(r->d[0], off);
    r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
    if ((int16_t)rd16(m, 0x0012F1D8u + (uint32_t)(int32_t)(int16_t)off) <= 0) { /* 0x12F11C */
        a1 = rd32(m, obj + 0x0Cu);                       /* 0x12F154 */
        wr8(m, a1 + 0x3Eu, 0u);
        wr32(m, obj, 0x0012EF40u);
        r->a[1] = a1;
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);                           /* 0x12F124 */
    wr8(m, a1 + 0x3Eu, (rd32(m, obj + 0x10u) != 0x00028740u) ? 0xFFu : 0x00u);
    wr32(m, obj, 0x0012F13Cu);
    r->a[1] = a1;
    return 0;
}
