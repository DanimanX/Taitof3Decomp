/* ROM 0x15F172: palette set-up for a stage-intro screen. Copies 0x20 longs
 * from ROM 0x25240 to 0x40B3AE, queues an upload of 0x18 words from 0x25240
 * to palette 0xF00 (0x1660), arms four 0x100/0x100/0x100/0x80-frame palette
 * ramps (0x256C0 -> 0x1380, 0x25A40 -> 0x1700, 0x25A80 -> 0x1740,
 * 0x15FABC -> 0x1310) through 0x183C, and copies 0x20 longs from ROM 0x15FA68
 * to 0x40B52E.
 *
 * No object is touched; A0/A1 come back from the movem save/restore pairs as
 * at entry and D0 holds the last 0x183C result (claimed ramp slot, or
 * 0xFFFFFFFF when no slot was free). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"

/* ROM 0x16A19C: copy d0+1 longs from a0 to a1 (d0 is the caller's count-1) */
static void fn15f172_copy(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t d0)
{
    for (uint32_t i = 0u; i <= (uint32_t)d0; i++)
        wr32(m, dst + i * 4u, rd32(m, src + i * 4u));
}

/* 0x00183C wraps 0x001792: returns the claimed ramp slot, 0xFFFFFFFF if none */
static uint32_t fn15f172_ramp(const f3mem_t *m, uint32_t src, uint32_t ramp_src,
                              uint32_t dst, uint32_t frames, uint32_t count)
{
    uint32_t slot = Pal_QueueRamp(m, src, ramp_src, dst, frames, count);
    return slot ? slot : 0xFFFFFFFFu;
}

int Fn_15F172(const f3mem_t *m, f3regs_t *r)
{
    /* 0x15F172..0x15F192: jsr 0x16A19C(0x25240, 0x40B3AE) with D0 = 0x1F */
    fn15f172_copy(m, 0x00025240u, 0x0040B3AEu, 0x1Fu);

    /* 0x15F196..0x15F1AE: jsr 0x1660(0x25240, 0xF00, 0x18) */
    Pal_QueueUpload(m, 0x00025240u, 0x0F00u, 0x18u);

    /* 0x15F1B2..0x15F1D6: jsr 0x183C(0x15F994, 0x256C0, 0x1380, 0x100, 0x1B) */
    (void)fn15f172_ramp(m, 0x0015F994u, 0x000256C0u, 0x1380u, 0x100u, 0x1Bu);
    /* 0x15F1DA..0x15F1FE: jsr 0x183C(0x15FA00, 0x25A40, 0x1700, 0x100, 9) */
    (void)fn15f172_ramp(m, 0x0015FA00u, 0x00025A40u, 0x1700u, 0x100u, 0x09u);
    /* 0x15F202..0x15F226: jsr 0x183C(0x15FA40, 0x25A80, 0x1740, 0x100, 0xA) */
    (void)fn15f172_ramp(m, 0x0015FA40u, 0x00025A80u, 0x1740u, 0x100u, 0x0Au);
    /* 0x15F22A..0x15F24E: jsr 0x183C(0x15FABC, 0x15FA78, 0x1310, 0x80, 0xD) */
    r->d[0] = fn15f172_ramp(m, 0x0015FABCu, 0x0015FA78u, 0x1310u, 0x80u, 0x0Du);

    /* 0x15F252..0x15F272: jsr 0x16A19C(0x15FA68, 0x40B52E) with D0 = 0x1F */
    fn15f172_copy(m, 0x0015FA68u, 0x0040B52Eu, 0x1Fu);

    return 0;                                               /* 0x15F276 rts */
}
