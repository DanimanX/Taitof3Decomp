#include "obj_18B2CA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../audio/sound_cmd.h"

/* ROM 0x18B2CA: per-tick colour/speaker state for the 0x179DEE family.
 * While the script timer +0x24 stays positive it republishes +0x1D (0 / 6 / 8
 * depending on +0x35 and the current value), fires the 0x17ABDC one-shot once,
 * walks the 10-byte movement script (0x178982) and re-arms until the position
 * word +0x50 drops to <= -0x96. On expiry it drops a 0x179DEE burst in groups
 * 5..6, plays sound 0x16, adds 0x4E20 to the spawner's +0x48, sets flags bit 2
 * and arms the 1-frame 0x18B3AE. */

/* ROM 0x18B2C2: move.l #0x18B2CA,(A0); rts */
static void arm_self(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0018B2CAu);
}

/* ROM 0x17B02E: the object that owns this one -- its +0x08 parent when
 * +0x35 == 1, the +0x36 field itself when +0x35 == 2, else -1. */
static int32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t kind = rd8(m, obj + 0x35u);
    if (kind == 1u)
        return (int32_t)rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (kind == 2u)
        return (int32_t)rd32(m, obj + 0x36u);
    return -1;
}

/* ROM 0x18B2CA */
int Obj_18B2CA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B2CA: tst.l (A0+0x24) ; ble 0x18B352 */
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        /* 0x18B2D2: cmpi.b #6,(A0+0x1D) ; beq 0x18B2F8 */
        if (rd8(m, obj + 0x1Du) == 0x06u) {
            wr8(m, obj + 0x1Du, 0x08u);                 /* 0x18B2F8 */
        } else if (rd8(m, obj + 0x35u) != 0u) {
            wr8(m, obj + 0x1Du, 0x06u);                 /* 0x18B2EE */
        } else {
            wr8(m, obj + 0x1Du, 0x00u);                 /* 0x18B2E4 */
        }

        /* 0x18B302: clr.b (A0+0x35) */
        wr8(m, obj + 0x35u, 0x00u);

        /* 0x18B306: btst.b #1,(0x1D58,A5) ; beq 0x18B33E */
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
            /* 0x18B310: bftst (A0+0x23){0:1} -- bit 0 is the MSB of byte 0x23 */
            uint8_t flags = rd8(m, obj + 0x23u);
            if ((flags & 0x80u) == 0u) {
                /* 0x18B31A: bfset (A0+0x23){0:1}, then the one-shot */
                wr8(m, obj + 0x23u, (uint8_t)(flags | 0x80u));
                Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000100u);
            }
        }

        /* 0x18B33E: jsr 0x178982 -- one movement-script step. Obj_Script10
         * only translates a few command addresses; a step it cannot resolve is
         * reported as -1 rather than guessed. */
        if (Obj_Script10(m, obj) < 0)
            return -1;

        /* 0x18B344: cmpi.w #-0x96,(A0+0x50) ; ble 0x18B398 */
        if ((int16_t)rd16(m, obj + 0x50u) > -0x96) {
            arm_self(m, obj);                           /* 0x18B34E */
            return 0;
        }
    }

    /* 0x18B352: pea (0x0).w -- a 0x179DEE burst in groups 5..6 */
    Obj_Spawn(m, obj, 0x00179DEEu, 5u, 2u, 0x00000000u);

    /* 0x18B36E: move.w #0x16,-(SP) ; jsr 0x13F238. The hook adds 0x53 to
     * commands 1..0x40 only when A5+0x3306 is set; 0x39D8 then plays it. */
    {
        int16_t cmd = 0x16;
        if (rd8(m, F3_A5 + 0x3306u) != 0u)
            cmd = (int16_t)(cmd + 0x53);
        Sound_PlayCmd(m, cmd);
    }

    /* 0x18B382: jsr 0x17B02E ; blt 0x18B398 */
    {
        int32_t owner = sub_17B02E(m, obj);
        if (owner >= 0) {
            /* 0x18B390: addi.l #0x4E20,(0x48,A1) -- A1 := D0 */
            uint32_t a1 = (uint32_t)owner;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00004E20u);
        }
    }

    /* 0x18B398: bset.b #2,(A0+0x23) ; move.w #1,(A0+6) ;
     *          move.l #0x18B3AE,(A0) */
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u));
    wr16(m, obj + 0x06u, 0x0001u);
    wr32(m, obj, 0x0018B3AEu);
    return 0;
}

/* The beq at 0x18B3A4 (target 0x18B3B4, Obj_Free) tests the Z left by
 * `move.w #1,(A0+6)`, which can never be set, so that tail is dead. */
