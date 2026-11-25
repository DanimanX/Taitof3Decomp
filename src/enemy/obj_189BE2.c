#include "obj_189BE2.h"
#include "../objects/obj_pool.h"

/* ROM 0x189BE2: 0x189C1E's head. A one-frame hold, then +0x22 is forced to 8
 * and the 0x189BEE colour cycle starts: a 0x18B230 speaker spawn per step,
 * 2 frames apart, then 0x18A572 (silence) and the 0x28-frame tail 0x189C56. */

/* ROM 0x189BDA: re-arm the same tick (move.l #0x189BE2,(A0)). */
static void arm_self(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00189BE2u);
}

/* ROM 0x189C16: arm 0x189C1E (move.l #0x189C1E,(A0)). */
static void arm_189C1E(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00189C1Eu);
}

/* ROM 0x189BE2 */
int Obj_189BE2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189BE2: subq.w #1,(A0+6) ; bgt 0x189BDA */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        arm_self(m, obj);
        return 0;
    }

    /* 0x189BE8: move.b #8,(A0+0x22) */
    wr8(m, obj + 0x22u, 0x08u);

    /* 0x189BEE: bfextu (A0+0x22){0:8} -> D0, spawn 0x18B230 in groups 3..4 */
    Obj_Spawn(m, obj, 0x0018B230u, 3u, 2u, (uint32_t)rd8(m, obj + 0x22u));

    /* 0x189C0E: move.w #2,(A0+6) ; move.l #0x189C1E,(A0) */
    wr16(m, obj + 0x06u, 0x0002u);
    arm_189C1E(m, obj);
    return 0;
}

/* The rest of the 0x189BEE block -- the `subq.b #1,(A0+0x22)` step and its
 * 0x18A572 / 0x4B74 / 0x189A8E tail -- is only reachable from 0x189C1E once
 * the countdown expires, and the beq at 0x189C14 tests the Z of
 * `move.w #2,(A0+6)`, which cannot be set. Neither path belongs to this tick:
 * this entry arm always ends in the 0x189C1E re-arm. */
