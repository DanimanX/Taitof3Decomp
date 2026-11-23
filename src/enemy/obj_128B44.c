#include "obj_128B44.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBE12 (jumped to at 0x128B82 with D0w = rnd & 0xFF):
 *   lea 0x27E40,A3 ; D1 = D2 = (0x42,A0) ; MULS.W (0x80,A3,D0.W*2),D1 ;
 *   ADD.L D1,(0x14,A0) ; MULS.W (A3,D0.W*2),D2 ; ADD.L D2,(0x18,A0)
 * (16-bit products; re-expressed here because Obj_MoveByAngle's local
 * copy uses obj+0x42 instead of (0x42,A0).) */
static void sub_0CBE12(const f3mem_t *m, uint32_t obj, uint16_t angle)
{
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int16_t)angle * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int16_t)angle * 2u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)cos_v));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)sin_v));
}

/* ROM 0x128B44: initialise a bullet that fades in with the scroll
 *   128B44  clr.w (0x1c,A0) / (0x1e,A0) / (0x34,A0) ; clr.l (0x48,A0)
 *   128B54  move.l #0xc80000,(0x14,A0) ; #0x780000,(0x18,A0)
 *   128B64  jsr 0x1d8a ; andi.w #0xff,D0w ; addi.w #0xa0,D0w -> (0x42,A0)
 *   128B76  jsr 0x1d8a ; andi.w #0xff,D0w ; jsr 0xcbe12
 *   128B86  (0x14,A0).w -> (0x50,A0) ; (0x18,A0).w -> (0x52,A0)
 *   128B92  clr.l (0x36,A0)
 *   128B96  move.l #0x128b9e,(A0) */
int Obj_128B44_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x48u, 0u);
    wr32(m, obj + 0x14u, 0x00C80000u);
    wr32(m, obj + 0x18u, 0x00780000u);

    /* 0x128B64 */
    uint16_t d0 = (uint16_t)(Rng_NextMem(m) & 0x00FFu);
    d0 = (uint16_t)(d0 + 0x00A0u);
    wr16(m, obj + 0x42u, d0);

    /* 0x128B76 */
    d0 = (uint16_t)(Rng_NextMem(m) & 0x00FFu);
    sub_0CBE12(m, obj, d0);                      /* ROM 0x0CBE12 */

    /* 0x128B86 */
    wr16(m, obj + 0x50u, rd16(m, obj + 0x14u));
    wr16(m, obj + 0x52u, rd16(m, obj + 0x18u));
    wr32(m, obj + 0x36u, 0u);

    /* 0x128B96 */
    wr32(m, obj + 0x00u, 0x00128B9Eu);
    return 0;
}
