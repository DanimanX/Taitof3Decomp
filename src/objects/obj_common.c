/* Darius Gaiden (Taito F3) — small helpers shared by object tick routines. */
#include "obj_common.h"

/* ROM 0x0CEE74 */
void Obj_StartDying(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x000CEE7Cu);
}

/* ROM 0x0CF85A */
void Obj_FlagOffscreen(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if (!(flags & 0x10u))
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (x < -0x40 || x > 0x17F || y < -0x40 || y > 0x127)
        wr8(m, obj + 0x07u, (uint8_t)(flags | 0x08u));
}

/* ROM 0x0D0DCC */
void Coll_AddHazard(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, F3_A5 + 0x906u);
    wr32(m, p, obj);
    wr32(m, F3_A5 + 0x906u, p + 4u);
}

/* ROM 0x0CBD52 */
void Obj_Animate(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl)
{
    for (;;) {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        uint32_t entry = anim_tbl + (uint32_t)(int32_t)off;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0);
            continue;
        }
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0);
        wr16(m, obj + 0x3Au, (uint16_t)((uint16_t)off + 6u));
    }
}
