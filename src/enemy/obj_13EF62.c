#include "obj_13EF62.h"

int Obj_13EF62_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x13EF62: cmpi.w #0x7,(0x32de,A5) ; seq (0x3306,A5) */
    int16_t cmp = (int16_t)rd16(m, 0x408000u + 0x32deu);
    wr8(m, F3_A5 + 0x3306u, (cmp == 7) ? 0xFFu : 0x00u);   /* seq: byte, 0xFF/0x00 */

    /* 0x13EF6C: movea.l (0x3308,A5),A1 */
    uint32_t a1 = rd32(m, 0x408000u + 0x3308u);

    /* 0x13EF70: tst.w (0x6,A1) ; bmi.w 0x0013ef7e ; lea (0x8,A1),A1 ; bra.b 0x0013ef70 */
    for (;;) {
        int16_t v = (int16_t)rd16(m, a1 + 0x06u);
        if (v < 0) {
            break;
        }
        a1 += 0x08u;
    }

    /* 0x13EF7E: move.w (A1),(0x22,A0) */
    wr16(m, obj + 0x22u, rd16(m, a1));

    /* 0x13EF82: move.w (0x32dc,A5),D0w ; muls.w #0xa,D0 ; addi.l #0x13f0ee,D0 */
    int32_t d0 = (int32_t)(int16_t)rd16(m, 0x408000u + 0x32dcu) * 10;
    d0 += 0x13f0ee;

    /* 0x13EF90: move.l D0,(0xc,A0) */
    wr32(m, obj + 0x0cu, (uint32_t)d0);

    /* 0x13EF94: clr.w (0x6,A0) */
    wr16(m, obj + 0x06u, 0x0000u);

    /* 0x13EF98: move.l #0x13efa0,(A0) */
    wr32(m, obj + 0x00u, 0x0013efa0u);

    return 0;
}
