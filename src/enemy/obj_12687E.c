#include "obj_12687E.h"
#include "obj_11B262.h"

/* 0x126896: bfset (0x30EE,A5){bit:1} -- bfset with Dn bit = 0 sets */
static void bfset_bit(const f3mem_t *m, unsigned bit)
{
    uint32_t a = F3_A5 + 0x30EEu + bit / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (bit % 8u))));
}

/* 0x12689C: bfclr (0x30EE,A5){bit:1} -- bfclr with Dn bit = 0 clears */
static void bfclr_bit(const f3mem_t *m, unsigned bit)
{
    uint32_t a = F3_A5 + 0x30EEu + bit / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (bit % 8u))));
}

/* ROM 0x12687E: steering tick (0x11B288 / 0x11B2CA / 0x11B262) with the
 * countdown at +0x06; on expiry set A5+0x30EE{1}, clear {0x11}, then start
 * the 0x1268AC step while the counter A5+0x30DA stays above 0x3E8; otherwise
 * go to the family's 0x1266A8 dead end via 0x12695C. */
int Obj_12687E_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x12687E */
    ObjHelper_11B2CA(m, obj);                          /* 0x126884 */
    ObjHelper_11B262(m, obj);                          /* 0x12688A */

    /* 0x126890: subq.w #1 -> bgt 0x126876 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0012687Eu);             /* 0x126876 */
        return 0;
    }

    bfset_bit(m, 0x01u);                               /* 0x126896 */
    bfclr_bit(m, 0x11u);                               /* 0x12689C */
    /* 0x1268A2: cmpi.w #0x3E8,(0x30DA,A5); ble.w 0x12695C */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x03E8) {
        /* 0x12695C */
        wr32(m, F3_A5 + 0x30EEu, 0u);
        wr8(m, obj + 0x54u, 0x00u);
        wr16(m, obj + 0x06u, 0x0078u);                 /* 0x126966 */
        /* 0x12696C beq -> 0x12698E: move.w only leaves Z set if the word
         * stored was zero, so 0x1E/0x78 always take the fall-through to
         * 0x12696E (0x12698E would also need the sound path). */
        wr32(m, obj + 0x00u, 0x00126976u);             /* 0x12696E */
        return 0;
    }

    /* 0x1268AC: step 6 (50 frames) */
    wr8(m, obj + 0x54u, 0x06u);
    wr16(m, obj + 0x50u, 0x0130u);
    wr16(m, obj + 0x52u, 0x0070u);
    wr16(m, obj + 0x3Eu, 0x000Cu);                     /* 0x1268BE */
    wr16(m, obj + 0x06u, 0x001Eu);                     /* 0x1268C4 */
    /* 0x1268CA beq -> 0x1268EC: same reasoning, 0x1E keeps Z clear */
    wr32(m, obj + 0x00u, 0x001268D4u);                 /* 0x1268CC */
    return 0;
}
