#include "obj_126670.h"
#include "obj_11B262.h"

/* ROM 0x126670: steering tick. The 0x11B288/0x11B2CA/0x11B262 trio, then the
 * +0x06 countdown: while it stays positive the same handler is re-installed.
 * Expiry clears A5+0x30EE bits 8/0xB/0x10 and either branches to 0x1261DE
 * (counter above 0x7D0 still live) or falls into the shared dead end
 * 0x1266A8: clear A5+0x30EE, 0x78 timer, hand over to 0x1266C2. */
int Obj_126670_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126670 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126676 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12667C jsr 0x11B262 */

    /* 0x126682: subq.w #1 -> bgt 0x126668 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126670u);         /* 0x126668 */
        return 0;
    }
    /* 0x126688 / 0x12668E / 0x126694: bfclr (0x30EE,A5){8,0xB,0x10:1} */
    {
        const unsigned bits[3] = { 0x08u, 0x0Bu, 0x10u };
        for (int i = 0; i < 3; i++) {
            uint32_t a = F3_A5 + 0x30EEu + bits[i] / 8u;
            wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (bits[i] % 8u))));
        }
    }
    /* 0x12669A: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        /* 0x1266A4: bra.w 0x1261DE -- falls into 0x1266A8 */
        wr16(m, obj + 0x50u, 0x0130u);             /* 0x1261DE */
        wr16(m, obj + 0x52u, 0x0060u);             /* 0x1261E4 */
        wr16(m, obj + 0x06u, 0x0028u);             /* 0x1261EA */
        wr8(m, obj + 0x54u, 0x05u);                /* 0x1261F0 */
        wr32(m, obj + 0x00u, 0x001261FEu);         /* 0x1261F6 */
        return 0;
    }
    /* 0x1266A8 */
    wr32(m, F3_A5 + 0x30EEu, 0u);                  /* clr.l */
    wr8(m, obj + 0x54u, 0x00u);                    /* 0x1266AC */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1266B2; Z clear */
    wr32(m, obj + 0x00u, 0x001266C2u);             /* 0x1266BA */
    return 0;
}
