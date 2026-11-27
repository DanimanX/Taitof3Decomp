/* ROM 0x1635AE: draw the compressed tilemap 0x6B5FA into 0x611000, clear
 * A5+0x4210 and A5-0x5D9C, set the delay word +0x06 = 2 and hand the object to
 * 0x1635E8. The four `move.w #2,(0x6,A0) ; beq` pairs further down are dead
 * for this entry: the moved constant is 2, so Z = 0 and every beq is skipped. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_1635AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* ROM 0x1635AE..0x1635CC: movem.l {A0},-(SP) ; move.w #0,-(SP) ;
     * pea 0x611000 ; move.l #0x6B5FA,-(SP) ; jsr 0x53B2 ; lea (0xA,SP),SP */
    Gfx_DrawCompressedTilemap(m, 0x0006B5FAu, 0x00611000u, 0u);
    wr32(m, F3_A5 + 0x4210u, 0u);       /* ROM 0x1635D0: clr.l (0x4210,A5) */
    wr32(m, F3_A5 - 0x5D9Cu, 0u);       /* ROM 0x1635D4: clr.l (-0x5d9c,A5) */
    wr16(m, obj + 0x06u, 0x0002u);      /* ROM 0x1635D8: move.w #2,(0x6,A0) -> Z=0 */
    wr32(m, obj, 0x001635E8u);          /* ROM 0x1635E0: move.l #0x1635e8,(A0) */
    return 0;                           /* ROM 0x1635E6 rts */
}
