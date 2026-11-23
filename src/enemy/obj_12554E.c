#include "obj_12554E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"

/* ROM 0x0CBD24 as an expression: D0w ends up as the table offset the step
 * stopped on, and the ROM's caller does not re-read obj+0x3A
 * (0x12557C, 0x125582). The helper hands the resulting sprite def back
 * through obj+0x10, so this repeats the same arithmetic to recover the offset
 * that 0x0CBD24 leaves in D0w. */
static uint16_t sub_0CBD24_offset(const f3mem_t *m, uint32_t obj, uint32_t tbl)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, tbl + off) <= 0)
            return off;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= (int16_t)rd16(m, tbl + off))
            return off;
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

/* ROM 0x12554E: parented muzzle-flash emitter. While the parent's +0x59 is
 * clear it steps the +0x0C animation table with 0x0CBD24: a positive
 * duration falls through to the draw path, a negative one rewinds to frame 0
 * (0x12570A), and a zero duration plays sound 0x1B once (guarded by stealing
 * +0x34), spawns ten 0x0CC756/0x0CDCA0 flashes when bit 10 of A5+0x30EE is
 * cleared, adds 0x29 when bit 9 is set, and otherwise advances +0x3A by 2.
 * Everything then runs 0x11B06E, copies the parent colour +0x1D and integrates
 * through 0x11B09A before re-arming 0x12552E. A set parent byte frees the
 * object (0x125730 -> 0x105EC).
 *
 * Capture note: the parent is 0x4128E0 and its +0x59 lands on the ROM long at
 * 0x122F86, so the captured byte reads non-zero and every pair takes the free
 * path. */
int Obj_12554E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12554E: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x125730 */
    if (rd8(m, rd32(m, obj + 0x08u) + 0x59u) != 0u) {
        Obj_Free(m, obj);                              /* 0x125730 -> 0x0105EC */
        return 0;
    }

    /* 0x12555A: bftst (0x30EE,A5){9:1} ; bne 0x125578 ; tst.b (0x34,A0) ;
     * bne 0x125578 ; move.l #0x3A0F8,(0x10,A0) ; bra 0x125716 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) == 0u && rd8(m, obj + 0x34u) == 0u) {
        wr32(m, obj + 0x10u, 0x0003A0F8u);             /* 0x12556C */
    } else {
        uint16_t off;
        {
            /* 0x125578: movea.l (0xC,A0),A1 */
            uint32_t a1 = rd32(m, obj + 0x0Cu);
            /* 0x12557C: jsr 0x0CBD24 ; tst.w (A1,D0w) */
            Obj_AnimFrameStep(m, obj, a1);
            off = sub_0CBD24_offset(m, obj, a1);
            int16_t dur = (int16_t)rd16(m, a1 + off);
            if (dur > 0)
                goto draw_125716;                      /* 0x125586 */
            if (dur < 0) {
                /* 0x12570A: sf (0x34,A0) ; clr.w (0x3A,A0) ; clr.w (0x3E,A0) */
                wr8(m, obj + 0x34u, 0u);
                wr16(m, obj + 0x3Au, 0u);
                wr16(m, obj + 0x3Eu, 0u);
                goto draw_125716;
            }
        }

        /* 0x12558E: tst.b (0x34,A0) ; bne 0x1255AE */
        if (rd8(m, obj + 0x34u) == 0u) {
            wr8(m, obj + 0x34u, 0xFFu);                /* 0x125596: st (0x34,A0) */
            Sound_PlayHooked(m, 0x001Bu);              /* 0x12559A -> 0x13F238 + 0x39D8 */
        }

        /* 0x1255AE: bfclr (0x30EE,A5){10:1} ; beq 0x1256F8 */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) != 0u) {
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & 0xDFu));
            (void)Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, 0x2801E000u);
            (void)Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, 0x2A05F000u);
            (void)Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, 0x2C0B0000u);
            (void)Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, 0x29111000u);
            (void)Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, 0x27152000u);
            (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x2801E000u);
            (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x2A05F000u);
            (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x2C0B0000u);
            (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x29111000u);
            (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x27152000u);
            Sound_PlayHooked(m, 0x0029u);              /* 0x1256E4 -> 0x13F238 + 0x39D8 */
        }

        /* 0x1256F8: bftst (0x30EE,A5){9:1} ; bne 0x125716 ; addq.w #2,(0x3A,A0) */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u)
            goto draw_125716;
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
    }

draw_125716:
    (void)Obj_11B06E_Tick(m, obj);                     /* 0x125716 */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x125720 */
    (void)Obj_11B09A_Tick(m, obj);                     /* 0x125726 */
    wr32(m, obj + 0x00u, 0x0012554Eu);                 /* 0x12572C -> 0x125546 */
    return 0;
}
