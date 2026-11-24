#include "obj_EDC38.h"
#include "obj_EE0A4.h"

/* ROM 0x0EDC38: formation enemy that fires 0xD9092 at the players.
 * Same code at 0xEE0A4, 0xEE468, 0xEE7D8 — see obj_EE0A4.c. */
int Obj_EDC38_Tick(const f3mem_t *m, uint32_t obj)
{
    return Enemy_FormationShooter(m, obj, 0x000EDEC2u, 0x000EDF60u);
}
