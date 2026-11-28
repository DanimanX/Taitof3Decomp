#include "fn_auto.h"
/* ROM 0x0E33C0: attached "sub-part" tick (A0 = object, A1 = the owner/definition
 * record at A0+0x08). If the owner's +0x06 byte is 0 the part dies at once
 * (0x0CEE74). Otherwise it copies the owner's +0x14/+0x18 longs (adding 0x11 and
 * 0x17 to their high words), draws the owner (0x001282 with A0 = owner), steps
 * its own animation/script (0x0D0690) and draws itself (0x001282). */
#include "../objects/obj_common.h"   /* Obj_StartDying (0x0CEE74) */
#include "../video/sprite.h"         /* Obj_DrawSprite (0x001282) */

/* ROM 0x0D0690 with D0/A1 register tracking (exact per the bytes: D0 is only
 * ever written by `move.w (A1)+,D0`, so its high half is cleared). */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)                                       /* 0x0D0694 beq ; 0x0D0698 rts */
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;          /* 0x0D069A movea.l ; adda.w #0xC */
    wr32(m, obj + 0x48u, a1);                            /* 0x0D06A2 */
    uint16_t d0 = rd16(m, a1);                           /* 0x0D06A6 move.w (A1)+,D0 */
    a1 += 2u;
    if ((int16_t)d0 < 0) {                               /* 0x0D06A8 bmi */
        int16_t back = (int16_t)rd16(m, a1);             /* 0x0D06C0 tst.w (A1) */
        if (back == 0) {                                 /* 0x0D06C2 beq 0x0D06DE */
            *d0_io = d0;
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;                   /* 0x0D06C6 suba.w (A1),A1 */
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));               /* 0x0D06CC */
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = pal;                                    /* 0x0D06D0 move.w (A1)+,D0 */
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);                            /* 0x0D06AC */
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = pal;                                        /* 0x0D06B0 move.w (A1)+,D0 */
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x001282 as 0x0E33EA/0x0E33F8 see it: A1 ends at the sprite cursor and
 * D0/D1 as the two exits of the drawer leave them.
 *  - single {1,1} header (0x0013CE): D0 = attr ^ (flip & 0xFF00), low byte
 *    replaced by the +0x1D colour when non-zero (0x0013EA..0x001406); D1.W =
 *    flip & 0xFF00 (0x0013EE..0x0013F2, sf.b).
 *  - tiled header: D0 = 0x0000FFFF (the dbra leaves 0xFFFF, 0x139E) and
 *    D1 = (rows-1)<<16 | 0xFFFF (0x1308 bfins put cols-1 in D1.W, the row dbra
 *    at 0x13C4 leaves 0xFFFF). */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint16_t flip = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        uint16_t attr = (uint16_t)(rd16(m, a2 + 6u) ^ flip);
        if (rd8(m, obj + 0x1Du) != 0u)
            attr = (uint16_t)((attr & 0xFF00u) | rd8(m, obj + 0x1Du));
        *d0_io = attr;
        *d1_io = flip;
    } else {
        uint16_t rows = (uint16_t)hdr;
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(rows - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0E33C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, a0 + 0x08u);                   /* 0x0E33C0 movea.l (8,A0),A1 */

    if (rd8(m, a1 + 0x06u) == 0u) {                      /* 0x0E33C4 tst.b ; beq.l 0x0CEE74 */
        Obj_StartDying(m, a0);
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, a0 + 0x14u, rd32(m, a1 + 0x14u));            /* 0x0E33CE move.l */
    wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) + 0x0011u)); /* 0x0E33D4 */
    wr32(m, a0 + 0x18u, rd32(m, a1 + 0x18u));            /* 0x0E33DA */
    wr16(m, a0 + 0x18u, (uint16_t)(rd16(m, a0 + 0x18u) + 0x0017u)); /* 0x0E33E0 */

    /* 0x0E33E6..0x0E33F0: push A0, A0 = A1 (the owner), draw it, pop A0 back. */
    draw_sprite_regs(m, a1, &d0, &d1, &a1);              /* 0x0E33EA jsr 0x001282 */
    step_script_regs(m, a0, &d0, &a1);                   /* 0x0E33F2 jsr 0x0D0690 */
    draw_sprite_regs(m, a0, &d0, &d1, &a1);              /* 0x0E33F8 jsr 0x001282 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;                                            /* 0x0E33FE rts */
}
