#include "obj_126772.h"
#include "obj_11B288.h"
#include "../audio/sound_cmd.h"

/* ROM 0x126772: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader. On the
 * +0x06 countdown the tail clears bit A5+0x30EE{0xB} and branches on
 * A5+0x30DA > 0x3E8 to 0x12679A (out of this file) or 0x12695C, which
 * clears A5+0x30EE, resets +0x54/+0x06, plays sound 0x13 and re-targets
 * +0x50/+0x52 before handing over to 0x1269C4. */
int Obj_126772_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126772 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126778 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12677E jsr 0x11B262 */

    /* 0x126784: subq.w #1 -> bgt 0x12676A tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126772u);         /* 0x12676A */
        return 0;
    }
    /* 0x12678A: bfclr (0x30EE,A5){0xB:1} -> bit 4 of the long word +0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Bu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x0Bu % 8u))));
    }
    /* 0x126790: cmpi.w #0x3E8,(0x30DA,A5); ble 0x12695C */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x03E8) {
        /* 0x12679A: next state at 0x1267BC (out of this routine's scope) */
        wr8(m, obj + 0x54u, 0x08u);
        wr16(m, obj + 0x50u, 0x0100u);
        wr16(m, obj + 0x52u, 0x0040u);
        wr16(m, obj + 0x06u, 0x0078u);
        wr32(m, obj + 0x00u, 0x001267BCu);         /* 0x1267B4 */
        return 0;
    }
    /* 0x12695C */
    wr32(m, F3_A5 + 0x30EEu, 0u);                  /* clr.l */
    wr8(m, obj + 0x54u, 0x00u);                    /* 0x126960 */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x126966 */
    wr32(m, obj + 0x00u, 0x00126976u);             /* 0x12696E */
    /* 0x12698E: move.w #0x13,-(SP); jsr 0x13F238; jsr 0x39D8 */
    {
        int16_t cmd = 0x13;
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
    /* 0x1269A2: next state 0x1269C4 with +0x50 = 0xC0, +0x52 = 0x30 */
    wr8(m, obj + 0x54u, 0x08u);
    wr16(m, obj + 0x50u, 0x00C0u);
    wr16(m, obj + 0x52u, 0x0030u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x001269C4u);             /* 0x1269BC */
    return 0;
}
