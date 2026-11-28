/* ROM 0x100A98: (recipe capture fnk_, work/zcap/recipes15.tsv): scrolling-enemy tick that draws its
 * sprite three times at successive heights (A0 = object).
 * Subtracts the camera scroll A5+0x4224/A5+0x4228 from (+0x14,+0x18), steps the
 * 6-byte script at +0x48 through 0x0D0690, and when the step timer +0x4C hits 0
 * starts dying (0x0CEE74). Otherwise it draws the body sprite (0x001282),
 * saves the sprite pointer +0x10, the flip byte +0x1C and the x word +0x14 on
 * the stack, draws the +0x68 sprite at x + 0x58 with flip = 1, draws it again at
 * x + 0x60, then restores the three saved fields. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x001282 as a call: Obj_DrawSprite plus the register trace of its
 * block-control loop (batch_sprite_outputs in batch_obj_regs.h, which
 * follows the single-sprite path at 0x13CE and the tile grid at 0x1330). */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    Obj_DrawSprite(m, r->a[0]);
    batch_sprite_outputs(m, r);
}

int Fn_100A98(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0;

    d0 = rd32(m, F3_A5 + 0x4224u);                        /* 0x100A98 move.l (0x4224,A5),D0 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);      /* 0x100A9C sub.l D0,(0x14,A0) */
    d0 = rd32(m, F3_A5 + 0x4228u);                        /* 0x100AA0 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);      /* 0x100AA4 */
    r->d[0] = d0;

    Obj_StepScript(m, obj);                               /* 0x100AA8 jsr 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) == 0u) {                     /* 0x100AAE tst.w / beq 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }

    draw_sprite_regs(m, r);                               /* 0x100AB8 jsr 0x001282 */

    /* 0x100ABE..0x100AC6: move.l (0x10,A0),-(A7) ;
     * move.b (0x1C,A0),-(A7) ; move.w (0x14,A0),-(A7) */
    {
        uint32_t sp = r->a[7];
        sp -= 4u; wr32(m, sp, rd32(m, obj + 0x10u));
        sp -= 2u; wr8(m, sp, rd8(m, obj + 0x1Cu));   /* byte push writes the high byte */
        sp -= 2u; wr16(m, sp, rd16(m, obj + 0x14u));
        r->a[7] = sp;
    }

    wr32(m, obj + 0x10u, rd32(m, obj + 0x68u));           /* 0x100ACA */
    d0 = (uint32_t)rd16(m, obj + 0x58u);                  /* 0x100AD0 move.w (0x58,A0),D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x100AD4 */
    draw_sprite_regs(m, r);                               /* 0x100AD8 jsr 0x001282 */

    wr8(m, obj + 0x1Cu, 0x01u);                           /* 0x100ADE move.b #1,(0x1C,A0) */
    d0 = (uint32_t)rd16(m, obj + 0x60u);                  /* 0x100AE4 move.w (0x60,A0),D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x100AE8 */
    draw_sprite_regs(m, r);                               /* 0x100AEC jsr 0x001282 */

    /* 0x100AF2 move.w (A7)+,(0x14,A0) ; 0x100AF6 move.b (A7)+,(0x1C,A0) ;
     * 0x100AFA move.l (A7)+,(0x10,A0) */
    {
        uint32_t sp = r->a[7];
        wr16(m, obj + 0x14u, rd16(m, sp));
        wr8(m, obj + 0x1Cu, rd8(m, sp + 2u));
        wr32(m, obj + 0x10u, rd32(m, sp + 4u));
        r->a[7] = sp + 8u;
    }
    return 0;                                             /* 0x100AFE rts */
}
