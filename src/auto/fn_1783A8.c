/* ROM 0x1783A8: (capture fnt_: up to the first yield): kernel task body (the RAM-test /
 * display screen). The prefix clears A5-0x144B, rebuilds the line-scroll
 * tables (0x005B0C), reloads sound bank 0 (0x0014DA), clears the text layer
 * (0x005880) and creates six kernel tasks through TRAP #1: (2, 0x144B60),
 * (3, 0x0C63E8), (5, 0x0C6A64), (6, 0x13F2B6), (4, 0x10488), (0xE, 0x10502);
 * it then clears A5+0x4ACE and yields at TRAP #5 (0x178422).
 *
 * Task1783A8_Frame runs one pass of the loop that follows the yield
 * (0x178424..0x178492): while A5-0x5DDB bit 6 is set and A5+0x4ACE < 0x3000 it
 * prints two 8-digit hex values (A5-0x5DA8, A5-0x5DA4) and the 4-digit counter
 * at 0x61C034 / 0x61C0B4 / 0x61C134 and increments the counter. Otherwise it
 * runs the exit prefix 0x178494 (A5+0x4AD0 = -1, 0x005880, sound bank 0) and
 * issues TRAP #3 (task exit). The code after TRAP #3 (0x1784A8: read of
 * address 4, 0x0B7506 continue prompt) only runs if the task is woken again
 * after leaving, and 0x0B7506 is not translated here. */
#include "fn_auto.h"
#include "../kernel/fn_010420.h"
#include "../kernel/kernel.h"
#include "../video/sprite.h"

/* TRAP #1 (0x001E76): the handler reads its arguments at A7+8 (the entry
 * point, pushed last) and A7+12 (the id, pushed first). */
static void trap01_create(const f3mem_t *m, f3regs_t *r, uint32_t id, uint32_t entry)
{
    uint32_t sp = r->a[7];

    wr32(m, sp - 8u, entry);                 /* 0x1783C4 pea.l */
    wr32(m, sp - 4u, id);                    /* 0x1783BE move.l #id,-(A7) */
    r->a[7] = sp - 16u;
    (void)Kern_001E76(m, r);                 /* 0x1783CA trap #1: sets D0 = slot */
    r->a[7] = sp;                            /* 0x1783CC addq.l #8,A7 */
}

/* ROM 0x005880: clear the 0x800 text longs at 0x61C000 and four counters. */
static void sub_005880(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x800u; i++)                        /* 0x005884..0x005892 */
        wr32(m, 0x0061C000u + 4u * i, 0u);
    wr32(m, F3_A5 - 0x5D88u, 0u);                                /* 0x005898 */
    wr16(m, F3_A5 - 0x5D84u, 0u);                                /* 0x00589C */
    wr32(m, F3_A5 - 0x5D60u, 0u);                                /* 0x0058A0 */
    wr16(m, F3_A5 - 0x5D5Cu, 0u);                                /* 0x0058A4 */
}

/* ROM 0x005BE8: fill 0xE8 words at A1 with D0.w, then skip 0x30 bytes. */
static uint32_t sub_005BE8(const f3mem_t *m, uint32_t a1, uint16_t d0)
{
    for (uint16_t i = 0; i <= 0x00E7u; i++)                      /* 0x005BEC dbf #0xE7 */
        wr16(m, a1 + 2u * (uint32_t)i, d0);
    return a1 + 0x1D0u + 0x30u;                                  /* 0x005BF2 lea (0x30,A1),A1 */
}

/* ROM 0x005B0C: rebuild the line-scroll tables at 0x624000+scroll and the
 * 0x620000 row table, then set the two line-RAM control words. */
