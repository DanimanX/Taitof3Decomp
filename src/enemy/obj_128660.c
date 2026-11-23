#include "obj_128660.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 + 0x39D8: sound command sent from a word on the stack; the
 * pre-cmd hook shifts 1..0x40 by +0x53 when A5+0x3306 is set. */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x128660: a wobbling sprite that follows its parent (0x8,A0) for
 * A0+0x6 frames in steps of 2, then plays sound 0x25 and jumps to 0x1286E2.
 * When the parent's 32-bit HP (0x24,A1) is <= 0 it frees itself (0x1287A4 ->
 * 0x105EC). The offsets at (0x10,A0) are scaled by (D0w - 0x100) with
 * muls.w/asr.w #5, so the object breathes with the countdown in +0x6. */
int Obj_128660_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    /* 0x128664: tst.l (0x24,A1); ble.w 0x1287A4 */
    if ((int32_t)rd32(m, a1 + 0x24u) <= 0) {
        Obj_Free(m, obj);                            /* ROM 0x1287A4 -> 0x0105EC */
        return 0;
    }

    /* 0x12866C: D0w = (0x6,A0); bytes +0x1F/+0x1E = D0b */
    uint16_t d0 = rd16(m, obj + 0x06u);
    wr8(m, obj + 0x1Fu, (uint8_t)d0);
    wr8(m, obj + 0x1Eu, (uint8_t)d0);

    /* 0x128678: subi.w #0x100; D1 = D0; muls.w (A1)+,D1 ; asr.w #5 */
    d0 = (uint16_t)(d0 - 0x0100u);
    uint16_t d1 = d0;
    a1 = rd32(m, obj + 0x10u);
    int16_t m0 = (int16_t)rd16(m, a1);                /* (A1)+ */
    int16_t m1 = (int16_t)rd16(m, a1 + 2u);           /* (A1)+ */
    d1 = (uint16_t)(((int32_t)(int16_t)d1 * (int32_t)m0) >> 5);
    d0 = (uint16_t)(((int32_t)(int16_t)d0 * (int32_t)m1) >> 5);

    /* 0x12868A: X = obj + parent(+0x14), Y = obj + parent(+0x18) */
    a1 = rd32(m, obj + 0x08u);
    d0 = (uint16_t)(rd16(m, obj + 0x50u) + d0);
    d0 = (uint16_t)(rd16(m, a1 + 0x14u) + d0);
    wr16(m, obj + 0x14u, d0);
    d1 = (uint16_t)(rd16(m, obj + 0x52u) + d1);
    d1 = (uint16_t)(rd16(m, a1 + 0x18u) + d1);
    wr16(m, obj + 0x18u, d1);

    /* 0x1286A6: cmp.w (0x22,A0),D0w with D0w = (A5+0x878) & 1; bne skips
     * the draw */
    if ((rd16(m, F3_A5 + 0x0878u) & 1u) == rd16(m, obj + 0x22u))
        Obj_DrawSprite(m, obj);                       /* ROM 0x001282 */

    /* 0x1286BC: subq.w #2,(0x6,A0); bgt 0x128658 (re-arm self) */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 2u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00128660u);            /* 0x128658 */
        return 0;
    }

    play_sound(m, 0x25);                              /* 0x1286C2 */
    wr16(m, obj + 0x1Eu, 0u);                         /* 0x1286D6: clr.w */
    wr32(m, obj + 0x00u, 0x001286E2u);                /* 0x1286DA */
    return 0;
}
