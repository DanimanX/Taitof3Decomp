#include "obj_189C1E.h"
#include "../objects/obj_pool.h"

/* ROM 0x189C1E: one 2-frame hold, then the 8-frame colour cycle at 0x189BEE
 * driven by +0x22 through the speaker object 0x18B230; when the count runs out
 * the speaker gets the 0x18A572 command with duration 0 (silence) and the
 * 0x28-frame tail arms 0x189C56. Siblings: 0x189CCC / 0x189E2E. */

/* ROM 0x189C16: re-arm the same tick (move.l #0x189C1E,(A0)). */
static void arm_self(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00189C1Eu);
}

/* ROM 0x189C1E */
int Obj_189C1E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189C1E: subq.w #1,(A0+6) ; bgt 0x189C16 */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        arm_self(m, obj);
        return 0;
    }

    /* 0x189C24: subq.b #1,(A0+0x22) ; bgt 0x189BEE */
    uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
    wr8(m, obj + 0x22u, c);
    if ((int8_t)c > 0) {
        /* 0x189BEE: bfextu (A0+0x22){0:8} -> D0, spawn 0x18B230 into groups
         * 3..4 with arg = that byte, then hold 2 frames at 0x189C1E. */
        Obj_Spawn(m, obj, 0x0018B230u, 3u, 2u, (uint32_t)rd8(m, obj + 0x22u));
        wr16(m, obj + 0x06u, 0x0002u);
        arm_self(m, obj);
        return 0;
    }

    /* 0x189C2A: pea (0x0).w -- silence -- for the speaker in groups 3..4 */
    Obj_Spawn(m, obj, 0x0018A572u, 3u, 2u, 0x00000000u);

    /* 0x189C46: move.w #0x28,(A0+6) ; move.l #0x189C56,(A0) */
    wr16(m, obj + 0x06u, 0x0028u);
    wr32(m, obj, 0x00189C56u);
    return 0;
}

/* The beq at 0x189C4C (target 0x189C5C) tests the Z left by
 * `move.w #0x28,(A0+6)`, which can never be set, so that tail -- the
 * A5+0x4B74 (-0x20,-0x30) write plus the 0x189C7C / 0x189A8E states -- is
 * dead code and is not translated here. */
