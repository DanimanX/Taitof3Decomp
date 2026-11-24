#include "obj_D5FFE.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* Table at A1 = 0x0D6016 + 0x112 (lea (0x112,PC) at 0x0D6014; PC = insn+2),
 * placed by 0x0CBEB0 from the difficulty (A5+0x87E): entry = {x-step.w,
 * y-step.w} = {scroll_vx/2 + 0x380, scroll_vy*2 + 0x600}. */
#define OBJ_D5FFE_TBL 0x000D6128u

/* ROM 0x0D5FFE: a drift object. While +0x35 is set it gives 3/4 of the scroll
 * velocity's X back (A1 = +0x36, word at +0x4E). The table above is stepped
 * with 0x0CBD24; a non-positive duration frees the object (0x0D609A),
 * otherwise it drops by the stage scroll, counts +0x06 down, re-queues itself
 * in the drift list (A5+0x906) while that count is positive, and draws. */
int Obj_D5FFE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0D5FFE: tst.b (0x35,A0) ; beq 0x0D6014 */
    if (rd8(m, obj + 0x35u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x36u);                         /* 0x0D6006 */
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x4Eu) >> 2);         /* 0x0D600A lsr.w #2 */
        wr16(m, a1 + 0x4Eu, (uint16_t)(rd16(m, a1 + 0x4Eu) - d0));  /* 0x0D6010 sub.w */
    }

    /* 0x0D6014: lea (0x112,PC),A1 ; jsr 0x0CBD24 ; tst.w (0x0,A1,D0w) */
    Obj_AnimFrameStep(m, obj, OBJ_D5FFE_TBL);
    int16_t off = (int16_t)rd16(m, obj + 0x3Au);
    if ((int16_t)rd16(m, OBJ_D5FFE_TBL + (uint32_t)(int32_t)off) <= 0) {
        Obj_Free(m, obj);                                           /* 0x0D609A -> 0x0105EC */
        return 0;
    }

    Obj_SubScrollVel(m, obj);                                       /* 0x0D6028 ROM 0x0CBE00 */

    /* 0x0D602E: subq.w #1,(0x6,A0) ; bgt 0x0D6040? (ble skips the queue) */
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {
        uint32_t a1 = rd32(m, F3_A5 + 0x906u);                      /* 0x0D6036 */
        wr32(m, a1, obj);                                           /* 0x0D603A move.l A0,(A1)+ */
        wr32(m, F3_A5 + 0x906u, a1 + 4u);                           /* 0x0D603C */
    }

    Obj_DrawSprite(m, obj);                                         /* 0x0D6040 ROM 0x001282 */
    wr32(m, obj + 0x00u, 0x000D5FFEu);                              /* 0x0D5FF6 move.l #0xd5ffe,(A0) */
    return 0;
}
