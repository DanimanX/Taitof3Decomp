#include "obj_17F7CE.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"

/* Frame records at 0x17F974 ({dur.w, sprite.l, colour.w} triplets) reached
 * through +0x3A, and the 8-byte movement script 0x17F98E handed to +0x44. */

/* ROM 0x17F7CE */
int Obj_17F7CE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17F7CE: bsr 0x17BD68 -- sign-extend the offset bytes to +0x16/+0x1A,
     * then +0x14/+0x18 = parent(+0x50/+0x54) + offset */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17F7D2: lea 0x17F974,A1; move.w (A1)+,D0w; +0x40 = D0w;
     * +0x3A = A1; A1 = (0x179A5C)[D0w]; jsr (A1). The index is the record's
     * own first word (0x17F974 = {0, 3, 2, 0x9898}), so idx = 0 here. */
    uint16_t idx = rd16(m, 0x0017F974u);
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, 0x0017F976u);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;

    /* 0x17F7EE: +0x44 = 0x17F98E; +0x48 = (0x17F992).w; clr +0x4A */
    wr32(m, obj + 0x44u, 0x0017F98Eu);
    wr16(m, obj + 0x48u, rd16(m, 0x0017F992u));
    wr16(m, obj + 0x4Au, 0u);
    wr16(m, obj + 0x1Cu, 0u);           /* 0x17F802 */
    wr16(m, obj + 0x1Eu, 0u);           /* 0x17F806 */

    wr32(m, obj + 0x00u, 0x0017F812u);  /* 0x17F80A */
    return 0;
}
