#include "obj_1261AA.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

/* ROM 0x11B262 (jsr at 0x1261B4): move.l A5+0x30D2 -> +0x14, A5+0x30D6 ->
 * +0x18, then 0x0CBE12 with D0w = (0x20,A0) >> 8: D1 = D2 = (0x42,A0).w*
 * { cos = s16[0x27E40 + 0x80 + 2*D0], sin = s16[0x27E40 + 2*D0] }, both
 * added as 32-bit (muls.w + add.l), then A5+0x30D2/6 = +0x14/+0x18. */
static void sub_11B262(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint16_t d1 = rd16(m, obj + 0x42u);
    int16_t cs = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(uint16_t)(d0 << 1));
    int16_t sn = (int16_t)rd16(m, 0x027E40u + (uint32_t)(uint16_t)(d0 << 1));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)((int32_t)(int16_t)d1 * cs));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)((int32_t)(int16_t)d1 * sn));
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

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

/* 0x1261AA never enters the 0x179A5C frame-script engine (bundle_1261AA.s has
 * no lea 0x179A5C / jsr (A1)), so no op-table helper is needed here. */
extern int Obj_179550_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x179550 */
extern int Obj_179572_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x179572 */


/* Frame-script helpers left unused here (0x1261AA never runs the script
 * engine: it only follows the leader, counts +0x56 down and jumps on). */

/* ROM 0x1261AA: follow the leader (0x11B262) while the countdown at +0x56
 * ticks down by 0xC0 per frame; on expiry clear A5+0x30F3, play sound 0x13 and
 * run the setup tail (0x1261DE). Positive counts re-arm through 0x1261A2. */
int Obj_1261AA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1261AA: D0w = (0x56,A0) >> 8 -> (0x42,A0) */
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x56u) >> 8));
    sub_11B262(m, obj);                                /* 0x1261B4 jsr 0x11B262 */

    /* 0x1261BA: subi.w #0xC0,(0x56,A0); bgt 0x1261A2 -- the branch tests
     * the NEW value, so frame 0 already takes the expiry path */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x56u) - 0x00C0u);
    wr16(m, obj + 0x56u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001261AAu);             /* 0x1261A2 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                       /* 0x1261C2: sf */
    play_sound(m, 0x13);                               /* 0x1261C6 */
    wr16(m, obj + 0x56u, 0u);                          /* 0x1261DA: clr.w */

    /* 0x1261DE (falls through after the expiry path) */
    wr16(m, obj + 0x50u, 0x0130u);
    wr16(m, obj + 0x52u, 0x0060u);
    wr16(m, obj + 0x06u, 0x0028u);
    wr8(m, obj + 0x54u, 0x05u);
    wr32(m, obj + 0x00u, 0x001261FEu);                 /* 0x1261F6 */
    return 0;
}
