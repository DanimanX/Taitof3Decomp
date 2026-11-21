/* Darius Gaiden (Taito F3) — stage timeline step and timed-effect starters.
 * Decompiled from ROM 0x0015D0..0x00161C and 0x13F2D4..0x13F340. */
#include "stage_timeline.h"
#include "../objects/obj_pool.h"

static void fx_start(const f3mem_t *m, uint32_t table, uint16_t mode, uint8_t id, uint8_t n)
{
    wr16(m, FX_MODE, mode);
    uint16_t idx = (uint16_t)(id % n);                 /* divu.w + swap */
    int32_t dur = (int32_t)rd32(m, table + 4u + 8u * idx);
    if (dur > 0) {
        wr16(m, FX_TIMER, (uint16_t)(dur + 1));
        wr32(m, FX_PTR, rd32(m, table + 8u * idx));
    }
}

void Fx_StartA(const f3mem_t *m, uint8_t id) { fx_start(m, 0x16890u, 0, id, 6); }
void Fx_StartB(const f3mem_t *m, uint8_t id) { fx_start(m, 0x168C0u, 1, id, 8); }

int StageTimeline_Step(const f3mem_t *m, int *spawned)
{
    if (!rd8(m, TL_FX_STARTED) && (int16_t)rd16(m, FX_TIMER) <= 0) {
        wr8(m, TL_FX_STARTED, 0xFF);
        Fx_StartA(m, 1);
    }
    wr32(m, TL_SCROLL_POS, rd32(m, TL_SCROLL_POS) + rd32(m, TL_SCROLL_SPD));
    uint16_t last = rd16(m, TL_LAST_POS);
    wr16(m, TL_LAST_POS, rd16(m, TL_SCROLL_POS));
    if (rd16(m, TL_SCROLL_POS) == last)
        return 0;
    uint32_t a0 = rd32(m, TL_SCRIPT_PTR);
    for (;;) {
        uint16_t pos = rd16(m, a0);
        a0 += 2;
        if (pos != rd16(m, TL_SCROLL_POS))
            return 0;
        if ((int16_t)rd16(m, a0 + 4) < 0)
            return 1;
        Obj_Spawn(m, a0, WAVE_OBJ_FN, 5, 2, a0);
        if (spawned) (*spawned)++;
        a0 += 6;
        wr32(m, TL_SCRIPT_PTR, a0);
    }
}
