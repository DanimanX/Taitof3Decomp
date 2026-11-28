/* ROM 0x0E3462: body-segment driver (A0 = object).
 * Copies its parent's +0x06 gate into its own +0x06 (0 when the parent is 0),
 * counts +0x23 down and, when it hits 0, spawns 0x0E34FE (group 9, count 2,
 * arg 0), splices the new object into the +0x50/+0x54 list and reloads +0x23
 * with 7. Then walks the +0x54 list: every node whose +0x06 is 0 gets
 * A5+0x4228 subtracted from its +0x18 and is drawn with 0x001282. When the
 * list is empty and +0x06 is 0 the object starts dying (0x0CEE74). */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0E3462(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];

    r->a[1] = rd32(m, a0 + 0x08u);                      /* 0x0E3462 movea.l (0x8,A0),A1 */
    if (rd8(m, r->a[1] + 0x06u) == 0u)                       /* 0x0E3466 bne 0x0E3472 */
        wr8(m, a0 + 0x06u, 0u);                        /* 0x0E346E */

    if (rd8(m, a0 + 0x06u) != 0u) {                     /* 0x0E3472 beq 0x0E34CA */
        wr8(m, a0 + 0x23u, (uint8_t)(rd8(m, a0 + 0x23u) - 1u)); /* 0x0E347A */
        if (rd8(m, a0 + 0x23u) == 0u) {                 /* 0x0E347E bne 0x0E34CA */
            call_spawn(m, r, 0x000E349Au, 0x000E34FEu, 9u, 2u, 0u); /* 0x0E3482..0x0E349A */
            if ((int32_t)r->d[0] >= 0) {                /* 0x0E349E bmi 0x0E34C4 */
                uint32_t fresh = r->d[0];               /* 0x0E34A4 movea.l D0,A1 */
                uint32_t a2 = rd32(m, a0 + 0x54u);      /* 0x0E34A6 */
                wr8(m, fresh + 0x06u, 1u);              /* 0x0E34AA */
                wr32(m, a0 + 0x54u, fresh);             /* 0x0E34B0 */
                wr32(m, fresh + 0x50u, a0);             /* 0x0E34B4 */
                wr32(m, fresh + 0x54u, a2);             /* 0x0E34B8 */
                if ((int32_t)a2 >= 0)                   /* 0x0E34BC bmi 0x0E34C4 */
                    wr32(m, a2 + 0x50u, fresh);         /* 0x0E34C0 */
                r->a[1] = fresh;
            }
            wr8(m, a0 + 0x23u, 7u);                     /* 0x0E34C4 */
        }
    }

    uint32_t d0 = rd32(m, a0 + 0x54u);                  /* 0x0E34CA */
    if ((int32_t)d0 < 0) {                              /* 0x0E34CE bpl 0x0E34DE */
        if (rd8(m, a0 + 0x06u) == 0u) {                 /* 0x0E34D2 tst.b / beq 0x0CEE74 */
            Obj_StartDying(m, a0);                      /* 0x0CEE74 move.l #0xCEE7C,(A0) */
        }
        r->d[0] = d0;
        r->a[0] = a0;
        return 0;                                       /* 0x0E34DC rts */
    }

    for (;;) {
        a0 = d0;                                        /* 0x0E34DE movea.l D0,A0 */
        if (rd8(m, a0 + 0x06u) == 0u) {                 /* 0x0E34E0 bne 0x0E34F6 */
            d0 = rd32(m, F3_A5 + 0x4228u);              /* 0x0E34E8 */
            wr32(m, a0 + 0x18u, rd32(m, a0 + 0x18u) - d0); /* 0x0E34EC sub.l */
            r->a[0] = a0;
            draw_sprite_regs(m, r);                     /* 0x0E34F0 jsr 0x001282 */
        }
        d0 = rd32(m, a0 + 0x54u);                       /* 0x0E34F6 */
        if ((int32_t)d0 < 0)                            /* 0x0E34FA bpl 0x0E34DE */
            break;
    }

    r->d[0] = d0;
    r->a[0] = a0;
    return 0;                                           /* 0x0E34FC rts */
}
