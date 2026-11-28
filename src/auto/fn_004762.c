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
 * 0x0047C0 clocks the latch byte 0x406C52 (Work RAM) out to the write-only
 * port 0x4A0013 and shifts the reply bit 0x4A0001/0 into D0. The harness has
 * no I/O space, so those reads return the zeroed unmapped byte; neither
 * captured command reaches a read (0x800001BF and 0x100 both take the
 * 0x4836/0x48D4 path), and the first compare mismatches, so D0 = A0 = -1.
 * SR is modelled as 0x2000 (supervisor, IPL 0): 0x47C0 saves it at 0x406C50
 * and the capture shows exactly 0x2000 there after the call. */
#include "fn_auto.h"

#define CTRL_LATCH  0x00406C52u
#define CTRL_SR     0x00406C50u
#define CTRL_PORT   0x004A0013u
#define CTRL_IN     0x004A0001u

/* ROM 0x0047C0: clock D1+1 command bits of D0 out of the latch, then receive
 * (returning D0). The latch itself is the Work RAM byte 0x406C52: every
 * bset/bclr is a read-modify-write of that byte (so it is what the harness
 * compares), and each is followed by a write of the byte to the write-only
 * port 0x4A0013 (no RAM effect, kept as a comment). The ROM returns with
 * jmp (A6); both call sites are inlined by the caller below.
 *
 * 0x4A0013 is write-only and 0x4A0001 is the serial reply bit; the harness has
 * no I/O space, so a reply read returns the zeroed unmapped byte. Neither
 * captured command reaches a reply read. */
static uint32_t ctrl_xfer(const f3mem_t *m, uint32_t d0, uint16_t *d1p)
{
    uint8_t latch = rd8(m, CTRL_LATCH);
    uint16_t d1 = *d1p;
    int carry;

    wr16(m, CTRL_SR, 0x2000u);                    /* 0x47C0 move.w SR,0x406C50 */
    for (;;) {
        if (d0 & (1u << (d1 & 31u))) {            /* 0x47CA btst.l D1,D0 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x47EA bclr.b #3 */
            wr8(m, CTRL_LATCH, latch);
            latch = (uint8_t)(latch | 0x04u);             /* 0x47F2 bset.b #2 */
            wr8(m, CTRL_LATCH, latch);
        } else {
            latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x47CE bclr.b #3 */
            wr8(m, CTRL_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x04u);   /* 0x47D6 bclr.b #2 */
            wr8(m, CTRL_LATCH, latch);
        }
        /* 0x47DE / 0x47FA: move.b 0x406C52,0x4A0013 (write-only port). */
        latch = (uint8_t)(latch | 0x10u);         /* 0x4804 bset.b #4 */
        wr8(m, CTRL_LATCH, latch);
        /* 0x480C: port write. */
        latch = (uint8_t)(latch | 0x08u);         /* 0x481C bset.b #3 */
        wr8(m, CTRL_LATCH, latch);
        /* 0x4824: port write. */
        d1 = (uint16_t)(d1 - 1u);                 /* 0x482E dbf D1 */
        if (d1 != 0xFFFFu)
            continue;
        break;
    }
    *d1p = d1;

    carry = (int)((d0 >> 31) & 1u);
    d0 <<= 1;                                     /* 0x4832 add.l D0,D0 */
    if (!carry) {
        carry = (int)((d0 >> 31) & 1u);
        d0 <<= 1;                                 /* 0x4886 add.l D0,D0 */
        if (carry) {
            latch = (uint8_t)(latch & (uint8_t)~0x08u);  /* 0x488A bclr.b #3 */
            wr8(m, CTRL_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x04u);  /* 0x4892 bclr.b #2 */
            wr8(m, CTRL_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x10u);  /* 0x489A bclr.b #4 */
            wr8(m, CTRL_LATCH, latch);
            /* 0x48A2: port write. */
            latch = (uint8_t)(latch | 0x10u);            /* 0x48AC bset.b #4 */
            wr8(m, CTRL_LATCH, latch);
            /* 0x48B4: port write. */
            d1 = 0x270Fu;                                /* 0x48BE move.w #0x270F */
            while (!(rd8(m, CTRL_IN) & 0x01u)) {         /* 0x48C2 btst.b #0 */
                d1 = (uint16_t)(d1 - 1u);
                if (d1 == 0xFFFFu)
                    break;
            }
            *d1p = d1;
        } else {
            /* 0x48D4: clock burst without a receive. Port write after each. */
            latch = (uint8_t)(latch & (uint8_t)~0x10u); wr8(m, CTRL_LATCH, latch); /* 0x48D4 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, CTRL_LATCH, latch); /* 0x48E6 */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, CTRL_LATCH, latch); /* 0x4902 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, CTRL_LATCH, latch); /* 0x491E */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, CTRL_LATCH, latch); /* 0x493A */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, CTRL_LATCH, latch); /* 0x4956 */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, CTRL_LATCH, latch); /* 0x4972 */
        }
        goto tail;
    }

    /* 0x4836: receive path - D0 = 0x00010000, clock in one bit per shift. */
    latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x4836 bclr.b #3 */
    wr8(m, CTRL_LATCH, latch);
    /* 0x483E: port write. */
    d0 = 0x00010000u;                             /* 0x4848 moveq #1 ; 0x484A swap */
    for (;;) {
        latch = (uint8_t)(latch | 0x08u);         /* 0x484C bset.b #3 */
        wr8(m, CTRL_LATCH, latch);
        /* 0x4854: port write. */
        latch = (uint8_t)(latch & (uint8_t)~0x08u); /* 0x485E bclr.b #3 */
        wr8(m, CTRL_LATCH, latch);
        /* 0x4866: port write. */
        if (rd8(m, CTRL_IN) & 0x01u)              /* 0x4870 btst.b #0 */
            d0 += 1u;                             /* 0x487A addq.l #1 */
        carry = (int)((d0 >> 31) & 1u);
        d0 <<= 1;                                 /* 0x487C add.l D0,D0 */
        if (!carry)
            continue;
        d0 = (uint32_t)((int32_t)d0 >> 1);        /* 0x4880 asr.l #1 */
        break;
    }

