#include "obj_DC076.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* Jump table at 0x0D4F26: 51 entries, one per angle index, each block k at
 * 0x0D4FF2..0x0D538A returns the constant k in both D0 and D1 (k = 0 at
 * 0x0D4FF2 clears both). Dump from work/maincpu.bin confirms
 * [0..4] -> 0x0D4FF2, 0x0D4FF8, 0x0D4FFA, 0x0D5000, 0x0D500E, ... 0x0D538A. */
#define DC076_ANGLE_SCALE_MAX 50u

/* ROM 0x0D2D44: D0w = angle index, D1w = speed -- returns (D0, D1) =
 * (0x0D2D64[idx], 0x0D2ECC[idx]) * speed, both 32-bit. The tables are
 * PC-relative to the extension words 0x0D2D48 / 0x0D2D50. The angle comes
 * from 0x0D0E3A and is always 0..0xFF, so indices past 50 never occur. */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    if (d0w > DC076_ANGLE_SCALE_MAX)
        return -1;                              /* past the 51-entry table */
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);   /* (0x0D2D48,PC,D0w*4) */
    uint32_t d0 = rd32(m, 0x000D2D64u + off);   /* (0x0D2D50,PC,D0w*4) */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
    return 0;
}

/* ROM 0x0DC076: timed bullet spawner tail (also reached by 0x0DC328).
 * Off the field (+0x58 > 0x167 or +0x60 > 0x32) it hands over to the dying
 * state. Otherwise it re-centres +0x14/+0x18 by -0x18 (the shot is drawn
 * centred on its world point), installs the shared template 0x0D8FE2 via
 * 0x0CEDE4 and, weighted by the 0x0D0E3A counter (<=0x33 / <=0x99 / above),
 * starts one of the three animation sequences at 0x0DC124 / 0x0DC1CC /
 * 0x0DC238 with 0x0D0678 and stores +0x50/+0x54 = 0x0D2D44(+0x58, +0x60).
 * +0x18 loses the frame scroll (A5+0x4228) before the sprite is drawn; the
 * tick then becomes 0x0DC0F8. */
int Obj_DC076_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC076: cmpi.w #0x167,(0x58,A0) ; bhi 0x0CEE74 */
    if (rd16(m, obj + 0x58u) > 0x0167u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0DC082: cmpi.w #0x32,(0x60,A0) ; bhi 0x0CEE74 */
    if (rd16(m, obj + 0x60u) > 0x0032u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0DC08E: subi.w #0x18,(0x14,A0) ; subi.w #0x18,(0x18,A0) */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0018u));

    /* 0x0DC09A: lea (-0x30ba,PC),A1 -> 0x0DC09C - 0x30BA = 0x0D8FE2 */
    Obj_0CEDE4(m, obj, 0x000D8FE2u);              /* jsr 0x0CEDE4 */

    /* 0x0DC0A4: lea (0x7e,PC),A1 -> 0x0DC0A6 + 0x7E = 0x0DC124 */
    uint16_t d0 = Obj_Next4Way(m);
    /* 0x0DC0AE: cmpi.w #0x33,D0w ; bls 0x0DC0C6 */
    uint32_t a1 = 0x000DC124u;
    if (d0 > 0x0033u) {
        /* 0x0DC0B6: lea (0x114,PC),A1 -> 0x0DC0B8 + 0x114 = 0x0DC1CC */
        a1 = 0x000DC1CCu;
        /* 0x0DC0BA: cmpi.w #0x99,D0w ; bls 0x0DC0C6 */
        if (d0 > 0x0099u) {
            /* 0x0DC0C2: lea (0x174,PC),A1 -> 0x0DC0C4 + 0x174 = 0x0DC238 */
            a1 = 0x000DC238u;
        }
    }

    /* 0x0DC0C6: jsr 0x0D0678 -- {dur.w, defline.w, spr.l, next.l} record */
    Obj_StartBehaviour(m, obj, a1);

    /* 0x0DC0CC: D0w = (0x58,A0) ; D1w = (0x60,A0) ; jsr 0x0D2D44 */
    uint32_t vx, vy;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &vx, &vy) < 0)
        return -1;

    /* 0x0DC0DA: move.l D0,(0x50,A0) ; move.l D1,(0x54,A0) */
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);

    /* 0x0DC0E2: move.l (0x4228,A5),D0 ; sub.l D0,(0x18,A0) */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    /* 0x0DC0EA: jsr 0x001282 */
    Obj_DrawSprite(m, obj);

    /* 0x0DC0F0: move.l #0xDC0F8,(A0) -- hands over to 0x0DC0F8 */
    wr32(m, obj + 0x00u, 0x000DC0F8u);
    return 0;
}
