#ifndef TAITOF3_KERNEL_FN_010420_H
#define TAITOF3_KERNEL_FN_010420_H
#include "../common/f3mem.h"

#ifndef F3REGS_T_DEFINED
#define F3REGS_T_DEFINED
typedef struct { uint32_t d[8], a[8]; } f3regs_t;   /* a[7] = SP */
#endif
/* ROM 0x010420: coin-insert / start reset. Returns to the caller at 0x010478
 * when the two A5-0x5DE3 test-mode bits are set; otherwise clears three status
 * bytes, re-initialises sprites / layers / line RAM, reloads sound banks 0 and
 * 1 into 0x440000 and restarts the sound driver. */
void Kernel_CoinStart010420(const f3mem_t *m);

/* ROM 0x0014DA: load sound bank 0; ROM 0x0014E0 path: sound bank 1.
 * ROM 0x0014E8 / 0x001504 + 0x001520: index = D0.b mod 6 (bank 0) or mod 8
 * (bank 1) selects block table 0x16890 / 0x168C0; each block is
 * {count.w, {src_off.w, dst_off.w}*count} and its words are copied through
 * to the upload buffer at 0x440000. A negative block count stops the loop. */
void Kernel_SoundBank0_0014DA(const f3mem_t *m);
void Kernel_SoundBank1_001504(const f3mem_t *m);
void Kernel_SoundBank0_0014DA_Regs(const f3mem_t *m, f3regs_t *r); /* 0x0014DA + registers */

/* The registers the 0x001520 relay leaves: d0/d1 are the last copy's
 * (src+4)/(dst+4) words (or the caller's own d0/d1 when the block is negative),
 * a2 the stream cursor, a0 the upload buffer or the table, d3 the block long
 * with dbra's 0xFFFF. */
typedef struct { uint32_t d0, d1, a0, a1, a2, d3; } snd_relay_t;
void Kernel_SoundRelay(const f3mem_t *m, uint32_t table, uint32_t src, uint8_t idx,
                       snd_relay_t *rg);
#endif