static void sub_005B0C(const f3mem_t *m)
{
    uint32_t tbl = 0x00005BF8u;                                  /* 0x005B0C lea (0xEA,PC) */
    uint32_t a1 = 0x00624000u;                                   /* 0x005B10 */
    uint16_t d0 = (rd8(m, F3_A5 - 0x5D58u) == 0u) ? 0x0030u : 0x0000u; /* 0x005B18..0x005B20 */
    a1 += d0;                                                    /* 0x005B22 adda.l */
    wr16(m, F3_A5 - 0x5D56u, d0);                                /* 0x005B24 */

    for (int row = 0; row < 8; row++) {                          /* 0x005B16 dbf #7 */
        for (int col = 0; col < 4; col++) {                      /* 0x005B28..0x005B3C */
            uint16_t v = rd16(m, tbl);
            tbl += 2u;
            a1 = sub_005BE8(m, a1, v);
        }
        a1 += 0x800u;                                            /* 0x005B40 */
    }

    a1 = 0x00620000u;                                            /* 0x005B48 */
    uint32_t d = 0x00624000u;                                    /* 0x005B50 */
    d >>= 8;                                                     /* 0x005B56 */
    d >>= 4;                                                     /* 0x005B58 */
    d <<= 8;                                                     /* 0x005B5A */
    d <<= 2;                                                     /* 0x005B5C */
    d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d + 0x000Fu);    /* 0x005B5E addi.w */
    d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d & 0x7FFFu);    /* 0x005B62 andi.w */
    for (int row = 0; row < 8; row++) {                          /* 0x005B4E dbf #7 */
        for (uint16_t i = 0; i <= 0x00FFu; i++)                  /* 0x005B66..0x005B6C */
            wr16(m, a1 + 2u * (uint32_t)i, (uint16_t)d);
        a1 += 0x200u;
        d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d + 0x0400u); /* 0x005B72 */
    }

    if ((int16_t)rd16(m, F3_A5 - 0x5D58u) == 0) {                /* 0x005B7A tst.w */
        wr16(m, 0x00626000u, 0x0800u);                           /* 0x005B82 */
        wr16(m, 0x006275FEu,
             (uint16_t)(rd16(m, 0x006275FEu) | 0x0080u));        /* 0x005B8A..0x005B94 */
        wr16(m, 0x006257FEu, 0x6E00u);                           /* 0x005B9A */
        wr16(m, 0x006247FEu,
             (uint16_t)(rd16(m, 0x006247FEu) | 0x8000u));        /* 0x005BA2..0x005BAC */
        return;                                                  /* 0x005BB2 bra 0x5BE6 */
    }
    wr16(m, 0x006261FEu, 0x0800u);                               /* 0x005BB6 */
    wr16(m, 0x00627400u,
         (uint16_t)(rd16(m, 0x00627400u) | 0x0080u));            /* 0x005BBE..0x005BC8 */
    wr16(m, 0x00625600u, 0x6E00u);                               /* 0x005BCE */
    wr16(m, 0x00624600u,
         (uint16_t)(rd16(m, 0x00624600u) | 0x8000u));            /* 0x005BD6..0x005BE0 */
}

/* 0x1783A8 up to its TRAP #5 at 0x178422. */
int Fn_1783A8(const f3mem_t *m, f3regs_t *r)
{
    wr8(m, F3_A5 - 0x144Bu, 0x00u);                              /* 0x1783A8 */
    sub_005B0C(m);                                               /* 0x1783AC jsr 0x005B0C */
    Kernel_SoundBank0_0014DA_Regs(m, r);                                /* 0x1783B2 jsr 0x0014DA */
    sub_005880(m);                                               /* 0x1783B8 jsr 0x005880 */

    trap01_create(m, r, 0x0002u, 0x00144B60u);                   /* 0x1783BE..0x1783CC */
    trap01_create(m, r, 0x0003u, 0x000C63E8u);                   /* 0x1783CE..0x1783DC */
    trap01_create(m, r, 0x0005u, 0x000C6A64u);                   /* 0x1783DE..0x1783EC */
    trap01_create(m, r, 0x0006u, 0x0013F2B6u);                   /* 0x1783EE..0x1783FC */
    trap01_create(m, r, 0x0004u, 0x00010488u);                   /* 0x1783FE..0x17840C */
    trap01_create(m, r, 0x000Eu, 0x00010502u);                   /* 0x17840E..0x17841C */

    wr16(m, F3_A5 + 0x4ACEu, 0u);                                /* 0x17841E */
    return 0;                                                    /* 0x178422 trap #5 (yield) */
}

/* One pass of the loop that follows the yield. Returns 1 when the task loops
 * again, 0 after the TRAP #3 exit prefix. */
int Task1783A8_Frame(const f3mem_t *m, f3regs_t *r)
{
    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x40u) == 0u                  /* 0x178424 btst #6 */
        || (int16_t)rd16(m, F3_A5 + 0x4ACEu) >= 0x3000) {        /* 0x17842E */
        wr16(m, F3_A5 + 0x4AD0u, 0xFFFFu);                       /* 0x178494 */
        sub_005880(m);                                           /* 0x17849A */
        Kernel_SoundBank0_0014DA(m);                             /* 0x1784A0 */
        uint32_t sp = r->a[7];
        r->a[7] = sp - 6u;                                       /* 0x1784A6 trap #3: PC+SR */
        (void)Kern_001E48(m, r);
        r->a[7] = sp;
        return 0;                                                /* 0x1784A8 tail not reachable */
    }

    (void)Gfx_DrawHex(m, rd32(m, F3_A5 - 0x5DA8u), 8u, 0x0061C034u, 7u); /* 0x178438 */
    (void)Gfx_DrawHex(m, rd32(m, F3_A5 - 0x5DA4u), 8u, 0x0061C0B4u, 7u); /* 0x178454 */
    (void)Gfx_DrawHex(m, (uint32_t)rd16(m, F3_A5 + 0x4ACEu), 4u, 0x0061C134u, 7u); /* 0x178470 */
    wr16(m, F3_A5 + 0x4ACEu, (uint16_t)(rd16(m, F3_A5 + 0x4ACEu) + 1u)); /* 0x17848E */
    return 1;                                                    /* 0x178492 bra 0x178422 */
}
