#include "obj_FF1DA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "obj_E8B72.h" /* Obj_SetScript (0x0CF1D8), Obj_SetAnim (0x0D0678) */

/* ROM 0x0FF1DA: setup tick of the 0xFF232 object. It takes position and
 * velocity from its parent record at +0x08 (y shifted by +0xD, vy by -1),
 * seeds the parameter records 0xFF326 (0x0CEDE4), 0xFF346 (0x0CF1D8) and the
 * behaviour record 0xFF376 (0x0D0678), draws once, then parks in 0xFF232. */

int Obj_FF1DA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0FF1DA: movea.l (0x8,A0),A1 ; move.l (0x14,A1),(0x14,A0) ;
     * addi.w #0xD,(0x14,A0) -- long copy, then the high half (y) += 0xD */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Du));

    /* 0x0FF1EA: move.l (0x18,A1),(0x18,A0) ; subq.w #1,(0x18,A0) */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 1u));

    /* 0x0FF1F4: lea (0x130,PC),A1 -> 0xFF326 ; jsr 0x0CEDE4 */
    Obj_0CEDE4(m, obj, 0x00FF326u);

    /* 0x0FF1FE: lea (0x146,PC),A1 -> 0xFF346 ; jsr 0x0CF1D8 */
    Obj_SetScript(m, obj, 0x00FF346u);

    /* 0x0FF208: lea (0x16C,PC),A1 -> 0xFF376 ; jsr 0x0D0678 */
    Obj_SetAnim(m, obj, 0x00FF376u);

    /* 0x0FF212: bset.b #4,(0x7,A0) -- 68k bit numbering counts bits from the
     * LSB of the byte, so bit 4 of the byte at +0x07 is 0x10 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));

    /* 0x0FF218: bclr.b #1,(0x34,A0) ; 0x0FF21E: bset.b #2,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));

    /* 0x0FF224: jsr 0x001282 */
    Obj_DrawSprite(m, obj);

    /* 0x0FF22A: move.l #0xFF232,(A0) ; 0x0FF230: rts */
    wr32(m, obj + 0x00u, 0x000FF232u);
    return 0;
}
