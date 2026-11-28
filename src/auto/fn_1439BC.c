/* ROM 0x1439BC: UNVERIFIED (no capture): stage-8 boss task entry.
 * It calls 0x143A16 and that call does NOT return: 0x143A16 ends its first
 * pass in a TRAP #5 at 0x143A8E, so the task yields inside the bsr and the
 * continuation (0x143A90 -> rts -> 0x1439C0 bra 0x1439EA) runs on the next
 * schedule of the same task. Fn_1439BC therefore covers everything up to that
 * yield; Task1439BC_Frame below runs the continuation the ROM reaches next.
 *
 * 0x143A16 (prefix): point the wave-list pointers A5+0x3308 / A5+0x3320 at
 * 0x0040C2B4, scan its 0xFF eight-byte slots for the first one whose +0x06
 * word is negative; if none is found, stamp the base slot with {0x1400, -1}.
 * Then stamp the following slot (base + 0x7F8) with {0x4000, -1}, reload sound
 * bank 0 (0x0014DA), create three kernel tasks -- (id 4, 0x10488), (id 0xE,
 * 0x10502), (id 3, 0x0C63E8) -- through TRAP #1 (0x001E76), clear bit 1 of
 * A5-0x144C and yield at TRAP #5 (0x143A8E).
 *
 * 0x1439C4..0x1439E8 is the task's own resume loop (`restore the scroll
 * A5+0x4224/28 from A5+0x3370/74, TRAP #5, mirror A5+0x3308 into A5+0x3320,
 * stop on A5-0x144B bit 4, loop while A5-0x5DDB bit 6`); it is not reachable
 * from 0x1439BC and belongs to whatever re-arms the task at 0x1439C4.
 *
 * Continuation (Task1439BC_Frame): spawn object 0x143AAE (group 0x0B, count 1,
 * arg 0), then 0x1439EA clears A5+0x331C, saves the scroll registers into
 * A5+0x3370/0x3374, zeroes them, issues TRAP #8 with mask 0x1FE6 (0x00205C),
 * sets A5+0x1D36 bit 3, and tail-jumps to the 0x144894 task body, which clears
 * the A5-0x5D56-offset 0xE8 attribute words at 0x627200 and yields. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../kernel/fn_010420.h"
#include "../kernel/kernel.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* TRAP #1 (0x001E76): the handler reads its arguments at A7+8 (the entry point,
 * pushed last) and A7+12 (the id, pushed first), where A7 is the value the
 * handler sees; the RTE and the caller's `addq.l #8,A7` leave SP balanced. */
static void trap01_create(const f3mem_t *m, f3regs_t *r, uint32_t id, uint32_t entry)
{
    uint32_t sp = r->a[7];

    wr32(m, sp - 8u, entry);                 /* 0x143A5E pea.l */
    wr32(m, sp - 4u, id);                    /* 0x143A58 move.l #id,-(A7) */
    r->a[7] = sp - 16u;
    (void)Kern_001E76(m, r);                 /* 0x143A64 trap #1 */
    r->a[7] = sp;                            /* 0x143A66 addq.l #8,A7 */
}

/* TRAP #8 (0x00205C): one long mask argument at A7+8, popped by `addq.l #4`. */
static void trap08_wake(const f3mem_t *m, f3regs_t *r, uint32_t mask)
{
    uint32_t sp = r->a[7];

    wr32(m, sp - 4u, mask);                  /* 0x143A02 move.l #mask,-(A7) */
    r->a[7] = sp - 12u;
    (void)Kern_00205C(m, r);                 /* 0x143A08 trap #8 */
    r->a[7] = sp;                            /* 0x143A0A addq.l #4,A7 */
}

