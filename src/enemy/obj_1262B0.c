#include "obj_1262B0.h"
#include "obj_11B262.h"

/* ROM 0x1262B0: steering tick (0x1261FE family). Same trio; on +0x06 expiry
 * set A5+0x30EE{0x1} and stay in the 0x126310 step while A5+0x30DA > 0x7D0
 * and A5+0x3272{0x3} is clear, otherwise go to the family's 0x1266A8 dead end.
 * 0x126306 is again a `beq` with a truncated flag producer; the golden pair
 * keeps +0x06 = 0x12C, so the branch is NOT taken and 0x126308 always runs. */
int Obj_1262B0_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1262B0 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1262B6 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1262BC jsr 0x11B262 */

    /* 0x1262C2: subq.w #1 -> bgt 0x1262A8 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001262B0u);         /* 0x1262A8 */
        return 0;
    }

    /* 0x1262C8: bfset (0x30EE,A5){0x1:1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x01u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x01u % 8u))));
    }
    /* 0x1262CE: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        /* 0x1262D8: bftst (0x3272,A5){0x3:1} -> bit 4 of the byte +0x3272;
         * bne 0x1266A8 */
        if ((rd8(m, F3_A5 + 0x3272u + 0x03u / 8u) & (0x80u >> (0x03u % 8u))) == 0u) {
            wr16(m, obj + 0x50u, 0x00E0u);         /* 0x1262E2 */
            wr16(m, obj + 0x52u, 0x0070u);         /* 0x1262E8 */
            wr8(m, obj + 0x54u, 0x05u);            /* 0x1262EE */
            /* 0x1262F4: bfset (0x30EE,A5){0x10:1} */
            {
                uint32_t a = F3_A5 + 0x30EEu + 0x10u / 8u;
                wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x10u % 8u))));
            }
            /* 0x1262FA: bfset (0x30EE,A5){0x5:1} */
            {
                uint32_t a = F3_A5 + 0x30EEu + 0x05u / 8u;
                wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x05u % 8u))));
            }
            wr16(m, obj + 0x06u, 0x012Cu);         /* 0x126300 */
            wr32(m, obj + 0x00u, 0x00126310u);     /* 0x126308 */
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
