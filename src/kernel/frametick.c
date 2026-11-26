/* Darius Gaiden (Taito F3) — per-frame system tick.
 * ROM: 0x001A0E  Kernel_FrameTick
 *
 *   movem.l d0-d7/a0-a6,-(a7)
 *   jsr 0x001C90   ; upload scroll regs, video shadows
 *   jsr 0x001A78   ; poll inputs + edge detect
 *   jsr 0x000DFA   ; per-frame task0/coin service (0x000E00 entry point family)
 *   jsr 0x00161E   ; per-frame handler (not translated)
 *   jsr 0x000BF6   ; sound queue -> MMIO
 *   d0 = vblank_frame_cnt (A5-0x5D54, 0x4022AC); ++A5-0x5D54
 *   movem.l (a7)+,d0-d7/a0-a6
 *
 * Called from FUN_0000222e (VBlank preemption) either on the idle stack or on
 * kernel_tmp_sp, so the only durable effects are in Work RAM and MMIO. The
 * capture pair for it also carries the calling scheduler's work (it is the
 * whole frame), which is why the test compares its inputs and this chain
 * separately rather than claiming a whole-frame match.
 */
#include "frametick.h"

uint16_t Kernel_FrameTick(const f3mem_t *m, const frametick_hooks_t *h)
{
    uint16_t d0 = 0;
    if (h->upload_scroll) h->upload_scroll(m);
    if (h->poll_inputs) h->poll_inputs(m);
    if (h->coin_task) h->coin_task(m);
    if (h->pad_task) h->pad_task(m, d0);
    if (h->push_sound) (void)h->push_sound(m);
    d0 = rd16(m, F3_A5 - 0x5D54u);
    wr16(m, F3_A5 - 0x5D54u, (uint16_t)(d0 + 1u));
    return d0;
}