tail:
    latch = (uint8_t)(latch & (uint8_t)~0x10u);   /* 0x498E bclr.b #4 */
    wr8(m, CTRL_LATCH, latch);
    /* 0x4996: port write. */
    wr16(m, CTRL_SR, 0x2000u);                    /* 0x49A0 move.w 0x406C50,SR */
    return d0;
}

int Fn_004762(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a6 = r->a[6], d2 = r->d[2];
    uint32_t obj, verdict;
    int16_t count;
    uint16_t d1;

    /* 0x004762 movem.l {A6 D2},-(SP): D2 at SP-8, A6 at SP-4. */
    r->a[7] = sp - 8u;
    wr32(m, r->a[7], d2);
    wr32(m, r->a[7] + 4u, a6);

    obj = rd32(m, r->a[7] + 0xCu) + 0x80u;        /* 0x4778/0x477C: arg at entry SP+4 */
    count = 0x3F;                                 /* 0x4780 moveq #0x3F,D2 */
    for (;;) {
        d1 = 9;
        r->d[0] = ctrl_xfer(m, 0x80000180u | (uint16_t)count, &d1); /* 0x4782..0x4790 */
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
    d1 = 9;
    (void)ctrl_xfer(m, 0x100u, &d1);              /* 0x47A6..0x47B2 */
    r->d[1] = d1;                                 /* moveq #9 cleared the high word; dbra leaves 0xFFFF */
    r->a[0] = obj;
    r->d[0] = obj;                                /* 0x47B8 move.l A0,D0 */

    r->a[7] = sp;                                 /* 0x47BA movem.l (SP)+,{D2 A6} */
    r->d[2] = rd32(m, r->a[7]);
    r->a[6] = rd32(m, r->a[7] + 4u);
    r->a[7] = sp + 4u;                            /* the rts pops the caller's PC */
    return 0;
}
