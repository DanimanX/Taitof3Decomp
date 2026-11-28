#include "fn_auto.h"
/* ROM 0x121DAC: boss chase tick (A0 = obj). Steers the heading byte +0x20
 * toward the point (+0x3A,+0x3C) through 0x006422, nudging it by +-2 each
 * frame, then moves the object (camera-relative position from A5+0x30D2/D6,
 * angle +0x20 >> 8, speed +0x42 through 0x0CBE12). It then decrements the
 * timer +0x06: when it expires it clears A5+0x30EE, sets its bit 3 and clears
 * the parent's flag byte (A0+0x08)+0x34. While +0x06 > -0x78 it re-arms
 * 0x121DAC; otherwise it runs the setup block 0x121D68 (0x0CBEB0 with 0x3C
 * against A5+0x87E, random +0x3A/+0x3C) and re-arms 0x121DAC. */
#include "fn_boss_camera.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_121DAC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    /* 0x121DAC..0x121DBC: dx = +0x3A - +0x14, dy = +0x3C - +0x18 ; jsr 0x006422 */
    r->d[0] = setw(r->d[0], (uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, obj + 0x14u)));
    r->d[1] = setw(r->d[1], (uint16_t)(rd16(m, obj + 0x3Cu) - rd16(m, obj + 0x18u)));
    boss_angle(m, r, 0x00121DC2u);

    /* 0x121DC2 sub.b (0x20,A0),D0b ; 0x121DC6 tst.b D0b ; 0x121DC8 blt 0x121DD4 */
    r->d[0] = setw(r->d[0], (uint16_t)((uint16_t)r->d[0] - rd8(m, obj + 0x20u)));
    if ((int8_t)r->d[0] < 0)
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) - 2u)); /* 0x121DD4 */
    else
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + 2u)); /* 0x121DCC */

    /* 0x121DD8 jsr 0x0011B262: position from the camera, move by angle/speed. */
    wr32(m, obj + 0x14u, rd32(m, globals + 0x30D2u));  /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, globals + 0x30D6u));  /* 0x11B268 */
    r->d[0] = setw(r->d[0], (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x11B26E..0x11B272 */
    {
        uint16_t angle = (uint16_t)r->d[0];
        int16_t speed = (int16_t)rd16(m, obj + 0x42u);
        int16_t cy = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)angle * 2u);
        int16_t sy = (int16_t)rd16(m, 0x00027E40u + (uint32_t)angle * 2u);
        r->d[1] = (uint32_t)((int32_t)speed * cy);     /* 0x0CBE1E..0x0CBE24 */
        r->d[2] = (uint32_t)((int32_t)speed * sy);     /* 0x0CBE28..0x0CBE2E */
        r->a[3] = 0x00027E40u;                         /* 0x0CBE12 */
        Obj_MoveByAngle(m, obj, angle);                /* 0x0CBE12 */
    }
    wr32(m, globals + 0x30D2u, rd32(m, obj + 0x14u));  /* 0x11B27A */
    wr32(m, globals + 0x30D6u, rd32(m, obj + 0x18u));  /* 0x11B280 */

    /* 0x121DDE subq.w #1,(0x6,A0) ; 0x121DE2 bgt.w 0x121DF8 */
    uint16_t timer = rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(timer - 1u));
    if ((int16_t)timer <= 1) {
        wr32(m, globals + 0x30EEu, 0u);                /* 0x121DE6 */
        wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x10u)); /* 0x121DEA {3:1} = bit 4 */
        r->a[1] = rd32(m, obj + 0x08u);                /* 0x121DF0 */
        wr8(m, r->a[1] + 0x34u, 0u);                   /* 0x121DF4 sf (0x34,A1) */
    }

    /* 0x121DF8 cmpi.w #-0x78,(0x6,A0) ; 0x121DFE bgt.b 0x121DA4 */
    if ((int16_t)rd16(m, obj + 0x06u) > -0x78) {
        wr32(m, obj, 0x00121DACu);                     /* 0x121DA4 */
        return 0;                                      /* 0x121DAA rts */
    }

    /* 0x121E00 bra.w 0x121D68 */
    wr32(m, globals + 0x30EEu, 0u);                    /* 0x121D68 */
    uint16_t d1w = (uint16_t)((rd16(m, globals + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0 */
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)0x003Cu * (int32_t)(int16_t)d1w) << 4;
    d0 = (d0 << 16) | (d0 >> 16);                      /* 0x0CBEBE swap D0 */
    uint16_t cnt = (uint16_t)(0u - (uint16_t)((uint16_t)d0 - 0x0078u)); /* 0x121D76..0x121D7A */
    wr16(m, obj + 0x06u, cnt);                         /* 0x121D7C */

    uint32_t rng1 = Rng_NextMem(m);                    /* 0x121D80 */
    wr16(m, obj + 0x3Au, (uint16_t)((rng1 & 0x001Fu) + 0x00CDu)); /* 0x121D86..0x121D8E */

    uint32_t rng2 = Rng_NextMem(m);                    /* 0x121D92 */
    uint16_t y = (uint16_t)((rng2 & 0x007Fu) + 0x0040u); /* 0x121D98..0x121D9C */
    wr16(m, obj + 0x3Cu, y);                           /* 0x121DA0 */

    wr32(m, obj, 0x00121DACu);                         /* 0x121DA4 */
    r->d[0] = setw(rng2, y);
    r->d[1] = setw(r->d[1], d1w);
    return 0;                                          /* 0x121DAA rts */
}
