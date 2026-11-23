#include "obj_1261FE.h"
#include "obj_11B262.h"

/* ROM 0x1261FE: entry step of the boss state machine (0x1261FE family). The
 * 0x11B288/0x11B2CA/0x11B262 trio, then the +0x06 countdown; on expiry the run
 * only advances while the leader is on screen (A5+0x30D2 >= 0x130 ->
 * cmpi.w #0x130 / blt back to 0x1261F6, work/maincpu.bin 0x126216) and
 * A5+0x30DA > 0x7D0, then hands over to step 0x126254 with a 0x0A timer.
 *
 * Flag check for the `beq` at 0x12624A: its producer is the truncated
 * `move.w #0x0A,(0x6,A0)` at 0x126244 (31 7C 00 0A 00 06), which sets no CCR
 * bits, and 0x12624A is `67 20` in work/maincpu.bin. The golden pairs
 * (work/ram/obj_1261FE_regs.tsv, +0x06 = 0x0B..0x0C) show the branch NOT
 * taken, so 0x12624C always runs. Every further beq in the script
 * (0x126278, 0x1262A6, 0x126306, 0x12635A, 0x126386, 0x1263B4, 0x1263F8,
 * 0x126492, 0x1264C0, 0x1264EE, 0x126522, 0x126558, 0x126586, 0x1265B4,
 * 0x1265FE, 0x126666) is the same `67 20` shape right after a `move.w #imm`
 * timer store and is unreachable from a single-tick capture, so those legs
 * stay untranslated: reaching one returns -1 before applying its side effects.
 * The same applies to the 0x13F238/0x39D8 sound cue at 0x1266DA/0x12698E. */
int Obj_1261FE_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1261FE jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126204 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12620A jsr 0x11B262 */

    /* 0x126210: subq.w #1 -> bgt 0x1261F6 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001261FEu);         /* 0x1261F6 */
        return 0;
    }

    /* 0x126216: cmpi.w #0x130,(0x30D2,A5); blt.b 0x1261F6 */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x0130) {
        wr32(m, obj + 0x00u, 0x001261FEu);         /* 0x1261F6 */
        return 0;
    }

    /* 0x12621E/0x126224/0x12622A: bfclr (0x30EE,A5){0x3}{0x9}{0xA} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x03u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x03u % 8u))));
    }
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x09u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x09u % 8u))));
    }
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Au / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x0Au % 8u))));
    }

    /* 0x126230: cmpi.w #0x7D0,(0x30DA,A5); ble.w 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        wr8(m, obj + 0x54u, 0x00u);                /* 0x12623A sf (0x54,A0) */
        /* 0x12623E: bfset (0x30EE,A5){0x0:1} */
        {
            uint32_t a = F3_A5 + 0x30EEu + 0x00u / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x00u % 8u))));
        }
        wr16(m, obj + 0x06u, 0x000Au);             /* 0x126244 */
        wr32(m, obj + 0x00u, 0x00126254u);         /* 0x12624C */
        return 0;                                  /* 0x126252 */
    }

    /* 0x1266A8 (unreachable in the capture: A5+0x30DA is 0xBB8 there, and the
     * script's dead end clears A5+0x30EE, sets +0x54 = 0 and a 0x78 timer, then
     * 0x1266C2). Reaching it would also cue sound 0x13 at 0x1266DA. */
    return -1;
}
