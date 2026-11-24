#include "obj_1289EA.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x1289EA: spawn a 0x128B44 bullet through groups 5..6 with arg 0;
 * the child's script slot (+0x4C) is zeroed and this object gets a 200
 * frame lifetime, a 0x08 x 0x01 hitbox, the parent position plus the
 * {dx.b, dy.b} byte record in obj+0xC, and spin speed 0x68.
 *   1289EA  pea (0x0).w ; move.w #2,-(SP) ; move.w #5,-(SP)
 *   1289F6  move.l #0x128B44,-(SP) ; jsr 0x01057C ; lea (0xC,SP),SP
 *   128A06  (0x48,A0) = D0 ; ble.s 0x128D4A
 *   128A0E  A1 = (0x48,A0) ; clr.w (0x4C,A1) ; (0x4C,A0) = 0xC8
 *   128A1C  jsr 0x0CBD86 (Obj_ResetFields)
 *   128A22  (0x2C,A0)=6 (0x30,A0)=3 (0x2E,A0)=0x1B (0x32,A0)=0xE
 *   128A3A  (0x28,A0)=1 (0x24,A0)=8 ; clr.b (0x4F,A0)
 *   128A4A  A1 = (0x8,A0) ; (A1+0x14) -> (A0+0x14) ; (A1+0x18) -> (A0+0x18)
 *   128A5A  bfexts (0xC,A0){0:8} -> +0x14.w ; bfexts {8:8} -> +0x18.w
 *   128A6E  (0x20,A0)=0x68 ; clr.w (0x42,A0) ; clr.l (0x36,A0)
 *   128A7C  fn = 0x128A84
 *   128D4A  Obj_Free (spawn failed) */
int Obj_1289EA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = Obj_Spawn(m, obj, 0x00128B44u, 5u, 2u, 0u);
    wr32(m, obj + 0x48u, d0);
    if ((int32_t)d0 <= 0) {              /* ble.w 0x128D4A */
        Obj_Free(m, obj);
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x48u);
    wr16(m, a1 + 0x4Cu, 0u);
    wr16(m, obj + 0x4Cu, 0x00C8u);

    Obj_ResetFields(m, obj);             /* ROM 0x0CBD86 */

    wr16(m, obj + 0x2Cu, 6u);
    wr16(m, obj + 0x30u, 3u);
    wr16(m, obj + 0x2Eu, 0x001Bu);
    wr16(m, obj + 0x32u, 0x000Eu);
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x24u, 8u);
    wr8(m, obj + 0x4Fu, 0u);

    a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    uint32_t rec = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)(int8_t)(rec >> 24);   /* bfexts {0:8} */
    int16_t dy = (int16_t)(int8_t)(rec >> 16);   /* bfexts {8:8} */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));

    wr16(m, obj + 0x20u, 0x0068u);
    wr16(m, obj + 0x42u, 0u);
    wr32(m, obj + 0x36u, 0u);
    wr32(m, obj + 0x00u, 0x00128A84u);
    return 0;
}
