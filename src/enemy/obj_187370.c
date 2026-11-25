/* ROM 0x187370: spawner record. The spawn argument at +0xC is the packed
 * {dx.b, dy.b, 0, speed.b} offset pair; the child is placed at the parent's
 * +0x50/+0x54 plus that offset, its own speed at +0x58 (byte, zero extended),
 * and then it is pointed at the frame list 0x18743C through the 0x179A5C INIT
 * half. +0x1C and +0x42 are armed, and the object switches to 0x1873D6. */
#include "obj_187370.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "obj_179550.h"
#include "obj_179572.h"
#include "obj_1795A4.h"
#include "obj_1795F6.h"
#include "obj_179600.h"
#include "obj_17960A.h"
#include "obj_179614.h"
#include "obj_179632.h"
#include "obj_179646.h"

/* INIT half of the 0x179A5C table, (0x179A5C,D0w*8) -- 0x1873BC. */
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

int Obj_187370_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x187370: movea.l (0x8,A0),A4 */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);

        /* 0x187374: bfexts (0xC,A0){0:8},D0 ; 0x18737A: {8:8},D1 ;
         * 0x187380: bfextu {24:8},D2 */
        int16_t dx = (int16_t)(int8_t)rd8(m, obj + 0x0Cu);
        int16_t dy = (int16_t)(int8_t)rd8(m, obj + 0x0Du);
        uint32_t speed = (uint32_t)rd8(m, obj + 0x0Fu);

        /* 0x187386: add.w (0x50,A4),D0 ; add.w (0x54,A4),D1 */
        uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + (uint16_t)dx);
        uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + (uint16_t)dy);
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr32(m, obj + 0x58u, speed);            /* 0x187396: move.l D2,(0x58,A0) */
        wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));   /* 0x18739A */
        wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));   /* 0x1873A0 */

        /* 0x1873A6: A1 = 0x18743C ; move.w (A1)+,D0 ; move.w D0,(0x40,A0) ;
         * move.l A1,(0x3A,A0) */
        uint16_t idx = rd16(m, 0x0018743Cu);
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x0018743Eu);

        /* 0x1873B6: lea 0x179A5C,A1 ; movea.l (A1,D0w*8),A1 ; jsr (A1) */
        if (anim_init(m, obj, idx) < 0)
            return -1;
    }

    wr16(m, obj + 0x1Cu, 0x0092u);              /* 0x1873C2 */
    wr16(m, obj + 0x42u, 0xFD00u);              /* 0x1873C8 */
    wr32(m, obj, 0x001873D6u);                  /* 0x1873CE */
    return 0;
}
