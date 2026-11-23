#include "obj_126608.h"
#include "obj_11B262.h"
#include "../audio/sound_cmd.h"

/* ROM 0x126608: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader. On +0x06
 * expiry +0x54 is cleared and the state tail branches on A5+0x30DA > 0x7D0:
 *   - taken: set A5+0x30EE{0x8}, {0x9}, {0x10}, plus {0xB} when
 *     A5+0x3272{0x6} and {0xA} when A5+0x3272{0x7}; a 0x12C timer then hands
 *     over to 0x126670.
 *   - below: 0x1266A8 -- clear A5+0x30EE, +0x54 = 0, a 0x78 timer, hand over
 *     to 0x1266C2, then fall through to its 0x1266DA tail (sound 0x13 and
 *     0x1266EE: +0x54 = 8, +0x50 = +0x52 = 0x40, 0x78 timer, 0x126710). */
int Obj_126608_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126608 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12660E jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126614 jsr 0x11B262 */

    /* 0x12661A: subq.w #1 -> bgt 0x126600 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126608u);         /* 0x126600 */
        return 0;
    }

    /* 0x126620: sf (0x54,A0) */
    wr8(m, obj + 0x54u, 0x00u);

    /* 0x126624: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        /* 0x12662E: bfset (0x30EE,A5){0x8:1} */
        {
            uint32_t a = F3_A5 + 0x30EEu + 0x08u / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x08u % 8u))));
        }
        /* 0x126634: bftst (0x3272,A5){0x6:1} -> bit 1 of the byte +0x3272 */
        if ((rd8(m, F3_A5 + 0x3272u + 0x06u / 8u) & (0x80u >> (0x06u % 8u))) != 0u) {
            /* 0x12663E: bfset (0x30EE,A5){0xB:1} */
            uint32_t a = F3_A5 + 0x30EEu + 0x0Bu / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Bu % 8u))));
        }
        /* 0x126644: bfset (0x30EE,A5){0x9:1} */
        {
            uint32_t a = F3_A5 + 0x30EEu + 0x09u / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x09u % 8u))));
        }
        /* 0x12664A: bftst (0x3272,A5){0x7:1} -> bit 0 of the byte +0x3272 */
        if ((rd8(m, F3_A5 + 0x3272u + 0x07u / 8u) & (0x80u >> (0x07u % 8u))) != 0u) {
            /* 0x126654: bfset (0x30EE,A5){0xA:1} */
            uint32_t a = F3_A5 + 0x30EEu + 0x0Au / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Au % 8u))));
        }
        /* 0x12665A: bfset (0x30EE,A5){0x10:1} */
        {
            uint32_t a = F3_A5 + 0x30EEu + 0x10u / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x10u % 8u))));
        }
        wr16(m, obj + 0x06u, 0x012Cu);             /* 0x126660 */
        /* 0x126666: beq 0x126688 -- Z clear after the 0x12C store: not taken */
        wr32(m, obj + 0x00u, 0x00126670u);         /* 0x126668 */
        return 0;
    }

    /* 0x1266A8 */
    wr32(m, F3_A5 + 0x30EEu, 0u);                  /* clr.l */
    wr8(m, obj + 0x54u, 0x00u);                    /* 0x1266AC */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1266B2 */
    /* 0x1266B8: beq 0x1266DA -- Z clear after the 0x78 store: not taken */
    wr32(m, obj + 0x00u, 0x001266C2u);             /* 0x1266BA */

    /* 0x1266DA: move.w #0x13,-(SP); jsr 0x13F238; jsr 0x39D8 */
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
    /* 0x1266EE tail (Z clear after the 0x78 store: beq 0x126728 not taken) */
    wr8(m, obj + 0x54u, 0x08u);
    wr16(m, obj + 0x50u, 0x0040u);
    wr16(m, obj + 0x52u, 0x0040u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x00126710u);             /* 0x126708 */
    return 0;
}