/* 0x143A16 up to its TRAP #5 at 0x143A8E. */
static void init_143A16(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = 0x0040C2B4u;               /* 0x143A16 lea.l 0x40C2B4,A0 */

    wr32(m, F3_A5 + 0x3308u, a0);            /* 0x143A1C move.l A0,(0x3308,A5) */
    wr32(m, F3_A5 + 0x3320u, a0);            /* 0x143A20 move.l A0,(0x3320,A5) */
    r->a[0] = a0;
    r->a[1] = a0;                            /* 0x143A24 movea.l A0,A1 */

    {
        uint16_t d0 = 0x00FEu;               /* 0x143A26 move.w #0xFE,D0 */
        for (;;) {
            if ((int16_t)rd16(m, a0 + 0x06u) < 0)  /* 0x143A2A tst.w / 0x143A2E bmi */
                break;
            a0 += 8u;                        /* 0x143A32 lea (0x8,A0),A0 */
            d0--;                            /* 0x143A36 dbra D0 */
            if ((int16_t)d0 < 0) {
                /* No free slot: terminate the list in its first slot. */
                wr16(m, 0x0040C2B4u, 0x1400u);     /* 0x143A3A move.w #0x1400,(A1) */
                wr16(m, 0x0040C2BAu, 0xFFFFu);     /* 0x143A3E move.w #0xFFFF,(0x6,A1) */
                break;
            }
        }
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
    }
    /* 0x143A44 lea (0x7F8,A1),A0 ; 0x143A48/0x143A4C */
    wr16(m, 0x0040CAACu, 0x4000u);
    wr16(m, 0x0040CAB2u, 0xFFFFu);
    r->a[0] = 0x0040CAACu;                   /* 0x143A44 lea (0x7F8,A1),A0 */

    Kernel_SoundBank0_0014DA(m);             /* 0x143A52 jsr 0x0014DA */

    trap01_create(m, r, 0x00000004u, 0x00010488u); /* 0x143A58..0x143A66 */
    trap01_create(m, r, 0x0000000Eu, 0x00010502u); /* 0x143A68..0x143A76 */
    trap01_create(m, r, 0x00000003u, 0x000C63E8u); /* 0x143A78..0x143A86 */

    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xFDu)); /* 0x143A88 bclr.b #1 */
    /* 0x143A8E trap #5: the task yields here (A7 points at the trap frame). */
}

/* 0x144894 up to its TRAP #5 at 0x1448C0. */
static void ending_144894(const f3mem_t *m, f3regs_t *r)
{
    Gfx_ClearPf3(m);                         /* 0x144894 jsr 0x0058AE */
    Stage_DrawBanner13D11E(m);               /* 0x14489A jsr 0x13D11E */
    Leaf_13CAAE(m, r);                       /* 0x1448A0 jsr 0x13CAAE */

    /* 0x1448A6 lea 0x627200,A0 ; 0x1448AC adda.w (A5-0x5D56),A0 ;
     * 0x1448B0 move.w #0xE7,D0 ; clear bits 14-15 of 0xE8 words. */
    {
        uint32_t a0 = 0x00627200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        uint16_t d0 = 0x00E7u;
        uint16_t d1 = 0;
        do {
            d1 = (uint16_t)(rd16(m, a0) & 0x3FFFu);   /* 0x1448B4 move.w (A0),D1 ; andi.w */
            wr16(m, a0, d1);                          /* 0x1448BA move.w D1,(A0)+ */
            a0 += 2u;
            d0--;                                     /* 0x1448BC dbra D0 */
        } while ((int16_t)d0 >= 0);
        r->a[0] = a0;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0; /* D0w = -1 after dbra */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1;
    }
    /* 0x1448C0 trap #5: the task yields; the menu that follows (0x1448C2..)
     * ends in `jmp (A0)` through the option table at 0x144AF0. */
}

int Fn_1439BC(const f3mem_t *m, f3regs_t *r)
{
    init_143A16(m, r);                       /* 0x1439BC bsr.w 0x143A16 (yields inside) */
    return 0;
}

/* One pass of the loop after the first yield: 0x143A90 (the rest of 0x143A16)
 * through the 0x1439EA block to the 0x144894 yield at 0x1448C0. */
int Task1439BC_Frame(const f3mem_t *m, f3regs_t *r)
{
    /* 0x143A90 pea.l 0 ; move.w #1,-(A7) ; move.w #0xB,-(A7) ;
     * move.l #0x143AAE,-(A7) ; jsr 0x01057C ; lea 0xC(A7),A7 */
    r->d[0] = Obj_Spawn(m, r->a[0], 0x00143AAEu, 0x000Bu, 1u, 0u);
    /* 0x143AAC rts -> 0x1439C0 bra.w 0x1439EA */

    wr16(m, F3_A5 + 0x331Cu, 0u);            /* 0x1439EA clr.w (0x331C,A5) */
    wr32(m, F3_A5 + 0x3370u, rd32(m, F3_A5 + 0x4224u)); /* 0x1439EE */
    wr32(m, F3_A5 + 0x3374u, rd32(m, F3_A5 + 0x4228u)); /* 0x1439F4 */
    wr32(m, F3_A5 + 0x4224u, 0u);            /* 0x1439FA clr.l (0x4224,A5) */
    wr32(m, F3_A5 + 0x4228u, 0u);            /* 0x1439FE clr.l (0x4228,A5) */

    trap08_wake(m, r, 0x00001FE6u);          /* 0x143A02..0x143A0A */

    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x08u)); /* 0x143A0C bset.b #3 */

    ending_144894(m, r);                     /* 0x143A12 bra.w 0x144894 */
    return 0;
}
