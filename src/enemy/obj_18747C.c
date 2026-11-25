/* ROM 0x18747C: the sibling spawner of 0x187370. Same head -- child at the
 * parent's +0x50/+0x54 plus the packed {dx.b, dy.b, 0, speed.b} at +0xC --
 * but the frame list comes from the 0x1875AA pair selected by A5-0x1456 bit 14,
 * the landing spot (+0x5C) from 0x1794F0(0x200) + 0x200 negated, and the object
 * switches to the 0x187508 glide instead of 0x1873D6. */
#include "obj_18747C.h"
#include "obj_187246.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "obj_179550.h"
#include "obj_179572.h"
#include "obj_1795A4.h"
#include "obj_1795F6.h"
#include "obj_179600.h"
#include "obj_17960A.h"
#include "obj_179614.h"
#include "obj_179632.h"
#include "obj_179646.h"

/* INIT half of 0x179A5C, (0x179A5C,D0w*8). */
static int anim_init(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  return Obj_179550_Tick(m, obj);
    case 1u:  return Obj_1795F6_Tick(m, obj);
    case 2u:  return Obj_17960A_Tick(m, obj);
    case 3u:  return Obj_179572_Tick(m, obj);
    case 5u:  return Obj_1795A4_Tick(m, obj);
    case 6u:  return Obj_179632_Tick(m, obj);
    case 7u:  return Obj_179646_Tick(m, obj);
    case 9u:  return Obj_179600_Tick(m, obj);
    case 10u: return Obj_179614_Tick(m, obj);
    default:  return Obj_AnimInit(m, obj, idx);
    }
}

int Obj_18747C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18747C: movea.l (0x8,A0),A4 */
    uint32_t a4 = rd32(m, obj + 0x08u);

    /* 0x187480: bfexts (0xC,A0){0:8},D0 ; 0x187486: {8:8},D1 ;
     * 0x18748C: bfextu {24:8},D2 */
    int16_t dx = (int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    int16_t dy = (int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint32_t speed = (uint32_t)rd8(m, obj + 0x0Fu);

    /* 0x187492: add.w (0x50,A4),D0 ; add.w (0x54,A4),D1 */
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + (uint16_t)dx);
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + (uint16_t)dy);
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr32(m, obj + 0x58u, speed);                 /* 0x1874A2: move.l D2,(0x58,A0) */
    wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));   /* 0x1874A6 */
    wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));   /* 0x1874AC */

    /* 0x1874B2: bfextu (A5-0x1456){14:1},D0 ; lea 0x1875AA(PC),A1 ;
     * 0x1874BE: movea.l (A1,D0w*4),A1 ; 0x1874C4: move.w (A1)+,D0 ;
     * move.w D0,(0x40,A0) ; move.l A1,(0x3A,A0) */
    /* {14:1} on a big-endian long starts at MSB=bit 31 and counts down, so
     * bit 14 is bit 1 of the word at A5-0x1456. */
    uint16_t list = (uint16_t)((rd16(m, F3_A5 - 0x1456u) >> 1) & 0x01u);
    uint32_t a1 = rd32(m, 0x001875AAu + (uint32_t)list * 4u);
    uint16_t idx = rd16(m, a1);
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1 + 2u);

    /* 0x1874CE: lea 0x179A5C,A1 ; movea.l (A1,D0w*8),A1 ; jsr (A1) */
    if (anim_init(m, obj, idx) < 0)
        return -1;

    wr16(m, obj + 0x1Cu, 0u);                    /* 0x1874DA: clr.w (0x1C,A0) */

    /* 0x1874DE: move.l #0x200,D0 ; jsr 0x1794F0 ; addi.w #0x200,D0 ;
     * neg.w D0 ; move.w D0,(0x5C,A0) */
    wr16(m, obj + 0x5Cu, (uint16_t)(0u - (uint16_t)(Obj_RandRange(m, 0x0200u) + 0x0200u)));

    wr16(m, obj + 0x42u, 0xFA00u);               /* 0x1874F4 */
    wr16(m, obj + 0x64u, rd16(m, obj + 0x54u));  /* 0x1874FA */
    wr32(m, obj, 0x00187508u);                   /* 0x187500 */
    return 0;
}
