/* ROM 0x004762: read the serial settings device through 0x0047C0 and compare
 * it with the RAM block 0x80 bytes above the pointer argument:
 *
 *   004762  movem.l {A6 D2},-(SP)
 *   004766  D0 = 0x130 ; D1 = 9 ; A6 = 0x4778 ; jmp 0x0047C0
 *   004778  A0 = *(SP+0xC) (the first stack argument) + 0x80 ; D2 = 0x3F
 *   004782  loop: D0 = 0x80000180 | D2 ; call 0x0047C0 (returns to 0x4796)
 *   004796          cmp.w -(A0),D0   ; the read walked backwards over the block
 *   004798          beq 0x479E -> dbra D2, 0x4782 (all 0x40 words matched)
 *   00479A          mismatch: D0 = -1, done
 *   0047A2          all matched: D0 = 0
 *   0047A4  A0 = D0 ; D0 = 0x100 ; D1 = 9 ; call 0x0047C0 (returns to 0x47B8)
 *   0047B8  D0 = A0 (0 or -1) ; movem.l (SP)+,{D2 A6} ; rts
 *
 */
#include "eeprom_block.h"

int Fn_004762(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a6 = r->a[6], d2 = r->d[2];
    uint16_t sr = m->eeprom ? r->sr : 0x2000u;
    uint32_t obj, verdict;
    int16_t count;

    /* 0x004762 movem.l {A6 D2},-(SP): D2 at SP-8, A6 at SP-4. */
    r->a[7] = sp - 8u;
    wr32(m, r->a[7], d2);
    wr32(m, r->a[7] + 4u, a6);

    ee_xfer(m, 0x130u, 9, sr);                    /* 0x004766..0x004774 */
    obj = rd32(m, r->a[7] + 0xCu) + 0x80u;        /* 0x4778/0x477C: arg at entry SP+4 */
    count = 0x3F;                                 /* 0x4780 moveq #0x3F,D2 */
    for (;;) {
        r->d[0] = ee_xfer(m, 0x80000180u | (uint16_t)count, 9, sr); /* 0x4782..0x4790 */
        obj -= 2u;                                /* 0x4796 cmp.w -(A0),D0 */
        if ((uint16_t)r->d[0] != rd16(m, obj)) {  /* beq 0x479E */
            verdict = 0xFFFFFFFFu;                /* 0x479A moveq #-1 */
            break;
        }
        count = (int16_t)(count - 1);             /* 0x479E dbra D2 */
        if (count == -1) {
            verdict = 0;                          /* 0x47A2 moveq #0 */
            break;
        }
    }

    obj = verdict;                                /* 0x47A4 movea.l D0,A0 */
    ee_xfer(m, 0x100u, 9, sr);                    /* 0x47A6..0x47B2 */
    r->d[1] = 0xFFFFu;                            /* MOVEQ then DBRA */
    r->a[0] = obj;
    r->d[0] = obj;                                /* 0x47B8 move.l A0,D0 */

    /* 0x47BA movem.l (SP)+,{D2 A6} */
    r->d[2] = rd32(m, r->a[7]);
    r->a[6] = rd32(m, r->a[7] + 4u);
    r->a[7] = sp + 4u;                            /* the rts pops the caller's PC */
    return 0;
}
