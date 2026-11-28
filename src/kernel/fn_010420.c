#include "fn_010420.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x001520: block relay. a0 = block table, a1 = source base,
 * d3 = index (low byte used), d2 is loaded per block from the block header. */
static void sound_blocks_001520(const f3mem_t *m, uint32_t a0, uint32_t a1, uint8_t idx)
{
    uint32_t a2 = rd32(m, a0 + (uint32_t)idx * 8u);         /* movea.l (a0,d0.w*8),a2 */
    int32_t d3 = (int32_t)rd32(m, a0 + 4u + (uint32_t)idx * 8u);
    if (d3 < 0)                                             /* bmi 0x00154C */
        return;
    do {
        d3--;
        int16_t d2 = (int16_t)(rd16(m, a2) - 1u);           /* move.w (a2)+,d2 */
        a2 += 2u;
        uint16_t d0 = 0u;
        do {
            d2--;
            d0 = rd16(m, a2);                               /* move.w (a2)+,d0 */
            uint16_t d1 = rd16(m, a2 + 2u);                 /* move.w (a2)+,d1 */
            a2 += 4u;
            /* move.l (a1,d0.l),(a0,d1.w) with a0 = 0x440000 */
            wr32(m, 0x440000u + (uint32_t)(int32_t)(int16_t)d1, rd32(m, a1 + d0));
            d0 = (uint16_t)(d0 + 4u);
            d1 = (uint16_t)(d1 + 4u);
        } while (d2 >= 0);                                  /* dbra d2 */
    } while (d3 >= 0);                                      /* dbra d3 */
}

/* ROM 0x0014DA + 0x0014E8: sound bank 0 (D0 = 0 -> index 0) */
void Kernel_SoundBank0_0014DA(const f3mem_t *m)
{
    sound_blocks_001520(m, 0x00016890u, 0x00016900u, 0u);
}

/* ROM 0x001504: sound bank 1 (D0 = 0). 0x00151A swap d0 makes the block index
 * the remainder of D0 mod 8; the table's word entries are 8 bytes apart. */
void Kernel_SoundBank1_001504(const f3mem_t *m)
{
    sound_blocks_001520(m, 0x000168C0u, 0x0001FB40u, 0u);
}

/* ROM 0x010420: start / coin reset path.
 * 0x010420-0x010430: test-mode bit4 and bit3 of A5-0x5DE3 both set -> straight
 * to the rts at 0x010478. Otherwise (0x010432) trap #2 (supervisor, no RAM
 * effect), then 0x010452-0x01046E: three sf.b clears, Sprite_ClearAll
 * (0x00117E), Gfx_ClearAllLayers (0x005A12), Gfx_InitLineRam (0x005B0C),
 * the bank-0/1 sound reset 0x0014DA, Sound_StopAll (0x003C24), trap #5 and
 * bra 0x010474 (stops at its own rts). */
void Kernel_CoinStart010420(const f3mem_t *m)
{
    uint8_t flags = rd8(m, F3_A5 - 0x5DE3u);
    if ((flags & 0x10u) && (flags & 0x08u))     /* btst #4 ; btst #3 ; bra 0x10478 */
        return;
    wr8(m, F3_A5 - 0x5DC7u, 0u);                /* sf.b A5-0x5DC7 */
    wr8(m, F3_A5 - 0x5DCCu, 0u);                /* sf.b A5-0x5DCC */
    wr8(m, F3_A5 - 0x1444u, 0u);                /* sf.b A5-0x1444 */
    Sprite_ClearAll(m);                          /* 0x00117E */
    Gfx_ClearAllLayers(m);                       /* 0x005A12 */
    Gfx_InitLineRam(m);                          /* 0x005B0C */
    Kernel_SoundBank0_0014DA(m);                 /* 0x0014DA */
    Kernel_SoundBank1_001504(m);                 /* 0x0014E0 */
    Sound_StopAll(m);                            /* 0x003C24 */
}
