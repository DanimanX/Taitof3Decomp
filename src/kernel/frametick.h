#ifndef TAITOF3_KERNEL_FRAMETICK_H
#define TAITOF3_KERNEL_FRAMETICK_H
#include "../common/f3mem.h"

/* The five jsr targets of ROM 0x001A0E Kernel_FrameTick, in call order. An
 * entry may be NULL: that routine is not translated yet and the tick writes
 * nothing for it. Each one is verified on its own capture (work/ram/fn_*) and
 * the tick can only be bit-exact against fn_001A0E when every entry the frame
 * exercises is non-NULL (see tests/test_kernel.c). */
typedef struct {
    void (*upload_scroll)(const f3mem_t *m);         /* 0x001C90 Video_UploadScroll */
    void (*poll_inputs)(const f3mem_t *m);           /* 0x001A78 IO_PollInputs     */
    void (*coin_task)(const f3mem_t *m);             /* 0x000DFA, not translated   */
    void (*pad_task)(const f3mem_t *m, uint16_t d0); /* 0x00161E, not translated */
    int  (*push_sound)(const f3mem_t *m);            /* 0x000BF6 Sound_PushCmd      */
} frametick_hooks_t;

/* ROM 0x001A0E: jsr chain, then d0 = vblank_frame_cnt (0x4022AC), ++.
 * Returns d0. Saves/restores D0..A6 in ROM, so RAM is the only visible state. */
uint16_t Kernel_FrameTick(const f3mem_t *m, const frametick_hooks_t *h);

#endif
