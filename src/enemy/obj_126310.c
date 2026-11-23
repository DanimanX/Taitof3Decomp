#include "obj_126310.h"
#include "obj_11B262.h"

/* ROM 0x126310: steering tick (0x1261FE family). The 0x11B288/0x11B2CA/
 * 0x11B262 trio, then the +0x06 countdown; on expiry clear A5+0x30EE{0x10}
 * and {0x5} and continue into 0x126364 while A5+0x30DA > 0x7D0 and
 * A5+0x3272{0x5} is clear, otherwise go to the family's 0x1266A8 dead end.
 * 0x12635A is a `beq` with a truncated flag producer; the golden pairs show
 * the branch NOT taken, so it always falls into 0x12635C. */
int Obj_126310_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126310 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126316 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12631C jsr 0x11B262 */

    /* 0x126322: subq.w #1 -> bgt 0x126308 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126310u);         /* 0x126308 */
        return 0;
    }

    /* 0x126328: bfclr (0x30EE,A5){0x10:1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x10u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x10u % 8u))));
    }
    /* 0x12632E: bfclr (0x30EE,A5){0x5:1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x05u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x05u % 8u))));
    }
    /* 0x126334: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        /* 0x12633E: bftst (0x3272,A5){0x5:1} -> bit 2 of the byte +0x3272;
         * bne 0x1266A8 */
        if ((rd8(m, F3_A5 + 0x3272u + 0x05u / 8u) & (0x80u >> (0x05u % 8u))) == 0u) {
            wr16(m, obj + 0x50u, 0x0100u);         /* 0x126348 */
            wr16(m, obj + 0x52u, 0x0048u);         /* 0x12634E */
            wr16(m, obj + 0x06u, 0x0064u);         /* 0x126354 */
            wr32(m, obj + 0x00u, 0x00126364u);     /* 0x12635C */
            return 0;
        }
    }

    /* 0x1266A8: clr.l (0x30EE,A5); +0x54 = 0; 0x78 timer; -> 0x1266C2 */
    wr32(m, F3_A5 + 0x30EEu, 0u);
    wr8(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x001266C2u);
    return 0;
}
