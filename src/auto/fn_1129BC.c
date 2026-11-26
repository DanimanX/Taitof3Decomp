/* ROM 0x1129BC: parallax/scrolling foreground object tick (A0 = object).
 * Re-arms 0x1129BC if (+0x0C).w > A5+0x3384. Otherwise advances X (+0x14) by
 * -((A5+0x4224>>8) * (A5+0x4234>>8) >> 8) * (+0x44>>8), draws sprite (0x001282),
 * and frees via Obj_Free (0x0105EC) if X.w < -0xA0 else re-arms 0x1129BC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

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

int Fn_1129BC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, obj + 0x0Cu);                    /* 0x1129BC */
    r->d[0] = d0;

    if ((int16_t)d0 > (int16_t)rd16(m, F3_A5 + 0x3384u)) { /* 0x1129C0..0x1129C4 */
        wr32(m, obj, 0x001129BCu);                         /* 0x1129B4 */
        return 0;
    }

    int32_t sx = ASR32((int32_t)rd32(m, F3_A5 + 0x4224u), 8); /* 0x1129C6..0x1129CE */
    int32_t sy = ASR32((int32_t)rd32(m, F3_A5 + 0x4234u), 8); /* 0x1129CA..0x1129D0 */
    int32_t p = (int32_t)(int16_t)sx * (int32_t)(int16_t)sy;  /* 0x1129D2 */
    p = ASR32(p, 8);                                          /* 0x1129D8 */
    int32_t spd = ASR32((int32_t)rd32(m, obj + 0x44u), 8);    /* 0x1129D4..0x1129DA */
    p = (int32_t)(int16_t)p * (int32_t)(int16_t)spd;          /* 0x1129DC */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - (uint32_t)p); /* 0x1129DE */

    draw_sprite_regs(m, r);                                   /* 0x1129E2 */

    if ((int16_t)rd16(m, obj + 0x14u) < -0x00A0) {            /* 0x1129E8..0x1129EE */
        Obj_Free(m, obj);                                     /* 0x1129F6 -> 0x0105EC */
        return 0;
    }

    wr32(m, obj, 0x001129BCu);                                /* 0x1129F2 -> 0x1129B4 */
    return 0;
}
