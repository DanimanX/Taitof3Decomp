/* ROM 0x0C8E1A: draw the two halves of a stage/tilemap object (A0 = object).
 * Divides the two counters +0x56 and +0x58 by the divisors in the two ROM
 * headers at 0x0C9008 (3) and 0x0C9022 (5), each time taking quotient ->
 * byte offset into the tile record table and remainder -> palette split, and
 * per half: blits one tilemap block to playfield/character RAM 0x61C000 and
 * queues two palette uploads (the first only when the remainder is non-zero).
 * Every register is saved by the entry movem and restored before rts, so the
 * register file is unchanged; only RAM/gfx change. */
#include "fn_auto.h"
#include "../video/sprite.h"
#include "../video/palette.h"

/* ROM 0x0C8E52: one half. A2 = header (ROM), D2.w = quotient, D1.w = remainder
 * (the caller's divu left it in the high word and swapped it down). */
static void sub_0C8E52(const f3mem_t *m, uint32_t obj, uint32_t a2,
                       uint16_t d1w, uint16_t d2w)
{
    uint16_t d0w = rd16(m, obj + 0x22u);                     /* 0x0C8E56 */
    uint32_t a0 = rd32(m, a2 + 0x0Au);                       /* 0x0C8E5A */
    a0 += (uint32_t)(int32_t)(int16_t)d0w * 4u;              /* 0x0C8E5E lea (0x0,A0,D0w*0x4),A0 */
    uint32_t d4 = rd32(m, a0 + (uint32_t)(int32_t)(int16_t)d2w * 8u); /* 0x0C8E62 */

    a0 = 0x000C914Au;                                        /* 0x0C8E66 lea (0x2e2,PC),A0 */
    uint32_t ai = a0 + (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint16_t d3w = rd16(m, ai);                              /* 0x0C8E6C */
    d0w = rd16(m, ai + 2u);                                  /* 0x0C8E70 */
    d3w = (uint16_t)(d3w + rd16(m, a2 + 0x4u));              /* 0x0C8E74 add.w (0x4,A2),D3w */
    d3w = (uint16_t)(d3w << 1);                              /* 0x0C8E78 */
    d0w = (uint16_t)(d0w << 7);                              /* 0x0C8E7A */
    d3w = (uint16_t)(d3w + d0w);                             /* 0x0C8E7C */
    a0 = 0x0061C000u;                                        /* 0x0C8E7E */
    a0 += (uint32_t)(int32_t)(int16_t)d3w;                   /* 0x0C8E84 adda.w D3w,A0 */

    /* 0x0C8E86 move.w #0,-(SP) ; 0x0C8E8A move.l A0,-(SP) ;
     * 0x0C8E8C move.l D4,-(SP) ; 0x0C8E8E jsr 0x0055C8 ; 0x0C8E94 lea (0xa,SP),SP */
    Gfx_DrawTilemapBlock(m, d4, a0, 0u);

    a0 = 0x000C915Eu;                                        /* 0x0C8E98 lea (0x2c4,PC),A0 */
    d0w = rd16(m, obj + 0x22u);                              /* 0x0C8E9E */
    d0w = rd16(m, a0 + 2u + (uint32_t)(int32_t)(int16_t)d0w * 4u); /* 0x0C8EA2 */
    d3w = rd16(m, a2 + 0x2u);                                /* 0x0C8EA6 */
    d3w = (uint16_t)(d3w << 2);                              /* 0x0C8EAA */
    d0w = (uint16_t)(d0w + d3w);                             /* 0x0C8EAC */
    d3w = d1w;                                               /* 0x0C8EAE move.w D1w,D3w */

    if (d3w != 0u) {                                         /* 0x0C8EB0 beq -> 0x0C8EC6 */
        /* 0x0C8EB4 push D3 ; 0x0C8EB6 push D0 ; 0x0C8EB8 push (0xe,A2) ;
         * 0x0C8EBC jsr 0x001660 ; 0x0C8EC2 lea (0xc,SP),SP */
        Pal_QueueUpload(m, rd32(m, a2 + 0xEu), d0w, d3w);
    }

    uint16_t d4w = rd16(m, a2);                              /* 0x0C8EC6 move.w (A2),D4w */
    d4w = (uint16_t)(d4w - d3w);                              /* 0x0C8EC8 sub.w D3w,D4w */
    d3w = (uint16_t)(d3w << 2);                              /* 0x0C8ECA lsl.w #2 */
    d0w = (uint16_t)(d0w + d3w);                             /* 0x0C8ECC add.w D3w,D0w */
    /* 0x0C8ECE push D4 ; 0x0C8ED0 push D0 ; 0x0C8ED2 push #0xc9242 ;
     * 0x0C8ED8 jsr 0x001660 ; 0x0C8EDE lea (0xc,SP),SP */
    Pal_QueueUpload(m, 0x000C9242u, d0w, d4w);
}

/* divu.w: 32-bit dividend / 16-bit divisor -> quotient in the low word,
 * remainder in the high word; on overflow the register is left unchanged. */
static uint32_t divu_w(uint32_t d, uint16_t divisor)
{
    uint32_t q = d / divisor;
    if (q > 0xFFFFu)
        return d;
    return ((d % divisor) << 16) | (q & 0xFFFFu);
}

int Fn_0C8E1A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = r->a[0];                           /* 0x0C8E1E movea.l A0,A1 */

    /* 0x0C8E20 lea (0x1e6,PC),A2 = 0x0C9008 ; 0x0C8E26..0x0C8E32 */
    uint32_t d1 = rd16(m, a1 + 0x56u);               /* moveq #0,D1 ; move.w (0x56,A1),D1w */
    d1 = divu_w(d1, rd16(m, 0x000C9008u));           /* divu.w (A2),D1 */
    uint16_t d2w = (uint16_t)d1;                     /* move.w D1w,D2w */
    uint32_t d1sw = (d1 << 16) | (d1 >> 16);         /* swap D1 */
    sub_0C8E52(m, a1, 0x000C9008u, (uint16_t)d1sw, d2w); /* 0x0C8E32 bsr */

    /* 0x0C8E36 lea (0x1ea,PC),A2 = 0x0C9022 ; 0x0C8E3C..0x0C8E48 */
    d1 = rd16(m, a1 + 0x58u);
    d1 = divu_w(d1, rd16(m, 0x000C9022u));
    d2w = (uint16_t)d1;
    d1sw = (d1 << 16) | (d1 >> 16);
    sub_0C8E52(m, a1, 0x000C9022u, (uint16_t)d1sw, d2w);

    (void)r;                                         /* restored by the entry movem */
    return 0;
}
