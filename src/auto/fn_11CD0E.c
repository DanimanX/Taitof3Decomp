/* ROM 0x11CD0E: camera/boss scroll-patrol phase (A0 = object).
 * If (int8_t)(A5+0x325C) >= 0x37, sets timer +0x06 = 0x78 and arms 0x11CDF8
 * (the beq after move.w #0x78 is never taken). Otherwise steps angle +0x20
 * toward +0x3A, advances scroll (A5+0x30D2/+0x30D6) via 0x11B262, bounces
 * +0x3A off [0xB4..0x104] x [0x58..0xAD], clears A5+0x30EE bit 2 when
 * +0x06 <= 0x3C, and decrements +0x06. When +0x06 reaches 0, picks speed 3 or
 * 8 from the 0x409D2C object list, aims toward (0xFA, 0x80) via 0x006422, sets
 * +0x06 = 0x96, and arms 0x11CD8C. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B262: step camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u)); /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u)); /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;       /* 0x11B26E..0x11B272 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    Obj_MoveByAngle(m, obj, ang);                   /* 0x11B274 -> 0x0CBE12 */
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)((int32_t)speed * (int32_t)c);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u)); /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u)); /* 0x11B280 */
}

/* ROM 0x11CF90: steer +0x20 toward +0x3A, step scroll, bounce +0x3A on box. */
static void sub_11CF90(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t diff = (int16_t)(rd16(m, obj + 0x3Au) - rd16(m, obj + 0x20u)); /* 0x11CF90..0x11CF94 */
    r->d[0] = setw(r->d[0], (uint16_t)diff);
    if (diff > 0)                                   /* 0x11CF9A..0x11CF9E */
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0200u));
    else if (diff < 0)
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0200u));

    sub_11B262(m, r);                               /* 0x11CFB2 */

    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 0x4000u)); /* 0x11CFB8 */
    if ((int16_t)rd16(m, obj + 0x14u) <= 0x00B4 && (int16_t)rd16(m, obj + 0x3Au) >= 0)
        wr16(m, obj + 0x3Au, (uint16_t)(0u - rd16(m, obj + 0x3Au))); /* 0x11CFD0 */
    if ((int16_t)rd16(m, obj + 0x14u) >= 0x0104 && (int16_t)rd16(m, obj + 0x3Au) <= 0)
        wr16(m, obj + 0x3Au, (uint16_t)(0u - rd16(m, obj + 0x3Au))); /* 0x11CFE6 */

    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x4000u)); /* 0x11CFEA */
    if ((int16_t)rd16(m, obj + 0x18u) <= 0x0058 && (int16_t)rd16(m, obj + 0x3Au) <= 0)
        wr16(m, obj + 0x3Au, (uint16_t)(0u - rd16(m, obj + 0x3Au))); /* 0x11D002 */
    if ((int16_t)rd16(m, obj + 0x18u) >= 0x00AD && (int16_t)rd16(m, obj + 0x3Au) >= 0)
        wr16(m, obj + 0x3Au, (uint16_t)(0u - rd16(m, obj + 0x3Au))); /* 0x11D018 */
}

int Fn_11CD0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x325Cu) >= 0x37) {  /* 0x11CD0E..0x11CD14 */
        wr16(m, obj + 0x06u, 0x0078u);              /* 0x11CDE8 (Z=0, 0x11CDEE beq folded) */
        wr32(m, obj, 0x0011CDF8u);                  /* 0x11CDF0 */
        return 0;                                   /* 0x11CDF6 */
    }

    sub_11CF90(m, r);                               /* 0x11CD18 */

    if ((int16_t)rd16(m, obj + 0x06u) <= 0x003C)    /* 0x11CD1C..0x11CD22 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x04u)); /* 0x11CD26 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11CD2C */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                         /* 0x11CD30 -> 0x11CD06 */
        wr32(m, obj, 0x0011CD0Eu);
        return 0;
    }

    wr16(m, obj + 0x3Au, rd16(m, obj + 0x20u));     /* 0x11CD32 */
    wr16(m, obj + 0x42u, 0x0003u);                  /* 0x11CD38 */

    uint16_t rem = rd16(m, F3_A5 + 0x1D34u);        /* 0x11CD3E */
    uint32_t a1 = 0x00409D2Cu;                      /* 0x11CD42 */
    int16_t obj_x = (int16_t)rd16(m, obj + 0x14u);
    for (;;) {                                      /* 0x11CD48..0x11CD5C */
        rem = (uint16_t)(rem - 1u);
        if ((int16_t)rem < 0)
            break;
        uint32_t a2 = rd32(m, a1);
        a1 += 4u;
        uint16_t d1w = (uint16_t)(rd16(m, a2 + 0x14u) + 0x0082u);
        r->d[1] = setw(r->d[1], d1w);
        if ((int16_t)d1w >= obj_x) {
            wr16(m, obj + 0x42u, 0x0008u);          /* 0x11CD5E */
            break;
        }
    }
    r->a[1] = a1;

    int16_t dx = (int16_t)(0x00FA - (int16_t)rd16(m, obj + 0x14u)); /* 0x11CD64..0x11CD68 */
    int16_t dy = (int16_t)(0x0080 - (int16_t)rd16(m, obj + 0x18u)); /* 0x11CD6C..0x11CD70 */
    uint16_t ang = Obj_AtanByte(m, dx, dy);         /* 0x11CD74 -> 0x006422 */
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = setw(r->d[1], (uint16_t)dy);

    wr8(m, obj + 0x20u, (uint8_t)ang);              /* 0x11CD7A */
    wr16(m, obj + 0x06u, 0x0096u);                  /* 0x11CD7E */
    wr32(m, obj, 0x0011CD8Cu);                      /* 0x11CD84 */
    return 0;                                       /* 0x11CD8A */
}
