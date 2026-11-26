/* ROM 0x112FF6: boss/enemy charge-up aura and beam/ramp sub-object (A0 = object).
 * If (+0x36)->+0x65 bit 0 is set, clears parent (+0x08)->+0x24 and installs
 * 0x1131A0 (free). Otherwise animates charge ring 0x1131B8, tracks target
 * (+0x36) + (12, 8) along +0x20, draws sprite, queues palette upload
 * (0x1136EA or 0x016980 from +0x0C), jitters parent position via Rng_NextMem,
 * and counts down +0x06 while re-arming 0x112FF6. When +0x06 <= 0, sets +0x50
 * and parent flags, queues palette ramp (0x0018BA), installs (+0x52, +0x54)
 * and new script pointer in +0x0C, steps animation +0x0C via 0x0CBD24, follows
 * parent (0x113400), draws sprite, clears parent +0x24 if (+0x36)->+0x65 bit 0,
 * and arms 0x113146 (or 0x1131A0 on negative terminator). */
#include "fn_auto.h"
#include "../enemy/obj_112C3C.h"
#include "../objects/obj_common.h"
#include "../player/player_move.h"
#include "../video/palette.h"
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
}

int Fn_112FF6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x36u);                 /* 0x112FF6 */
    if ((rd8(m, a1 + 0x65u) & 0x01u) != 0u) {           /* 0x112FFA */
        a1 = rd32(m, obj + 0x08u);                       /* 0x1131AE */
        wr32(m, a1 + 0x24u, 0u);
        wr32(m, obj, 0x001131A0u);
        r->a[1] = a1;
        return 0;
    }

    Obj_Animate(m, obj, 0x001131B8u);                    /* 0x113004..0x11300A */
    wr32(m, obj + 0x14u, 0u);                            /* 0x113010 */
    wr32(m, obj + 0x18u, 0u);                            /* 0x113014 */

    a1 = rd32(m, obj + 0x36u);                           /* 0x113018 */
    uint16_t x = (uint16_t)(rd16(m, a1 + 0x14u) + 0x000Cu);
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, a1 + 0x18u) + 0x0008u);
    wr16(m, obj + 0x18u, y);
    r->d[0] = setw(r->d[0], x);
    r->d[1] = setw(r->d[1], y);

    move_by_angle_regs(m, r, rd16(m, obj + 0x20u));      /* 0x113032..0x113036 */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 0x10u)); /* 0x11303C */
    draw_sprite_regs(m, r);                              /* 0x113042 */

    /* 0x113048..0x11307A */
    a1 = rd32(m, obj + 0x0Cu);
    uint32_t d0 = rd32(m, a1 + 4u);
    uint16_t dst = rd16(m, a1 + 8u);
    uint16_t cnt = rd16(m, a1 + 10u);
    r->d[0] = d0;
    r->d[1] = setw(r->d[1], dst);
    r->d[2] = setw(r->d[2], cnt);
    uint32_t src = ((rd8(m, obj + 0x07u) & 0x02u) == 0u) ? 0x001136EAu : 0x00016980u;
    Pal_QueueUpload(m, src, dst, cnt);

    /* 0x11307E..0x11309A */
    a1 = rd32(m, obj + 0x08u);
    r->a[1] = a1;
    int32_t jx = (int32_t)(int16_t)(uint16_t)Rng_NextMem(m) << 2;
    wr32(m, a1 + 0x14u, rd32(m, a1 + 0x14u) + (uint32_t)jx);
    uint32_t rng2 = Rng_NextMem(m);
    int32_t jy = (int32_t)(int16_t)(uint16_t)rng2 << 2;
    wr32(m, a1 + 0x18u, rd32(m, a1 + 0x18u) + (uint32_t)jy);
    r->d[0] = (uint32_t)jy;

    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x11309E */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {                                /* 0x1130A2 */
        wr32(m, obj, 0x00112FF6u);                       /* 0x112FEE */
        return 0;
    }

    /* 0x1130A6..0x1130F8 */
    wr8(m, obj + 0x50u, 0xFFu);
    a1 = rd32(m, obj + 0x08u);
    wr8(m, a1 + 0x50u, rd8(m, a1 + 0x50u) | 0x04u);
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x4Fu));
    wr8(m, obj + 0x1Cu, 0u);

    a1 = rd32(m, obj + 0x0Cu);
    uint32_t ramp_d0 = rd32(m, a1 + 4u);
    uint16_t ramp_d1 = rd16(m, a1 + 8u);
    uint16_t ramp_d2 = rd16(m, a1 + 10u);
    a1 += 12u;
    r->d[0] = Pal_QueueRamp(m, 0x0000198Eu, ramp_d0,
                            (r->d[1] & 0xFFFF0000u) | ramp_d1,
                            0x00000050u,
                            (r->d[2] & 0xFFFF0000u) | ramp_d2);
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x54u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);

    /* 0x1130FC..0x113144 */
    for (;;) {
        a1 = rd32(m, obj + 0x0Cu);
        Obj_AnimFrameStep(m, obj, a1);                   /* 0x113100 */
        int16_t dur = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au));
        if (dur < 0) {                                   /* 0x11310A */
            uint32_t p = rd32(m, obj + 0x08u);
            wr8(m, p + 0x50u, (uint8_t)((rd8(m, p + 0x50u) & ~0x06u) | 0x08u));
            wr32(m, obj, 0x001131A0u);
            r->a[1] = p;
            r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
            return 0;
        }
        if (dur > 0)                                     /* 0x11310E */
            break;
        wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) | 0x01u);
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
    }

    Obj_113400_FollowParent(m, obj);                     /* 0x11311E */
    draw_sprite_regs(m, r);                              /* 0x113122 */
    a1 = rd32(m, obj + 0x36u);                           /* 0x113128 */
    if ((rd8(m, a1 + 0x65u) & 0x01u) != 0u) {
        a1 = rd32(m, obj + 0x08u);
        wr32(m, a1 + 0x24u, 0u);
    }
    r->a[1] = a1;
    wr32(m, obj, 0x00113146u);                           /* 0x11313E */
    return 0;
}
