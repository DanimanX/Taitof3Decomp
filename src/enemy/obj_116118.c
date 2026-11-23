#include "obj_116118.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"
#include "../enemy/obj_112C3C.h"

/* Uses the shared helpers Obj_ResetFields (0x0CBD86), Sound_PlayCmd (0x39D8,
 * via Sound_PlayHooked) and Obj_113400_FollowParent (0x113400). */

/* ROM 0x116118 */
int Obj_116118_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x116118: relink the child list of the template at +0x8 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x08u, rd32(m, a1 + 0x08u));
    Obj_ResetFields(m, obj);

    /* 0x116128: mirror the template flag bit3 into the angle / direction */
    a1 = rd32(m, obj + 0x08u);
    if ((rd8(m, a1 + 0x50u) & 0x08u) == 0u) {
        wr16(m, obj + 0x2Cu, 0x0004u);
        wr16(m, obj + 0x2Eu, 0x0018u);
        wr16(m, obj + 0x20u, 0x8000u);      /* move.w #-0x8000,(0x20,A0) */
        wr16(m, obj + 0x22u, 0x0000u);
    } else {
        /* 0x116152 */
        wr16(m, obj + 0x2Cu, 0x0037u);
        wr16(m, obj + 0x2Eu, 0x004Bu);
        wr16(m, obj + 0x20u, 0x0000u);
        wr16(m, obj + 0x22u, 0xFFFFu);      /* move.w #-0x1,(0x22,A0) */
    }

    /* 0x11616A */
    wr16(m, obj + 0x30u, 0x0004u);
    wr16(m, obj + 0x32u, 0x001Bu);
    wr16(m, obj + 0x42u, 0x001Eu);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x24u, 0x0064u);
    wr32(m, obj + 0x10u, 0x0005BECCu);
    wr16(m, obj + 0x52u, 0xFFA4u);          /* move.w #-0x5c,(0x52,A0) */
    wr16(m, obj + 0x54u, 0xFFF8u);          /* move.w #-0x8,(0x54,A0) */

    /* 0x11619C */
    Obj_113400_FollowParent(m, obj);
    wr8(m, obj + 0x1Du, 0x0Eu);

    /* 0x1161A8: move.w #0x2a,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 */
    Sound_PlayHooked(m, 0x002Au);

    /* 0x1161BC */
    wr32(m, obj + 0x00u, 0x001161C4u);
    return 0;
}
