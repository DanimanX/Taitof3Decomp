#include "obj_17A50E.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17A50E: spawn of the 0x17A52A debris. Copies the packed offset/angle
 * word at +0x0C into the movement words -- {0:8} byte into +0x16 and {8:8}
 * into +0x1A -- then hands the slot to 0x17A52A. */
int Obj_17A50E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A50E: bfexts (0xC,A0){0:8},D0 ; bfexts (0xC,A0){8:8},D1 */
    uint32_t d = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x16u, (uint16_t)(int16_t)(int8_t)(d >> 24));
    wr16(m, obj + 0x1Au, (uint16_t)(int16_t)(int8_t)(d >> 16));
    wr32(m, obj + 0x00u, 0x0017A52Au);          /* 0x17A522 */
    return 0;
}
