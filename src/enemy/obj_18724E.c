/* ROM 0x18724E: the moving half of the 0x187370 spawner family. While the
 * freeze bit (A5+0x4B3A bit 13) is set it bounces back to the idle park at
 * 0x187246; otherwise it seats itself at (parent +0x50/+0x54) + (0xF, 0x36),
 * takes the parent's speed/flip, runs 0x179434, copies its world position into
 * the A5+0x4B6E HUD pair, and -- when bit 12 of the same flag word is clear and
 * the parent is at y >= 0xAE -- spawns the next wave from one of the four
 * 0x187360 slots, then parks itself on 0x187246 for the frame. */
#include "obj_18724E.h"
#include "obj_187246.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Obj_18724E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18724E: bftst.b (A5+0x4B3A){13:1} ; beq.b 0x187246. A memory bit-field
     * is numbered from the MSB of the byte it starts in, so {13:1} is bit 13
     * of the 32-bit big-endian long at A5+0x4B3A = bit 2 of the word (0x0004). */
    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0004u) == 0u) {
        return Obj_187246_Tick(m, obj);
    }

    /* 0x187256..0x187280: A4 = parent ; place at parent +0x50/+0x54 + (0xF, 0x36) */
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + 0x000Fu);
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + 0x0036u);
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);
    wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));   /* 0x18727A */
    wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));   /* 0x187280 */

    /* 0x187286: jsr 0x179434 (world -> screen coordinates) */
    Obj_BossMapScreenPos(m, obj);

    /* 0x18728C..0x187298: HUD scratch words */
    wr16(m, F3_A5 + 0x4B6Eu, (uint16_t)(rd16(m, obj + 0x14u) - 0x003Fu));
    wr16(m, F3_A5 + 0x4B70u, rd16(m, obj + 0x18u));

    /* 0x18729E: bne 0x187348 when bit 12 is set; 0x1872A8: blt 0x187348 when
     * the parent's y is below 0xAE. Otherwise:
     * 0x1872B2: bfextu (A5+0x878){14:2},D0 ; movea.l (0x187360,D0w*4),A1 ;
     *           jmp (A1). */
    /* 0x18729E: bftst.b {12:1} -> bit 12 of the long = bit 3 of the word. */
    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0008u) != 0u)
        goto tail;
    if ((int16_t)rd16(m, a4 + 0x54u) < (int16_t)0x00AEu)
        goto tail;

    /* The pushed argument is D3 << 16 with D3 in {0, 1}: bfextu (A5-0x1456){14:1}
     * numbers from the MSB of the long at A5-0x1456, i.e. bit 14 -> bit 1 of
     * the word. */
    {
        uint32_t arg = (uint32_t)((rd16(m, F3_A5 - 0x1456u) >> 1) & 0x01u) << 16;
        /* 0x1872B2: bfextu $878(A5){14:2},D0 -- NOT settled. If the field is
         * numbered from the MSB of the long at A5+0x878 it is word bit 8 /
         * bit 7, i.e. (v >> 8) & 3; the capture cannot arbitrate: its four
         * ticks that take this path (50..59) come from a ROM build whose
         * 0x187360 table targets (0x0BDxxx) are not in work/maincpu.bin, so
         * slot identity there is unknown. Both variants score 50/60 and
         * switching slot 1 and 3 in the ROM copy changes nothing, so the
         * spawn slots are not observable from this capture either way. */
        uint16_t sel = (uint16_t)((rd16(m, F3_A5 + 0x0878u) >> 1) & 0x03u);

        switch (sel) {
        case 0u:                                    /* 0x1872C0 */
            (void)Obj_Spawn(m, obj, 0x00187370u, 5u, 2u, arg);
            break;
        case 2u:                                    /* 0x1872E2 */
            (void)Obj_Spawn(m, obj, 0x00187370u, 0u, 1u, arg);
            break;
        case 1u:                                    /* 0x187304 */
            (void)Obj_Spawn(m, obj, 0x0018747Cu, 5u, 2u, arg);
            break;
        default:                                    /* 0x187326 */
            (void)Obj_Spawn(m, obj, 0x0018747Cu, 0u, 1u, arg);
            break;
        }
    }

tail:
    /* 0x187348: tst.b (A0+0x1E) ; bne 0x18735C ; move.w (A5-0x1456),D0 ;
     * andi.w #0xF,D0 ; bne 0x18735C ; 0x18735C: bra.w 0x187246. Both branches
     * reach the park routine, which rewrites the tick fn to 0x18724E. */
    return Obj_187246_Tick(m, obj);
}
