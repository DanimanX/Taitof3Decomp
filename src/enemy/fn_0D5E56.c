#include "fn_0D5E56.h"
#include "../collision/collision.h"
#include "../objects/obj_pool.h"
#include "obj_124E30.h"     /* Aim_0CC33A (0x0CC33A) */

/* ROM 0x0CC1E0: true when any object in the A5+0x1D2C list whose centre is
 * within the two windows around this object's centre. cmp2.w D1,D2 compares
 * D2 (the pair after the extension word) with D1: bcs = D2 < D1 = in range.
 * The windows are the sign-extended words at 0x0CC242 / 0x0CC244
 * (-0x50 / +0x50). */
int Enemy_0CC1E0(const f3mem_t *m, uint32_t a0)
{
    uint32_t a1 = F3_A5 + 0x1D2Cu;
    int16_t n = (int16_t)(rd16(m, F3_A5 + 0x1D34u) - 1u);
    if (n < 0)
        return 0;
    do {
        uint32_t a2 = rd32(m, a1);
        a1 += 4u;
        int16_t d1 = (int16_t)(uint16_t)(rd16(m, a0 + 0x2Cu) + rd16(m, a0 + 0x2Eu)
                                         - rd16(m, a2 + 0x2Cu) - rd16(m, a2 + 0x2Eu));
        d1 = (int16_t)((uint16_t)(d1 >> 1) + rd16(m, a0 + 0x14u));
        d1 = (int16_t)((uint16_t)d1 - rd16(m, a2 + 0x14u));
        if (d1 < -0x50 || d1 > 0x50)
            continue;                                       /* bcs 0x0CC1E8 */
        d1 = (int16_t)(uint16_t)(rd16(m, a0 + 0x30u) + rd16(m, a0 + 0x32u)
                                 - rd16(m, a2 + 0x30u) - rd16(m, a2 + 0x32u));
        d1 = (int16_t)((uint16_t)(d1 >> 1) + rd16(m, a0 + 0x18u));
        d1 = (int16_t)((uint16_t)d1 - rd16(m, a2 + 0x18u));
        if (d1 < -0x50 || d1 > 0x50)
            continue;
        return 1;                                           /* st.b D0 */
    } while (--n >= 0);
    return 0;                                               /* sf.b D0 */
}

/* ROM 0x0CC24A: true while the object's centre is inside the screen box
 * (0x0CC288 = {minx, miny}, 0x0CC28C = {maxx, maxy}, both as words). */
int Enemy_0CC24A(const f3mem_t *m, uint32_t a0)
{
    int16_t d1 = (int16_t)(uint16_t)(rd16(m, a0 + 0x2Cu) + rd16(m, a0 + 0x2Eu));
    d1 = (int16_t)((uint16_t)(d1 >> 1) + rd16(m, a0 + 0x14u));
    if (d1 < (int16_t)rd16(m, 0x0000CC288u) || d1 > (int16_t)rd16(m, 0x0000CC28Cu))
        return 0;                                           /* bcs 0x0CC284 */
    d1 = (int16_t)(uint16_t)(rd16(m, a0 + 0x30u) + rd16(m, a0 + 0x32u));
    d1 = (int16_t)((uint16_t)(d1 >> 1) + rd16(m, a0 + 0x18u));
    if (d1 < (int16_t)rd16(m, 0x0000CC28Au) || d1 > (int16_t)rd16(m, 0x0000CC28Eu))
        return 0;
    return 1;                                               /* sf.b D0 */
}

/* ROM 0x0CC306: risk roll 0x0D0C9E() >= (avg of D0w and D1w). The mid-value
 * is halved with asr.w, the roll is 0..255, and sle.b D0 sets the byte for
 * (roll - mid) >= 0. */
int Enemy_0CC306(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    uint16_t mid = (uint16_t)(d0w + d1w);
    mid = (uint16_t)((int16_t)mid >> 1);
    uint16_t roll = rd16(m, 0x0D0D42u + (uint32_t)(rd8(m, F3_A5 + 0x087Fu) >> 3) * 2u);
    return (int16_t)roll >= (int16_t)mid;
}

/* ROM 0x0D5E56 */
int Obj_0D5E56_AI(const f3mem_t *m, uint32_t obj)
{
    if (Enemy_0CC1E0(m, obj))
        return 0;                                   /* jsr 0x0CC1E0 ; bne 0x0D5EB0 */
    if (Enemy_0CC24A(m, obj))
        return 0;                                   /* jsr 0x0CC24A ; bne 0x0D5EB0 */
    if (Enemy_0CC306(m, 0u, 0u))
        return 0;                                   /* move.w #0,D1 ; jsr 0x0CC306 */
    uint32_t a1;
    uint16_t ang = Aim_0CC33A(m, obj, 0x0010u, 0x0010u, &a1);
    uint32_t arg = 0x01010000u | (uint32_t)(ang << 8);
    /* 0x0D5E96: pea arg ; move.w #2 ; move.w #9 ; pea 0x0D901A ; jsr 0x1057C */
    /* (the parent A0 of the caller, obj, is this routine's own argument) */
    (void)Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, arg);
    return 0;
}
