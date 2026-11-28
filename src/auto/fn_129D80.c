/* ROM 0x129D80: boss camera script state (A0 = object).
 * Runs the three boss-camera services (0x11B288 steer +0x20 toward
 * (+0x50, +0x52) from the camera, 0x11B2CA ease +0x56 toward +0x54 into
 * +0x42, 0x11B262 integrate A5+0x30D2/30D6), then decrements the +0x06
 * countdown. While it stays positive the state re-arms 0x129D80; once it
 * expires the object is re-aimed at (+0xDC, +0x78) with speed 2, A5+0x30F2
 * is set to 1, a 0x50-frame timer is loaded and the script moves on to
 * 0x129DC0. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B288: steer +0x20.b by [-2..2] toward (+0x50, +0x52) - (A5+30D2, A5+30D6).
 * 0x006422 leaves D0's HIGH word untouched (0x00647A only writes D0.b and
 * 0x006488 masks the low word), so D0's high half survives the call. */
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

int Fn_129D80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11B288(m, r);                                         /* 0x129D80 */
    sub_11B2CA(m, r);                                         /* 0x129D86 */
    sub_11B262(m, r);                                         /* 0x129D8C */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x129D92 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                   /* 0x129D96 */
        wr32(m, obj, 0x00129D80u);                            /* 0x129D78 */
        return 0;
    }

    /* 0x129D98: countdown expired. 0x129DB6's beq can never be taken
     * (move.w #0x50 leaves Z clear), so 0x129DB8 always runs. */
    wr8(m, obj + 0x54u, 0x02u);
    wr16(m, obj + 0x50u, 0x00DCu);
    wr16(m, obj + 0x52u, 0x0078u);
    wr8(m, F3_A5 + 0x30F2u, 0x01u);
    wr16(m, obj + 0x06u, 0x0050u);
    wr32(m, obj, 0x00129DC0u);
    return 0;
}
