/* ROM 0x183C32: aimed sweep-shot spawner/init (A0 = object).
 * Frees self if player ptr (A5+0x1D28) <= 0 or if parent-offset pos
 * (0x17BD68) falls outside [-16, 335] x [-16, 247] (0x1849F8). Otherwise aims
 * at player (0x17AF9A), sets sweep angle (+0x20) and direction (+0x43 = +/-1)
 * offset by +/-0x30 from opposite side, sets speed +0x42 = 0x60 and data
 * +0x0C = 0x17EFA6, computes +0x5C via 0x0CC2BC(0x0A), and arms 0x1840A6. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem, D1 = swapped scaled period (or 1). */
static void sub_0CC2BC(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint16_t div = (uint16_t)d1;
    uint16_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    r->d[0] = ((uint32_t)quot << 16) | (uint32_t)rem;
    r->d[1] = d1;
}

int Fn_183C32(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t player = rd32(m, F3_A5 + 0x1D28u);    /* 0x183C32 */

    if ((int32_t)player <= 0) {                     /* 0x183C36 -> 0x17C78E */
        Obj_Free(m, obj);
        return 0;
    }

    int32_t dx_ext = (int32_t)(int8_t)rd8(m, obj + 0x0Cu);
    int32_t dy_ext = (int32_t)(int8_t)rd8(m, obj + 0x0Du);
    Obj_InheritOffsetPos(m, obj);                   /* 0x183C3A -> 0x17BD68 */
    uint16_t x = rd16(m, obj + 0x50u);
    uint16_t y = rd16(m, obj + 0x54u);
    r->d[0] = ((uint32_t)dx_ext & 0xFFFF0000u) | x;
    r->d[1] = ((uint32_t)dy_ext & 0xFFFF0000u) | y;

    if ((int16_t)x < (int16_t)rd16(m, 0x001849F8u) || (int16_t)x > (int16_t)rd16(m, 0x001849FAu)) {
        Obj_Free(m, obj);                           /* 0x183C4A -> 0x17C78E */
        return 0;
    }

    r->d[0] = setw(r->d[0], y);                     /* 0x183C4E */
    if ((int16_t)y < (int16_t)rd16(m, 0x001849FCu) || (int16_t)y > (int16_t)rd16(m, 0x001849FEu)) {
        Obj_Free(m, obj);                           /* 0x183C5A -> 0x17C78E */
        return 0;
    }

    /* 0x183C5E..0x183C70: 0x17AF9A sets D0 = (player & 0xFFFF0000) | atan_byte */
    uint8_t ang = Obj_AimAtPlayer17AF9A(m, player, x, y);
    uint8_t opp = (uint8_t)(ang + 0x80u);           /* 0x183C74 addi.b #-0x80,D0b */
    uint16_t d0w = opp;
    uint8_t dir;
    if ((int8_t)opp >= 0) {                         /* 0x183C78..0x183C7A */
        d0w = (uint16_t)(d0w - 0x0030u);            /* 0x183C7E */
        dir = 1u;                                   /* 0x183C82 */
    } else {
        d0w = (uint16_t)(d0w + 0x0030u);            /* 0x183C8A */
        dir = 0xFFu;                                /* 0x183C8E */
    }
    r->d[0] = (player & 0xFFFF0000u) | d0w;
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | dir;

    wr8(m, obj + 0x20u, (uint8_t)d0w);              /* 0x183C92 */
    wr8(m, obj + 0x43u, dir);                       /* 0x183C96 */
    wr8(m, obj + 0x42u, 0x60u);                     /* 0x183C9A */
    wr32(m, obj + 0x0Cu, 0x0017EFA6u);              /* 0x183CA0 */

    sub_0CC2BC(m, r, 0x000Au);                      /* 0x183CA8..0x183CAC */
    wr16(m, obj + 0x5Cu, (uint16_t)r->d[1]);        /* 0x183CB2 */
    wr32(m, obj, 0x001840A6u);                      /* 0x183CB6 -> 0x18409E */
    return 0;
}
