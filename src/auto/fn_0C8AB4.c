/* ROM 0x0C8AB4: the lives counter of the HUD (A1 = player state record).
 * Register-level wrapper: the two 0x005576 text blits are Fn_005576 and the
 * counter lookup is Fn_0C8B48, so no logic is duplicated.
 *
 *   0C8AB4  movem.l {D0 A0 A1},-(A7)
 *   0C8AB8  lea.l (0xC8B5E,PC),A0      ; 11-long script table
 *   0C8ABE  cmpi.w #0x0,(0x22,A1)      ; player slot 0 -> 0x61C09E/0x61C0A2
 *   0C8AC8  move.w #0x24,-(A7) / pea 0x61C09E / move.l #0x00010B5C,-(A7)
 *   0C8AD8  jsr 0x005576               ; the "LIVES" label
 *   0C8AE2  bsr 0x0C8B48               ; D0 = table[min((0x4C,A1),10)]
 *   0C8AE6  move.w #0x24 / pea 0x61C0A2 / move.l D0,-(A7) / jsr 0x005576
 *   0C8AFC  bra 0x0C8B42
 *   0C8B00  cmpi.w #0x1,(0x22,A1)      ; slot 1 -> 0x61C0AC/0x61C0B0, 0x22
 *   0C8B42  movem.l (A7)+,{D0 A0 A1} ; rts
 *
 * Both paths end in the same movem, so D0/A0/A1 are restored: the capture's
 * post registers equal the pre ones and only text RAM 0x61C000.. moves.
 * 0x005576's direction mode comes from the flags word 0x24 (slot 0) / 0x22
 * (slot 1) pushed as its third argument. */
#include "fn_auto.h"

int Fn_0C8AB4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a0 = r->a[0], a1 = r->a[1], d0 = r->d[0];
    int16_t slot = (int16_t)rd16(m, a1 + 0x22u);   /* 0x0C8ABE / 0x0C8B00 */

    r->a[0] = 0x000C8B5Eu;                         /* 0x0C8AB8 lea (0xC8B5E,PC) */

    if (slot == 0 || slot == 1) {
        uint16_t flags = (uint16_t)(slot == 0 ? 0x24 : 0x22);
        uint32_t dst_label = slot == 0 ? 0x0061C09Eu : 0x0061C0ACu;
        uint32_t dst_count = slot == 0 ? 0x0061C0A2u : 0x0061C0B0u;

        /* first 0x005576: the label blit, src = ROM 0x00010B5C */
        wr16(m, sp - 2u, flags);                   /* 0x0C8AC8 move.w #flags,-(A7) */
        wr32(m, sp - 6u, dst_label);               /* 0x0C8ACC pea.l */
        wr32(m, sp - 10u, 0x00010B5Cu);            /* 0x0C8AD2 move.l #imm,-(A7) */
        r->a[7] = sp - 14u;                        /* the jsr's return address */
        Fn_005576(m, r);                           /* 0x0C8AD8 */
        r->a[7] = sp;                              /* 0x0C8ADE lea (0xA,A7),A7 */

        Fn_0C8B48(m, r);                           /* 0x0C8AE2 bsr */

        /* second 0x005576: the digits, src = the table long in D0 */
        wr16(m, sp - 2u, flags);                   /* 0x0C8AE6 */
        wr32(m, sp - 6u, dst_count);               /* 0x0C8AEA pea.l */
        wr32(m, sp - 10u, r->d[0]);                /* 0x0C8AF0 move.l D0,-(A7) */
        r->a[7] = sp - 14u;
        Fn_005576(m, r);                           /* 0x0C8AF2 */
        r->a[7] = sp;                              /* 0x0C8AF8 lea (0xA,A7),A7 */
    }

    /* 0x0C8B42 movem.l (A7)+,{D0 A0 A1} ; rts */
    r->d[0] = d0;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] = sp + 4u;
    return 0;
}
