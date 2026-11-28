/* ROM 0x0E2CA0: UNVERIFIED (no capture): spawn/attach initialiser of a
 * projectile-like enemy (A0 = object).
 * A1 = object->parent->parent; when that grandparent's +0x06 phase byte is 4
 * the object starts dying (0x0CEE74). Otherwise it places itself at the
 * grandparent's (+0x14, +0x18) + (0x0A, -0x02) as words (high halves of the
 * position longs), seeds stats from the parameter record 0x000E2DCE through
 * 0x0CEDE4, sets +0x07 bit 4, installs frame record 0x0E2DEE[variant byte
 * +0x10] through 0x0CFA86 (entry 0 = NULL: no frame change), computes the
 * velocity vector (D0,D1) = 0x0D2D44(0x87, 0x28) into (+0x50, +0x54), draws
 * the sprite (0x001282), clears +0x34 bit 1 and arms the tick 0x0E2D1E. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x001282 as a call: Obj_DrawSprite plus the register trace of its
 * block-control loop (batch_sprite_outputs in batch_obj_regs.h, which
 * follows the single-sprite path at 0x13CE and the tile grid at 0x1330). */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    Obj_DrawSprite(m, r->a[0]);
    batch_sprite_outputs(m, r);
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

int Fn_0E2CA0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                   /* 0x0E2CA0 movea.l (0x8,A0),A1 */
    a1 = rd32(m, a1 + 0x08u);                             /* 0x0E2CA4 movea.l (0x8,A1),A1 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x06u) == 0x04u) {                    /* 0x0E2CA8 cmpi.b #4 / beq 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, 0u);                             /* 0x0E2CB4 clr.l (0x14,A0) */
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));            /* 0x0E2CB8 move.w */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Au)); /* 0x0E2CBE */
    wr32(m, obj + 0x18u, 0u);                             /* 0x0E2CC4 clr.l (0x18,A0) */
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));            /* 0x0E2CC8 move.w */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u)); /* 0x0E2CCE subq.w #2 */

    Obj_0CEDE4(m, obj, 0x000E2DCEu);                      /* 0x0E2CD2 lea (0xFA,PC),A1 ; jsr 0x0CEDE4 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x0E2CDC bset.b #4 */

    {
        /* 0x0E2CE2 clr.w D0w ; move.b (0x10,A0),D0b ;
         * 0x0E2CE8 movea.l (0x104,PC,D0w*4),A1 -- base = 0x0E2CEA + 0x104 =
         * 0x0E2DEE, a 4-entry table of frame-record pointers whose entry 0 is
         * NULL (no frame change; the variants 1..3 sit 10 bytes apart). */
        uint32_t idx = (uint32_t)rd8(m, obj + 0x10u);
        uint32_t frame = rd32(m, 0x000E2DEEu + idx * 4u);

        r->a[1] = frame;
        /* Entry 0 of the table is NULL, so variant 0 would load the frame
         * record at address 0 (the ROM's exception vectors), exactly as the
         * ROM does; the game only spawns variants 1..3. */
        Obj_SetFrame(m, obj, frame);                      /* 0x0E2CF0 jsr 0x0CFA86 */
    }

    {
        uint32_t d0, d1;
        /* 0x0E2CF6 move.w #0x87,D0w ; 0x0E2CFA moveq #0x28,D1 ; jsr 0x0D2D44 */
        sub_0D2D44(m, 0x0087u, 0x0028u, &d0, &d1);
        r->d[0] = d0;
        r->d[1] = d1;
        wr32(m, obj + 0x50u, d0);                         /* 0x0E2D02 move.l D0,(0x50,A0) */
        wr32(m, obj + 0x54u, d1);                         /* 0x0E2D06 move.l D1,(0x54,A0) */
    }
    draw_sprite_regs(m, r);                               /* 0x0E2D0A jsr 0x001282 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & 0xFDu)); /* 0x0E2D10 bclr.b #1 */
    wr32(m, obj, 0x000E2D1Eu);                            /* 0x0E2D16 move.l #0xE2D1E,(A0) */
    return 0;                                             /* 0x0E2D1C rts */
}
