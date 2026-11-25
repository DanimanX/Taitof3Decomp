#include "obj_DC49A.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0DC49A: enemy spawn body of the 0xDC49A family (the same code that
 * 0xDC586 / 0xDC5B6 run inline after their sound command). Off-screen or off
 * the +0x60 speed limit it hands over to the dying sequence 0x0CEE74; else it
 * nudges +0x14/+0x18 by -0x18, seeds the record at 0x0D8FE2, picks one of the
 * three stat records 0x0DC124 / 0x0DC1CC / 0x0DC238 from a 4-way random
 * (<=0x33 / <=0x99 / else), aims at +0x58 + (rnd & 0x3F) - 0x20 wrapped to
 * [0,0x168), derives the velocity pair from the direction through 0x0D2D44,
 * subtracts the scroll long A5+0x4228 from +0x18 and finally draws, parking at
 * 0x0DC542. */
int Obj_DC49A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC49A: cmpi.w #0x167,(0x58,A0) ; bhi 0x0CEE74
     *           cmpi.w #0x32,(0x60,A0)  ; bhi 0x0CEE74  (unsigned) */
    if (rd16(m, obj + 0x58u) > 0x0167u || rd16(m, obj + 0x60u) > 0x0032u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0DC4B2: subi.w #0x18 on the position words */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0018u));

    /* 0x0DC4BE: lea (-0x34DE,PC),A1 -> 0x0DC4C2-0x34DE = 0x0D8FE2 ; jsr 0x0CEDE4 */
    Obj_0CEDE4(m, obj, 0x000D8FE2u);

    /* 0x0DC4C8: lea (-0x3A6,PC),A1 -> 0x0DC4CA-0x3A6 = 0x0DC124 ; jsr 0x0D0E3A */
    uint32_t a1 = 0x000DC124u;
    uint16_t r = Rng_Mix1D60(m);
    if (r > 0x0033u) {                              /* 0x0DC4D2: bls skips both */
        /* 0x0DC4DA: lea (-0x310,PC),A1 -> 0x0DC4DC-0x310 = 0x0DC1CC */
        a1 = 0x000DC1CCu;
        if (r > 0x0099u)                            /* 0x0DC4DE: bls 0x0DC4EA */
            a1 = 0x000DC238u;                       /* 0x0DC4E6: 0x0DC4E8-0x2B0 */
    }

    /* 0x0DC4EA: jsr 0x0D0678 */
    Obj_StartBehaviour(m, obj, a1);

    /* 0x0DC4F0: D0.w = rnd ; andi.w #0x3F ; subi.w #0x20 ; add.w (0x58,A0) */
    int16_t ang = (int16_t)(uint16_t)((uint16_t)(Rng_Mix1D60(m) & 0x003Fu) -
                                      0x0020u + rd16(m, obj + 0x58u));
    if (ang < 0) {
        ang = (int16_t)(uint16_t)((uint16_t)ang + 0x0168u);      /* 0x0DC516 */
    } else if (ang >= 0x0168) {                     /* 0x0DC506: blt 0x0DC51A */
        ang = (int16_t)(uint16_t)((uint16_t)ang - 0x0168u);      /* 0x0DC50E */
    }

    /* 0x0DC51A: D0.w = angle, D1.w = (0x60,A0) ; jsr 0x0D2D44
     * 0x0D2D44 reads its two long tables at 0xD2D4A+0x182 = 0x0D2ECC
     * (D1 source) and 0xD2D52+0x12 = 0x0D2D64 (D0 source), indexes them by
     * D0.w*4 and dispatches through 0x0D4F26[D1.w] (51 entries, 0x0D4FF2..
     * 0x0D538A) on the constant 0..50 -- i.e. both longs are multiplied by
     * D1.w. Nothing beyond the entry is reachable, so the multiply is inlined;
     * D1.w == 0 zeroes both outputs (table word 0 of 0xD4FF4). */
    uint16_t d1w = rd16(m, obj + 0x60u);
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)ang * 4u) * (uint32_t)d1w;
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)ang * 4u) * (uint32_t)d1w;
    wr32(m, obj + 0x50u, d0);                       /* 0x0DC524 */
    wr32(m, obj + 0x54u, d1);                       /* 0x0DC528 */

    /* 0x0DC52C: move.l (0x4228,A5),D0 ; sub.l D0,(0x18,A0) */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    /* 0x0DC534: jsr 0x001282 ; 0x0DC53A: move.l #0xDC542,(A0) */
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000DC542u);
    return 0;
}
