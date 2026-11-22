/* Darius Gaiden (Taito F3) — small helpers shared by object tick routines. */
#include "obj_common.h"
#include "obj_pool.h"

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

/* ROM 0x0D0690: advance the object script (obj+0x48) once its frame timer
 * (obj+0x4C) runs out. Copied from the five sibling tick routines that carry
 * this ROM range as a private static helper. */
void Obj_StepScript(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0CEE7C / 0x0CEE84 / 0x0CEE8C: the dying chain started by
 * Obj_StartDying waits two frames, then frees the object. */
int Obj_Dying1_Tick(const f3mem_t *m, uint32_t obj) { wr32(m, obj, 0x000CEE84u); return 0; }
int Obj_Dying2_Tick(const f3mem_t *m, uint32_t obj) { wr32(m, obj, 0x000CEE8Cu); return 0; }
int Obj_Dying3_Tick(const f3mem_t *m, uint32_t obj) { Obj_Free(m, obj); return 0; }
