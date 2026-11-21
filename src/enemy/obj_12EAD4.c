#include "obj_12EAD4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"

/* ROM 0x11C35E: keep the colour byte in sync with obj+0x4F; the first time it
 * changes, play sound 0x1C once and force colour 0x10. */
static void sub_11C35E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        goto plain;
    if (rd8(m, obj + 0x1Du) == 0x10u)
        goto plain;
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x1Du, 0x10u);
    {
        int16_t cmd = 0x1C;
        if (rd8(m, F3_A5 + 0x3306u) != 0u)
            cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
    }
    return;
plain:
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
}

/* ROM 0x12EAD4: follow the sprite sequence at obj+0x48, publishing its screen
 * position into the global A5+0x30D2/+0x30D6; frees itself when the global
 * A5+0x30F2 bit 1/2 flags are set. */
int Obj_12EAD4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint8_t f = rd8(m, F3_A5 + 0x30F2u);
    if ((f & 0x02u) || (f & 0x04u)) {
        Obj_Free(m, obj);
        return 0;
    }
    sub_11C35E(m, obj);
    uint32_t a1 = rd32(m, obj + 0x48u);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, a1 + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, a1 + 0x18u));
    wr32(m, obj + 0x00u, 0x0012EAD4u);
    return 0;
}
