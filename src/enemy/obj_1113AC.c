#include "obj_1113AC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x1113AC: reset fields, copy parent's +0x18/+0x14, adjust them, then
 * spawn three children (fn 0x111432, 0x111492, 0x1114f6) with group 0xb,
 * count 1, arg = obj. */
int Obj_1113AC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1113AC: jsr Obj_ResetFields */
    Obj_ResetFields(m, obj);

    /* 0x1113B2: movea.l (0xc,A0),A2 */
    uint32_t a2 = rd32(m, obj + 0x0cu);

    /* 0x1113B6: move.l (0x18,A2),(0x18,A0) */
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
    /* 0x1113BC: subi.w #0x20,(0x18,A0) */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x20u));

    /* 0x1113C2: move.l (0x14,A2),(0x14,A0) */
    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
    /* 0x1113C8: addi.w #0x10,(0x14,A0) */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));

    /* 0x1113CE: move.l A0,-(SP) ; move.w #1,-(SP) ; move.w #0xb,-(SP) ; move.l #0x111432,-(SP) ; jsr Obj_Spawn */
    (void)Obj_Spawn(m, obj, 0x00111432u, 0x0bu, 1u, obj);

    /* 0x1113E8: move.l A0,-(SP) ; move.w #1,-(SP) ; move.w #0xb,-(SP) ; move.l #0x111492,-(SP) ; jsr Obj_Spawn */
    (void)Obj_Spawn(m, obj, 0x00111492u, 0x0bu, 1u, obj);

    /* 0x1113FE: move.l A0,-(SP) ; move.w #1,-(SP) ; move.w #0xb,-(SP) ; move.l #0x1114f6,-(SP) ; jsr Obj_Spawn */
    (void)Obj_Spawn(m, obj, 0x001114F6u, 0x0bu, 1u, obj);

    /* 0x11141C: move.l #0x111424,(A0) ; rts */
    wr32(m, obj, 0x00111424u);
    return 0;
}
