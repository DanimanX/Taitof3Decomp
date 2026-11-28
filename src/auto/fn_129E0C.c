/* ROM 0x129E0C: boss camera script state (A0 = object).
 * Runs the three boss-camera services (0x11B288 steer +0x20, 0x11B2CA ease
 * +0x56 into +0x42, 0x11B262 integrate A5+0x30D2/30D6) and decrements +0x06.
 * While it stays positive the state re-arms 0x129E0C; when it expires it
 * clears A5+0x30EE bit 13, zeroes the +0x54 speed byte, loads a 0xB4-frame
 * timer and hands the script to 0x129E3E. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B288: steer +0x20.b by [-2..2] toward (+0x50, +0x52) - (A5+30D2, A5+30D6).
 * 0x006422 leaves D0's HIGH word untouched (0x00647A writes only D0.b and
 * 0x006488 masks the low word), so the caller's high half survives. */
static void sub_11B288(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u));
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int8_t diff = (int8_t)((uint8_t)ang - rd8(m, obj + 0x20u));
    if (diff > 0) {
        if (diff > 2)
            diff = 2;
    } else if (diff < -2) {
        diff = -2;
    }
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)diff;
    r->d[1] = setw(r->d[1], dy);
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 asr 8) into +0x42. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);
    if (target > cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));
    else if (target < cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));
    int16_t spd = (int16_t)(rd16(m, obj + 0x56u) >> 8);
    wr16(m, obj + 0x42u, (uint16_t)spd);
    r->d[0] = setw(r->d[0], (uint16_t)spd);
}

/* ROM 0x11B262: reload +0x14/+0x18 from the camera, integrate along +0x20.b at
 * speed +0x42 through 0x0CBE12, write the camera position back. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    r->d[0] = setw(r->d[0], ang);

    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    int32_t vx = (int32_t)speed * (int32_t)c;
    int32_t vy = (int32_t)speed * (int32_t)s;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);
    r->d[1] = (uint32_t)vx;
    r->d[2] = (uint32_t)vy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

int Fn_129E0C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11B288(m, r);                                         /* 0x129E0C */
    sub_11B2CA(m, r);                                         /* 0x129E12 */
    sub_11B262(m, r);                                         /* 0x129E18 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x129E1E */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                   /* 0x129E22 */
        wr32(m, obj, 0x00129E0Cu);                            /* 0x129E04 */
        return 0;
    }

    /* 0x129E24: bitfield {13:1} at A5+0x30EE is bit 5 (0x04) of byte 0x30EF.
     * 0x129E34's beq can never be taken (move.w #0xB4 leaves Z clear). */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x04u));
    wr8(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x06u, 0x00B4u);
    wr32(m, obj, 0x00129E3Eu);                                /* 0x129E36 */
    return 0;
}
