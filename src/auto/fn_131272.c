#include "fn_auto.h"
/* ROM 0x131272: draw one sprite of a multi-part object (A0 = obj). A0+0x44
 * holds the source object; while it is non-null the position is copied from
 * its +0x14/+0x18 plus the offsets +0x54/+0x56, the object is pushed onto the
 * list at A5+0x906 through 0x0CBDAE unless A5-0x144B bit 4 is set, its sprite
 * definition +0x10 comes from +0x58 (byte +0x06 bit 1 set) or +0x5C, byte
 * +0x06 is incremented, the sprite is drawn (0x001282) and the tick re-arms
 * 0x131272. With A0+0x44 == 0 it just arms 0x1312D2. */
#include "../video/sprite.h"
#include "batch_obj_regs.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_131272(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, obj + 0x44u);            /* 0x131272 move.l (0x44,A0),D0 */
    if (d0 == 0u) {                                /* 0x131276 beq.w 0x1312CA */
        wr32(m, obj, 0x001312D2u);                 /* 0x1312CA */
        r->d[0] = 0u;
        return 0;                                  /* 0x1312D0 rts */
    }

    r->a[1] = d0;                                  /* 0x13127A movea.l D0,A1 */
    d0 = setw(d0, (uint16_t)(rd16(m, r->a[1] + 0x14u) + rd16(m, obj + 0x54u))); /* 0x13127C */
    wr16(m, obj + 0x14u, (uint16_t)d0);            /* 0x131284 */
    d0 = setw(d0, (uint16_t)(rd16(m, r->a[1] + 0x18u) + rd16(m, obj + 0x56u))); /* 0x131288 */
    wr16(m, obj + 0x18u, (uint16_t)d0);            /* 0x131290 */
    r->d[0] = d0;

    /* 0x131294 btst.b #4,(-0x144B,A5) ; 0x13129A bne.w 0x1312A4 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) == 0u) {
        uint32_t p = rd32(m, F3_A5 + 0x0906u);     /* 0x0CBDAE movea.l (0x906,A5),A1 */
        wr32(m, p, obj);                           /* 0x0CBDB2 move.l A0,(A1)+ */
        wr32(m, F3_A5 + 0x0906u, p + 4u);          /* 0x0CBDB4 move.l A1,(0x906,A5) */
    }

    if ((rd8(m, obj + 0x06u) & 0x02u) != 0u)       /* 0x1312A4 btst.b #1,(0x6,A0) */
        wr32(m, obj + 0x10u, rd32(m, obj + 0x58u)); /* 0x1312AE */
    else
        wr32(m, obj + 0x10u, rd32(m, obj + 0x5Cu)); /* 0x1312B8 */

    wr8(m, obj + 0x06u, (uint8_t)(rd8(m, obj + 0x06u) + 1u)); /* 0x1312BE addq.b #1 */
    batch_draw_sprite(m, r, 0x001312C8u);          /* 0x1312C2 jsr 0x001282 */
    wr32(m, obj, 0x00131272u);                     /* 0x13126A */
    return 0;                                      /* 0x131270 rts */
}
