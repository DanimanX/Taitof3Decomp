/* ROM 0x15F456: init of one background playfield pair for the stage. Sets the
 * line-RAM priority nibble to 5 on the 0xE8 rows of the line table at
 * 0x62B200 + A5-0x5D56 (0x17668A), flags layer A5+0x4285 = 2, copies the
 * scroll/pointer pair A5-0x5D8C into the pf windows at A5+0x420C and
 * A5-0x5DA0, copies A5+0x423C/0x4240 into A5+0x422C/0x4230, and decompresses
 * six tilemap blocks into pf RAM 0x611000/0x611400 at +0x20 steps. */
#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x0017668A: for rows D7.w = 0..D6.w (D6 = 0xE7) set bits 3..0 of the
 * word at A0 + D7.w*2 to D0.w (bfins D0,D1{0x1c:0x4}), leaving bits 15..4. */
static void f15f456_lineprio_17668A(const f3mem_t *m, uint32_t a0, uint16_t d0w)
{
    uint16_t d7 = 0x0000u;

    for (;;) {                                                      /* 0x17668A */
        uint32_t p = a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u;
        uint16_t v = rd16(m, p);
        wr16(m, p, (uint16_t)((v & 0xFFF0u) | (d0w & 0x000Fu)));     /* 0x17668E bfins */
        d7++;                                                       /* 0x176696 addq.w */
        if ((int16_t)d7 > (int16_t)0x00E7)                          /* 0x176698..0x17669A */
            break;
    }
}

int Fn_15F456(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = 0x0062B200u
                + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x15F45A..0x15F460 */

    /* 0x15F464..0x15F474: push 0, push 0xE7, D0.w = 5, D6.w = 0xE7, D7.w = 0 */
    f15f456_lineprio_17668A(m, a0, 5u);

    wr8(m, F3_A5 + 0x4285u, 2u);                                    /* 0x15F47E..0x15F486 */

    /* 0x15F48C..0x15F4A2: D0 = (A5-0x5D8C), D1 = 0; A1 = A5-0x5DA0 */
    {
        /* the pushed 0 is popped into D0 and A5-0x5D8C into D1 (pop order) */
        uint32_t d1 = rd32(m, F3_A5 - 0x5D8Cu);
        wr32(m, F3_A5 + 0x420Cu, 0u);                               /* 0x16AB2C (A0)+ = D0 */
        wr32(m, F3_A5 + 0x4210u, d1);                               /* (A0) = D1 */
        wr32(m, F3_A5 - 0x5DA0u, 0u);                               /* (A1)+ = D0 */
        wr32(m, F3_A5 - 0x5D9Cu, d1);                               /* (A1) = D1 */
    }

    /* 0x15F4A8..0x15F4B8: D0 = (A5+0x423C), D1 = (A5+0x4240) */
    {
        uint32_t d0 = rd32(m, F3_A5 + 0x423Cu);
        uint32_t d1 = rd32(m, F3_A5 + 0x4240u);
        wr32(m, F3_A5 + 0x422Cu, d0);                               /* 0x16AB26 (A0)+ */
        wr32(m, F3_A5 + 0x4230u, d1);                               /* (A0) */
        r->d[0] = d0;
        r->d[1] = d1;
    }
    r->a[0] = F3_A5 + 0x4230u;                                      /* 0x15F4B8 post-increment */
    r->a[1] = F3_A5 - 0x5D9Cu;                                      /* 0x15F4A2 post-increment */

    /* 0x15F4BE..0x15F556: Gfx_DrawCompressedTilemap(src, dst, 0) x6 */
    Gfx_DrawCompressedTilemap(m, 0x0008ABA6u, 0x00611000u, 0u);     /* -> 0x611000 */
    Gfx_DrawCompressedTilemap(m, 0x0008AC2Cu, 0x00611400u, 0u);     /* -> 0x611400 */
    Gfx_DrawCompressedTilemap(m, 0x0008AC7Cu, 0x00611020u, 0u);     /* -> 0x611020 */
    Gfx_DrawCompressedTilemap(m, 0x0008AD3Eu, 0x00611420u, 0u);     /* -> 0x611420 */
    Gfx_DrawCompressedTilemap(m, 0x0008ADA0u, 0x00611040u, 0u);     /* -> 0x611040 */
    Gfx_DrawCompressedTilemap(m, 0x0008AE12u, 0x00611440u, 0u);     /* -> 0x611440 */
    return 0;                                                       /* 0x15F55A rts */
}
