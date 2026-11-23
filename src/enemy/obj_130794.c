#include "obj_130794.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../kernel/rng.h"

/* ROM 0x130794: one-frame death puff. It randomises the sprite pick and the
 * life counter, installs one of four sprite records from the 0x130828 table,
 * then switches to the follow-up state 0x1307E0. */
int Obj_130794_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                      /* 0x130794 jsr 0x0CBD86 */
    wr8(m, obj + 0x1Du, 0x7Eu);                   /* 0x13079A move.b #0x7E,(0x1D,A0) */

    /* 0x1307A0: jsr 0x1D8A ; andi.w #0x1F,D0w ; addi.w #0xF0,D0w */
    wr16(m, obj + 0x42u, (uint16_t)((uint16_t)(Rng_NextMem(m) & 0x1Fu) + 0x00F0u));

    /* 0x1307B2: jsr 0x1D8A ; andi.w #0x1F,D0w ; addi.b #0x7A,D0w
     * addi.b is a BYTE add: only the low byte of D0w changes. */
    {
        uint16_t d0 = (uint16_t)(Rng_NextMem(m) & 0x1Fu);
        d0 = (uint16_t)((d0 & 0xFF00u) | ((uint8_t)((uint8_t)d0 + 0x7Au)));
        wr16(m, obj + 0x20u, d0);                 /* 0x1307C0 */
    }

    /* 0x1307C4: move.b (A5-0x1456),D0b ; andi.w #0x3,D0w ;
     *          lea (0x5A,PC),A1 (= 0x130828) ; move.l (A1,D0w*4),(0x10,A0) */
    {
        uint16_t d0 = (uint16_t)(rd8(m, F3_A5 - 0x1456u) & 0x03u);
        wr32(m, obj + 0x10u, rd32(m, 0x00130828u + (uint32_t)d0 * 4u));
    }

    wr32(m, obj + 0x00u, 0x001307E0u);            /* 0x1307D8 */
    return 0;
}
