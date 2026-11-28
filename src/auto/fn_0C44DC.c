/* ROM 0x0C44DC: (recipe capture fnk_): animated effect object that spirals
 * up and out with two alternating sprite parts (A0 = object).
 * Clears the flip byte +0x1C, picks 0x027E40[(+0x60 + 0x40) & 0xFF] and scales
 * it by 0x3FC; the high word of that product is the zoom step. A negative value
 * is negated and sets flip = 2. The (inverted) low byte becomes the zoom byte
 * +0x1E and, *0x28 << 8, an offset added to +0x18 after it is seeded from
 * +0x4C. It then draws the +0x44 sprite (0x001282), then the +0x48 sprite
 * (0x001282), subtracts 0x3400 from +0x14 and either frees the object (x <
 * -0x700000: clears the long at A5+0x8CE + (+0xF)*4 and calls 0x0105EC) or,
 * when x <= 0x0A0000, integrates +0x60 += +0x5C and +0x5C += 0x80; both
 * in-range paths re-arm 0x0C44DC through the 0x0C44D4 thunk. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x001282 as a call: Obj_DrawSprite plus the register trace of its
 * block-control loop (batch_sprite_outputs in batch_obj_regs.h, which
 * follows the single-sprite path at 0x13CE and the tile grid at 0x1330). */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    Obj_DrawSprite(m, r->a[0]);
    batch_sprite_outputs(m, r);
}

int Fn_0C44DC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0;

    wr8(m, obj + 0x1Cu, 0x00u);                           /* 0x0C44DC clr.b (0x1C,A0) */
    {
        /* 0x0C44E0 move.w (0x60,A0),D0w ; addi.w #0x40 ; andi.w #0xFF ;
         * 0x0C44F2 move.w (0x27E40,D0w*2),D0w ; muls.w #0x3FC,D0w ; swap D0 */
        uint32_t idx = (uint32_t)((uint16_t)(rd16(m, obj + 0x60u) + 0x40u) & 0x00FFu);
        int32_t prod = (int32_t)(int16_t)rd16(m, 0x00027E40u + idx * 2u) * 0x03FC;
        d0 = ((uint32_t)prod << 16) | ((uint32_t)prod >> 16); /* swap */
        if ((int16_t)(uint16_t)d0 < 0) {                  /* 0x0C44FC tst.w / bge */
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)(-(int16_t)(uint16_t)d0); /* neg.w */
            wr8(m, obj + 0x1Cu, 0x02u);                   /* 0x0C4504 move.b #2,(0x1C,A0) */
        }
    }
    d0 = (d0 & 0xFFFFFF00u) | (uint8_t)~d0;               /* 0x0C450A not.b D0b */
    wr8(m, obj + 0x1Eu, (uint8_t)d0);                     /* 0x0C450C move.b D0b,(0x1E,A0) */

    wr32(m, obj + 0x18u, rd32(m, obj + 0x4Cu));           /* 0x0C4510 */
    d0 = ((d0 & 0x00FFu) * 0x28u) << 8;                   /* 0x0C4516..0x0C451E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);      /* 0x0C4520 */

    wr32(m, obj + 0x10u, rd32(m, obj + 0x44u));           /* 0x0C4524 */
    draw_sprite_regs(m, r);                               /* 0x0C452A jsr 0x001282 */
    wr32(m, obj + 0x10u, rd32(m, obj + 0x48u));           /* 0x0C4530 */
    draw_sprite_regs(m, r);                               /* 0x0C4536 jsr 0x001282 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00003400u); /* 0x0C453C */
    if ((int32_t)rd32(m, obj + 0x14u) < (int32_t)0xFF900000u) { /* 0x0C4544..0x0C454C blt */
        /* 0x0C4570 clr.w D0w ; 0x0C4572 move.b (0xF,A0),D0b: the word clear
         * keeps D0's high word, which the second 0x001282 left as
         * ((x & 0xFFF) << 16) (fnk_0C44DC pairs 2/5/...: 0x0F000003). */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint32_t)rd8(m, obj + 0x0Fu);
        d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* (A1,D0w*4) index */
        r->a[1] = F3_A5 + 0x08CEu;                        /* 0x0C4576 lea (0x8CE,A5),A1 */
        wr32(m, F3_A5 + 0x08CEu + d0 * 4u, 0u);           /* 0x0C4576..0x0C457A clr.l */
        Obj_Free(m, obj);                                 /* 0x0C4580 jsr 0x0105EC */
        return 0;                                         /* 0x0C458A rts */
    }
    if ((int32_t)rd32(m, obj + 0x14u) > 0x00A00000) {     /* 0x0C4550..0x0C4558 bgt */
        wr32(m, obj, 0x000C44DCu);                        /* 0x0C44D4 re-arm thunk */
        return 0;
    }
    d0 = rd32(m, obj + 0x5Cu);                            /* 0x0C455C move.l (0x5C,A0),D0 */
    r->d[0] = d0;
    wr32(m, obj + 0x60u, rd32(m, obj + 0x60u) + d0);      /* 0x0C4560 */
    wr32(m, obj + 0x5Cu, rd32(m, obj + 0x5Cu) + 0x00000080u); /* 0x0C4564 */
    wr32(m, obj, 0x000C44DCu);                            /* 0x0C456C bra 0x0C44D4 (re-arm) */
    return 0;
}
