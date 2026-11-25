#include "obj_113A2C.h"
#include "../objects/obj_pool.h"

/* ROM 0x113A2C: decrements +0x06; while positive it stays in this routine.
 * At zero it sets bit 5 of the +0x50 byte, frees the object at +0x08, and
 * hands over to 0x113A4E. */
int Obj_113A2C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x113A2C: subq.w #1,(0x6,A0) ; bgt 0x113A24 */
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00113A2Cu);          /* 0x113A24 */
        return 0;
    }

    /* 0x113A32: bset.b #0x5,(0x50,A0) */
    uint8_t b = rd8(m, obj + 0x50u);
    wr8(m, obj + 0x50u, (uint8_t)(b | (1u << 5)));

    /* 0x113A38: move.l (0x8,A0),-(SP) ; jsr 0x105EC ; lea (0x4,SP),SP */
    uint32_t a4 = rd32(m, obj + 0x08u);
    (void)Obj_Free(m, a4);

    /* 0x113A46: move.l #0x113a4e,(A0) */
    wr32(m, obj + 0x00u, 0x00113A4Eu);
    return 0;
}
