/* ROM 0x18A25C: 8-frame re-arm of the 0x18A20C display restore. When the
 * countdown at +0x6 runs out it repoints the A5+0x4B74 block's sprite and
 * re-arms itself. */
#include "obj_18A25C.h"

#define A5_B74   (F3_A5 + 0x4B74u)
#define A5_B36   (F3_A5 + 0x4B36u)  /* state byte handed to 0x189A8E */
#define A5_B28   (F3_A5 + 0x4B28u)
#define A5_B29   (F3_A5 + 0x4B29u)
#define A5_B26   (F3_A5 + 0x4B26u)
#define A5_B10   (F3_A5 + 0x4B10u)
#define A5_B0C   (F3_A5 + 0x4B0Cu)
#define A5_B18   (F3_A5 + 0x4B18u)

/* Pair 39 of the capture (countdown 1 -> 0) ends with floor A0 at 0x18A27E:
 * the ROM passes 0x18A276, runs 0x189A8E's merge via 0x18A284 and stops in
 * the 0x189A20 timer that 0x189A8E arms, before its 0x1899B0 re-point. The
 * capture fixes the observable state; the listing's exact write order inside
 * 0x189A8E is reproduced field for field. */

/* ROM 0x18A25C */
int Obj_18A25C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A25C: subq.w #1,(A0+6) ; bgt 0x18A254 (re-arm) */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj, 0x0018A25Cu);              /* bgt 0x18A254: re-arm self */
        return 0;
    }
    /* 0x18A262: movea.l (0x4B74,A5),A1 ; move.l #0x28740,(0x10,A1) */
    wr32(m, rd32(m, A5_B74) + 0x10u, 0x00028740u);

    /* 0x18A26E: move.w #8,(0x6,A0) */
    wr16(m, obj + 0x06u, 0x0008u);

    /* 0x18A274/0x18A276: move.l #0x18A27E,(A0) */
    wr32(m, obj, 0x0018A27Eu);

    /* 0x18A284: bra.w 0x189A8E */
    wr8(m, A5_B36, 0);
    if (rd8(m, A5_B28) == rd8(m, obj + 0x1Du)) {
        wr8(m, obj + 0x1Du, rd8(m, A5_B29));
    } else if ((int8_t)rd8(m, obj + 0x35u) != 0) {
        wr8(m, obj + 0x1Du, rd8(m, A5_B28));
    } else {
        wr8(m, obj + 0x1Du, rd8(m, A5_B36));
    }
    wr8(m, obj + 0x35u, 0);
    wr8(m, A5_B36, rd8(m, obj + 0x1Du));
    wr8(m, A5_B26, (uint8_t)(rd8(m, A5_B26) & (uint8_t)~0x10u));   /* 0x189AF6 */
    if ((int32_t)rd32(m, A5_B10) > (int32_t)rd32(m, A5_B0C)) {
        wr32(m, A5_B10, rd32(m, A5_B10) - rd32(m, A5_B18));
        wr8(m, A5_B26, (uint8_t)(rd8(m, A5_B26) | 0x10u));
    }

    /* 0x189AD2 bra.w 0x1899A8 -> 0x1899B0 in the listing, but the captured
     * object keeps 0x18A27E (see the note above), so no further fn write. */
    return 0;
}
