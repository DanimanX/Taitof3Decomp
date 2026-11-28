/* Darius Gaiden (Taito F3) — sprite buffer initialisation.
 * ROM: 0x001070  Sprite_InitBuffers
 *   (tail entry 0x001100 Sprite_InitHeaders uses the D0 built in 0x0010A2..)
 *
 *   A0 = 0x600010  A1 = 0x600020  A2 = 0x600000
 *   move.w #0xFFFF,A2+2    / A2+0x8002       ; 0x600002 (same byte twice)
 *   move.w #0x8000,A2+6    / A2+0x8006       ; 0x600006 (same byte twice)
 *   D0 = screen_flip_flag (A5-0x5D58, 0x4022A8) & 1
 *   move.w #0x5000,A1+4    / A1+0x8004       ; 0x600024 (same byte twice)
 *   D0 == 0 ? A0+4 = 0xA02E, A0+6 = 0x0018
 *           : A0+4 = 0xA092, A0+6 = 0x0000    ; and their +0x8000 twins
 *   ror.w #3,D0                              ; D0 = 0 or 0x2000
 * 0x001100 Sprite_InitHeaders:
 *   ori.w #0x100,D0                          ; 0x0100 / 0x2100
 *   move.w D0,A2+0x0A  (0x60000A)
 *   move.w D0,A2+0x800A (0x60800A)           ; does NOT sign-extend -> +0x8000
 *   move.w D0,A5-0xCA4  (0x40735C)
 *   for a0 in {0x603FF0, 0x60BFF0}: header
 *     +0 =0  +2 =0xFFFF  +4..+A =0  +C =0x83FF (bank 0) / 0x8BFF (bank 1)
 *     +E =0
 *
 * Sprite RAM is 64 KiB at 0x600000..0x60FFFF, so $8000(A2) lands on the twin
 * that wraps back into the same window (0x60800A is 0x60000A, 0x608014 is
 * 0x600014, ...). Captured per-entry (work/ram/fn_001070: breakpoint at the
 * 0x001070 entry, post at its rts, sprite RAM dumped) the bytes are
 * 0x600002/0x600006/0x600014/0x600016/0x600024, headers at 0x603FF0 and
 * 0x60BFF0, and 0x60000A = 0x60000C = 0x0100, with nothing at 0x60800A. The
 * previous capture (work/ram/fn_001100) was taken *at* 0x001100, i.e. after
 * MAME had run those stores, so it could not see them and the model wrongly
 * kept them as plain +0x8000 offsets on a 128 KiB map (0x60000C unwritten).
 */
#include "sprite.h"

#define SPR_WIN_MASK 0x00FFFFu /* 64 KiB sprite RAM window */

static void spr_w16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    wr16(m, F3_SPR_BASE + ((a - F3_SPR_BASE) & SPR_WIN_MASK), v);
}

/* 0x001100: the tail. d0 on entry is the already-ror'ed flip value. */
void Sprite_InitHeaders(const f3mem_t *m, uint16_t d0)
{
    d0 |= 0x0100u;
    spr_w16(m, 0x60000Au, d0);
    spr_w16(m, 0x60800Au, d0); /* wraps onto 0x60000A */
    wr16(m, SPR_CURSOR - 4u, d0); /* 0x40735C: 0x001110 move.w d0,-$ca4(a5) */

    for (int bank = 0; bank < 2; bank++) {
        uint32_t a0 = 0x603FF0u + 0x8000u * (uint32_t)bank;
        spr_w16(m, a0 + 0x0u, 0x0000u);
        spr_w16(m, a0 + 0x2u, 0xFFFFu);
        spr_w16(m, a0 + 0x4u, 0x0000u);
        spr_w16(m, a0 + 0x6u, 0x0000u);
        spr_w16(m, a0 + 0x8u, 0x0000u);
        spr_w16(m, a0 + 0xAu, 0x0000u);
        spr_w16(m, a0 + 0xCu, bank ? 0x8BFFu : 0x83FFu);
        spr_w16(m, a0 + 0xEu, 0x0000u);
    }
}

/* 0x001070: A0/A1/A2 setup, the two buffer headers, D0, then 0x001100. */
void Sprite_InitBuffers(const f3mem_t *m)
{
    spr_w16(m, 0x600002u, 0xFFFFu);
    spr_w16(m, 0x608002u, 0xFFFFu);
    spr_w16(m, 0x600006u, 0x8000u);
    spr_w16(m, 0x608006u, 0x8000u);

    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 - 0x5D58u) & 1u);

    spr_w16(m, 0x600024u, 0x5000u);
    spr_w16(m, 0x608024u, 0x5000u);

    if (!d0) {
        spr_w16(m, 0x600014u, 0xA02Eu);
        spr_w16(m, 0x608014u, 0xA02Eu);
        spr_w16(m, 0x600016u, 0x0018u);
        spr_w16(m, 0x608016u, 0x0018u);
    } else {
        spr_w16(m, 0x600014u, 0xA092u);
        spr_w16(m, 0x608014u, 0xA092u);
        spr_w16(m, 0x600016u, 0x0000u);
        spr_w16(m, 0x608016u, 0x0000u);
    }

    d0 = (uint16_t)((d0 << 13) | (d0 >> 3)); /* ror.w #3 */
    Sprite_InitHeaders(m, d0);
}
