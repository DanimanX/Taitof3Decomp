/* ROM 0x1245E0: boss aim/hover tick (A0 = object, A5+0x30DA = boss timer).
 * While the timer is above 0x78 the object steers: the angle to the aim point
 * (0x10E, 0x44 or 0xBC depending on A5+0x879 bit 5) is compared with its own
 * +0x20 heading and the heading is nudged 0x300 towards it, then the camera
 * position (A5+0x30D2 / A5+0x30D6) is restored to +0x14/+0x18, the object is
 * moved along +0x20 with +0x42 as its speed (0x0CBE12 = 0x11B262), and the
 * camera is re-latched from the new position.
 * Once the timer runs down it plays sound 0x14, clears A5+0x30EE and sets its
 * bit 0xF (0x00010000), clears A5+0x30F2, sets A5+0x30F3 = 0xFF, arms the
 * +0x0C fade-in value 0x12464E with a 0x50-frame timer and parks the tick at
 * 0x124B68. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"      /* Sound_PlayHooked */
#include "../objects/obj_shared.h"    /* Obj_Atan2 (0x006422) */
#include "../player/player_move.h"    /* Obj_MoveByAngle (0x0CBE12) */

#define TIMER     (F3_A5 + 0x30DAu)
#define AIM_FLAG  (F3_A5 + 0x0879u)
#define CAM_X     (F3_A5 + 0x30D2u)
#define CAM_Y     (F3_A5 + 0x30D6u)
#define MODE_L    (F3_A5 + 0x30EEu)
#define MODE_B    (F3_A5 + 0x30F2u)
#define MODE_C    (F3_A5 + 0x30F3u)
#define SIN_TBL   0x00027E40u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}


/* ROM 0x006422 as an inline block: the routine's `exg.l d1,d0` means the
 * returned D0 keeps the HIGH word of the dy operand (D1) when exactly one of
 * the two operands is negative, and of the dx operand (D0) otherwise. */
static uint32_t atan2_regs(const f3mem_t *m, uint32_t d0_in, uint32_t d1_in)
{
    uint16_t dx = (uint16_t)d0_in;
    uint16_t dy = (uint16_t)d1_in;
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int neg_dx = (int16_t)dx < 0;
    int neg_dy = (int16_t)dy < 0;
    uint32_t hi = (neg_dx != neg_dy) ? (d1_in & 0xFFFF0000u) : (d0_in & 0xFFFF0000u);
    return hi | ang;
}

/* ROM 0x11B262: move along +0x20 from the camera position, then re-latch it. */
static void sub_11B262(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *d1_io)
{
    wr32(m, obj + 0x14u, rd32(m, CAM_X));               /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, CAM_Y));               /* 0x11B268 */
    uint32_t d0 = setw(*d0_io, (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x11B26E..0x11B272 */
    uint16_t speed = rd16(m, obj + 0x42u);              /* 0x0CBE12 move.w $42(a0),d1 */
    uint16_t ang = (uint16_t)d0;
    int16_t c = (int16_t)rd16(m, SIN_TBL + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)rd16(m, SIN_TBL + (uint32_t)ang * 2u);
    uint32_t p1 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)c);
    uint32_t p2 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + p1);    /* 0x0CBE24 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + p2);    /* 0x0CBE2E */
    wr32(m, CAM_X, rd32(m, obj + 0x14u));               /* 0x11B27A */
    wr32(m, CAM_Y, rd32(m, obj + 0x18u));               /* 0x11B280 */
    *d0_io = d0;
    *d1_io = p1;                                        /* 0x0CBE12 leaves D1 = cos*speed */
}

int Fn_1245E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    if ((int16_t)rd16(m, TIMER) <= 0x0078) {            /* 0x1245E0 cmpi/ble */
        wr16(m, obj + 0x06u, 0x0050u);                  /* 0x12463A */
        wr32(m, obj + 0x0Cu, 0x0012464Eu);              /* 0x124640 */

        Sound_PlayHooked(m, 0x0014u);                   /* 0x124B3A..0x124B4A */
        wr32(m, MODE_L, 0u);                            /* 0x124B4E clr.l */
        /* 0x124B52 bfset (A5+0x30EE){0xF:1}: bit 15 from the MSB = 0x00010000. */
        wr32(m, MODE_L, rd32(m, MODE_L) | 0x00010000u);
        wr8(m, MODE_B, 0x00u);                          /* 0x124B58 sf */
        wr8(m, MODE_C, 0xFFu);                          /* 0x124B5C st */
        wr32(m, obj + 0x00u, 0x00124B68u);              /* 0x124B60 */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;
    }

    d0 = setw(d0, 0x010Eu);                             /* 0x1245EA */
    if ((rd8(m, AIM_FLAG) & 0x20u) != 0u)               /* 0x1245EE btst #5 */
        d1 = setw(d1, 0x00BCu);                         /* 0x124600 */
    else
        d1 = setw(d1, 0x0044u);                         /* 0x1245F8 */
    d0 = setw(d0, (uint16_t)(d0 - rd16(m, obj + 0x14u))); /* 0x124604 */
    d1 = setw(d1, (uint16_t)(d1 - rd16(m, obj + 0x18u))); /* 0x124608 */
    d0 = atan2_regs(m, d0, d1);                         /* 0x12460C */
    d0 = setw(d0, (uint16_t)(d0 << 8));                 /* 0x124612 lsl.w #8 */
    d0 = setw(d0, (uint16_t)(d0 - rd16(m, obj + 0x20u))); /* 0x124614 */
    if ((int16_t)d0 != 0) {                             /* 0x124618 tst/beq */
        if ((int16_t)d0 > 0)                            /* 0x12461E blt */
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0300u));
        else
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0300u));
    }

    sub_11B262(m, obj, &d0, &d1);                       /* 0x124632 */
    wr32(m, obj + 0x00u, 0x001245E0u);                  /* 0x124638 -> 0x1245D8 */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
