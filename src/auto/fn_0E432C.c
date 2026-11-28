#include "fn_auto.h"
/* ROM 0x0E432C: trailing part of an object that follows its parent (A0 = obj,
 * A1 = (A0+0x08)). If the parent's +0x06 byte is 0 the object starts dying
 * (0x0CEE74 -> arms 0x0CEE7C). Otherwise it adopts the parent's angle +0x1E
 * and position +0x14/+0x18, offsets the position by 0x11/0x17 pixels rotated
 * by the zero-extended angle byte +0x1E through 0x0D4A2C (16.16 scale table
 * 0x0D4A5A), draws the parent's sprite, steps this object's animation
 * (0x0D0690) and draws its own sprite. */
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"   /* Obj_AnimStep (0x0D0690) */
#include "../video/sprite.h"
#include "batch_obj_regs.h"

/* ROM 0x0D4A2C: D4 = table[D2w] (0x0D4A5A, 4-byte 16.16 entries); the jump
 * table 0x0D4E5A then multiplies D4 by the factor selected by D1.w (first,
 * result swapped into D1) and by the factor selected by D0.w (second, result
 * swapped into D0). Index 0..50 scales D0 by the index, 51..63 scales both by
 * index-51; D3/D4 are saved and restored, D2 ends holding D4. */
static uint32_t scale_0D4A2C(uint32_t src, uint32_t factor)
{
    uint32_t p = src * factor;                   /* 0x0D4E5A handlers: lsl/add */
    return (p << 16) | (p >> 16);                /* 0x0D4A44 / 0x0D4A52 swap D0 */
}

int Fn_0E432C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[1] = rd32(m, obj + 0x08u);              /* 0x0E432C movea.l (8,A0),A1 */
    if (rd8(m, r->a[1] + 0x06u) == 0u) {         /* 0x0E4334 beq.l 0x0CEE74 */
        Obj_StartDying(m, obj);                  /* 0x0CEE74 move.l #0xCEE7C,(A0) */
        return 0;
    }

    wr16(m, obj + 0x1Eu, rd16(m, r->a[1] + 0x1Eu));  /* 0x0E433A */
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));  /* 0x0E4340 */
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));  /* 0x0E4346 */

    /* 0x0E434C moveq #0x11,D0 ; 0x0E434E moveq #0x17,D1 ;
     * 0x0E4350 clr.w D2w ; 0x0E4352 move.b (0x1E,A0),D2b ;
     * 0x0E4356 jsr 0x0D4A2C */
    uint32_t d0 = r->d[0], d1 = r->d[1];
    uint16_t idx = rd8(m, obj + 0x1Eu);
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)idx * 4u);
    d1 = scale_0D4A2C(v, 0x17u);                 /* index 0x17 -> 0x0D514E: *23 */
    d0 = scale_0D4A2C(v, 0x11u);                 /* index 0x11 -> 0x0D50D2: *17 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = v;                                 /* handler scratch (D2 = D4) */

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x0E435C */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1)); /* 0x0E4360 */

    /* 0x0E4364 move.l A0,-(SP) ; 0x0E4366 movea.l A1,A0 ; 0x0E4368 jsr 0x001282 */
    r->a[7] -= 4u;
    wr32(m, r->a[7], obj);
    r->a[0] = r->a[1];
    batch_draw_sprite(m, r, 0x0E436Eu);

    r->a[0] = rd32(m, r->a[7]);                  /* 0x0E436E movea.l (SP)+,A0 */
    r->a[7] += 4u;
    Obj_AnimStep(m, obj);                        /* 0x0E4370 jsr 0x0D0690 */
    batch_draw_sprite(m, r, 0x0E437Cu);          /* 0x0E4376 jsr 0x001282 */
    return 0;                                    /* 0x0E437C rts */
}
