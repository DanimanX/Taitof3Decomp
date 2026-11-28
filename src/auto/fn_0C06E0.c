/* ROM 0x0C06E0: UNVERIFIED (no capture): animated effect-object tick with a
 * signed vertical drift (A0 = object).
 * Adds 0x4000 to +0x14 (high word of the position long) and the signed
 * 0x027E40[+0x60 & 0xFF] table word to +0x18, steps the {dur.w, sprdef.l}
 * animation table at +0x5C through 0x0CBD24, and draws the sprite (0x001282)
 * while the duration the step stopped on is positive, advancing +0x60 by 2 and
 * re-arming 0x0C06E0 (0x0C071E bra 0x0C06D8 = `move.l #0xC06E0,(A0) ; rts`).
 * When the duration is <= 0 it clears the long at A5+0x8DA and frees the object
 * through 0x0105EC. */
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

int Fn_0C06E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x00004000u);   /* 0x0C06E0 addi.l #0x4000 */
    {
        /* 0x0C06E8 moveq #0,D0 ; 0x0C06EA move.w (0x60,A0),D0w ;
         * 0x0C06F4 andi.w #0xFF ; 0x0C06F8 move.w (0x27E40,D0w*2),D0w ;
         * 0x0C06FC ext.l D0 ; 0x0C06FE add.l D0,(0x18,A0) */
        uint32_t idx = (uint32_t)(rd16(m, obj + 0x60u) & 0x00FFu);
        int32_t dy = (int32_t)(int16_t)rd16(m, 0x00027E40u + idx * 2u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
        r->d[0] = (uint32_t)dy;
    }

    r->a[1] = rd32(m, obj + 0x5Cu);                       /* 0x0C0702 movea.l (0x5C,A0),A1 */
    Obj_AnimFrameStep(m, obj, r->a[1]);                   /* 0x0C0706 jsr 0x0CBD24 */
    /* 0x0CBD24 leaves D0w = the table offset the step ended on, i.e. the word
     * it left in +0x3A. 0x0C070C tst.w (A1,D0w) ; ble 0x0C0720. */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x3Au);
    {
        int16_t dur = (int16_t)rd16(m, r->a[1] + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]);
        if (dur > 0) {
            draw_sprite_regs(m, r);                       /* 0x0C0714 jsr 0x001282 */
            wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 2u)); /* 0x0C071A */
            wr32(m, obj, 0x000C06E0u);                    /* 0x0C071E bra 0x0C06D8 (re-arm) */
        } else {
            wr32(m, F3_A5 + 0x08DAu, 0u);                 /* 0x0C0720 clr.l (0x8DA,A5) */
            Obj_Free(m, obj);                             /* 0x0C0726 jsr 0x0105EC */
        }
    }
    return 0;                                             /* 0x0C0730 rts */
}
