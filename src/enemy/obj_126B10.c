#include "obj_126B10.h"
#include "../player/player_move.h"
#include "../objects/obj_pool.h"

/* The three followers below are the static helpers of ROM 0x126B3E
 * (src/enemy/obj_126B3E.c, bit-exact against its own capture); they are not
 * exported through a header, so they are repeated here verbatim. */

/* ROM 0x006422: compass heading from a 16.16-free (D0w,D1w) offset */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint16_t d0 = d0_in, d1 = d1_in, d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t t = d0;
        d0 = d1;
        d1 = t;
    }

    d1 = (uint16_t)(d1 << 6);
    uint32_t a0 = 0x00026E40u + (uint32_t)(int32_t)(int16_t)d1;
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0));
    d0 = (uint16_t)(d0 + rd16(m, 0x00006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0011B288: turn obj+0x20 toward the leader by at most 2/frame */
static void sub_11B288(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u));
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u));

    d0 = sub_006422(m, d0, d1);
    /* sub.b (0x20,A0),D0b */
    uint8_t b = (uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if ((int8_t)b > 0) {
        if ((int8_t)b > 2)
            b = 0x02u;
    } else if ((int8_t)b < -2) {
        b = 0xFEu;
    }
    /* add.b D0b,(0x20,A0) */
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + b));
}

/* ROM 0x0011B2CA: ease obj+0x56 toward the target angle obj+0x54 */
static void sub_11B2CA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x54u) != rd8(m, obj + 0x56u)) {
        if ((int8_t)rd8(m, obj + 0x54u) > (int8_t)rd8(m, obj + 0x56u))
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));
        else
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));
    }
    wr16(m, obj + 0x42u, (uint16_t)((int16_t)rd16(m, obj + 0x56u) >> 8));
}

/* ROM 0x0011B262: follow the leader position, then re-store it */
static void sub_11B262(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

/* ROM 0x126B10: follow the leader for 0x64 ticks, then publish the hold
 * request (bfset (0x30EE,A5){0x14:1}) and re-arm itself for 0x190 ticks. */
int Obj_126B10_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x126B10: jsr 0x11B288; jsr 0x11B2CA; jsr 0x11B262 */
    sub_11B288(m, obj);
    sub_11B2CA(m, obj);
    sub_11B262(m, obj);

    /* 0x126B22: subq.w #1,(0x6,A0); bgt 0x126B08 */
    uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, c);
    if ((int16_t)c > 0) {
        wr32(m, obj + 0x00u, 0x00126B10u);
        return 0;
    }

    /* 0x126B28: bfset (0x30EE,A5){0x14:1} (offset 0x14 from the MSB) */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x14u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x14u % 8u))));
    }

    /* 0x126B2E: move.w #0x190,(0x6,A0); beq 0x126B56 never taken (Z=0) */
    wr16(m, obj + 0x06u, 0x0190u);
    wr32(m, obj + 0x00u, 0x00126B3Eu);   /* 0x126B36 */
    return 0;
}
