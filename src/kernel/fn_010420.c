#include "fn_010420.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x001520: block relay. a0 = block table, a1 = source base,
 * idx = block index (the word the caller leaves in d0). The ROM leaves the loop
 * leftovers in the register file: d0/d1 = the last 4-byte copy's (src+4) and
 * (dst+4) as words, a2 = the stream cursor, d3 = the loaded long with dbra's
 * 0xFFFF in its low word, a0 = 0x440000 (the upload buffer) - or, when the
 * block's long at +4 is negative (0x001528 bmi 0x00154C), a0 = the table,
 * a2 = the picked entry and d0/d1 untouched. `rg` carries both directions:
 * rg->d0/rg->d1 on entry are the caller's d0/d1 (only the negative path keeps
 * them), on return it holds everything the ROM leaves. */
void Kernel_SoundRelay(const f3mem_t *m, uint32_t table, uint32_t src, uint8_t idx,
                       snd_relay_t *rg)
{
    uint32_t a2 = rd32(m, table + (uint32_t)idx * 8u);          /* 0x001520 movea.l */
    uint32_t d3l = rd32(m, table + 4u + (uint32_t)idx * 8u);    /* 0x001524 move.l */
    if ((int32_t)d3l < 0) {                                     /* 0x001528 bmi 0x00154C */
        rg->a0 = table;
        rg->a1 = src;
        rg->a2 = a2;
        rg->d3 = d3l;
        return;
    }
    uint16_t d3 = (uint16_t)d3l;
    uint32_t d0 = 0u;               /* long: moveq/addq.l */
    uint16_t d1 = 0u;
    do {
        d3--;
        /* 0x001530: one record = {n.w, src.w, dst.w}; n longs are copied from
         * src (a1 base) to dst (0x440000 base), 4 bytes at a time. */
        uint16_t n = rd16(m, a2);                               /* move.w (a2)+,d2 */
        int16_t d2 = (int16_t)(n - 1u);                         /* 0x001532 subq.w #1,d2 */
        a2 += 2u;
        d0 = (d0 & 0xFFFF0000u) | rd16(m, a2);                 /* 0x001536 move.w (a2)+,d0 */
        d1 = rd16(m, a2 + 2u);                                  /* 0x001538 move.w (a2)+,d1 */
        a2 += 4u;
        for (;;) {
            /* 0x00153A move.l (a1,d0.l),(a0,d1.w) with a0 = 0x440000 */
            wr32(m, 0x440000u + (uint32_t)(int32_t)(int16_t)d1, /* d1.w sign-extended */
                 rd32(m, src + d0));
            d0 += 4u;                                           /* 0x001540 addq.l #4,d0 */
            d1 = (uint16_t)(d1 + 4u);                           /* 0x001542 addq.w #4,d1 */
            if (d2 == 0)                                        /* 0x001544 dbra d2,0x153A */
                break;
            d2--;
        }
    } while (d3 != 0xFFFFu);                                    /* 0x001548 dbra d3,0x1530 */
    rg->d0 = d0;
    rg->d1 = d1;
    rg->a0 = 0x00440000u;                                       /* 0x00152A lea */
    rg->a1 = src;
    rg->a2 = a2;
    rg->d3 = (d3l & 0xFFFF0000u) | 0xFFFFu;
}

/* ROM 0x0014DA + 0x0014E8: sound bank 0 (D0 = 0 -> index 0) */
void Kernel_SoundBank0_0014DA(const f3mem_t *m)
{
    snd_relay_t rg = { 0u, 0u, 0u, 0u, 0u, 0u };
    Kernel_SoundRelay(m, 0x00016890u, 0x00016900u, 0u, &rg);
}

/* ROM 0x001504: sound bank 1 (D0 = 0). 0x00151A swap d0 makes the block index
 * the remainder of D0 mod 8; the table's word entries are 8 bytes apart. */
void Kernel_SoundBank1_001504(const f3mem_t *m)
{
    snd_relay_t rg = { 0u, 0u, 0u, 0u, 0u, 0u };
    Kernel_SoundRelay(m, 0x000168C0u, 0x0001FB40u, 0u, &rg);
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
