#include "fn_auto.h"
/* ROM 0x0B7506: (capture fnt_: injected call up to its first yield, F3_INJECT_ABORT): draw and run the YES/NO prompt. */
/* All supplied entry-to-RTS pairs span VBlanks and are skipped by test_fn. */
#include "fn_text_frames.h"

static uint32_t pop_long(const f3mem_t *m, f3regs_t *r)
{
    uint32_t value = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return value;
}

/* ROM 0x0B73E2: clear a rectangle at {x.w, y.w} on the text layer. */
static void clear_box_0B73E2(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[1]);                                  /* 0x0B73E2 MOVEM */
    push_long(m, r, r->a[0]);
    for (unsigned reg = 4; reg > 0;)
        push_long(m, r, r->d[--reg]);

    uint32_t sp = r->a[7];
    uint16_t cols = (uint16_t)(rd16(m, sp + 0x1Cu) - 1u);     /* 0x0B73E6 / 0x0B7404 */
    uint16_t rows = (uint16_t)(rd16(m, sp + 0x1Eu) - 1u);     /* 0x0B73EA / 0x0B7406 */
    uint32_t rect = rd32(m, sp + 0x20u);                       /* 0x0B73EE */
    uint16_t x = rd16(m, rect);
    uint16_t y = rd16(m, rect + 2u);
    /* 0x0B73FC..0x0B7402: the two ADDA.W instructions sign-extend separately. */
    uint32_t dst = 0x61C000u + (uint32_t)(int32_t)(int16_t)(uint16_t)(x + x)
                            + (uint32_t)(int32_t)(int16_t)(uint16_t)(y << 7);
    do {                                                       /* 0x0B7408..0x0B7416 */
        uint32_t cell = dst;
        uint16_t remaining = cols;
        do {
            wr16(m, cell, 0u);
            cell += 2u;
        } while (remaining-- != 0u);
        dst += 0x80u;
    } while (rows-- != 0u);

    for (unsigned reg = 0; reg < 4; ++reg)                     /* 0x0B741A MOVEM */
        r->d[reg] = pop_long(m, r);
    r->a[0] = pop_long(m, r);
    r->a[1] = pop_long(m, r);
    r->a[7] += 4u;                                             /* 0x0B741E RTS */
}

/* 0x0B7506..0x0B7522: set up the window and leave its frame live at TRAP #5. */
int Fn_0B7506(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[0]);                                 /* 0x0B7506 MOVEM */
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->a[0]);                                 /* 0x0B750A */
    push_word(m, r, (uint16_t)r->d[1]);                        /* 0x0B750C */
    push_word(m, r, 3u);                                      /* 0x0B750E */
    push_word(m, r, 12u);                                     /* 0x0B7512 */
    push_long(m, r, 0x0B751Au);                               /* 0x0B7516 JSR */

    /* Fn_0B7332 reads arguments at its entry SP. Mirror its MOVEM writes. */
    uint32_t call_sp = r->a[7];
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (unsigned reg = 6; reg > 0;)
        push_long(m, r, r->d[--reg]);
    r->a[7] = call_sp;
    (void)Fn_0B7332(m, r);
    r->a[7] = call_sp + 4u + 10u;                             /* RTS / 0x0B751A LEA */

    r->d[1] = set_word(r->d[1], 1u);                          /* 0x0B751E */
    return 0;                                                 /* 0x0B7522 TRAP #5 */
}

/* 0x0B7560..0x0B7588 / 0x0B7596..0x0B75BE: draw at {D2.w, D3.w}. */
static void draw_at_cursor(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                           uint32_t text, uint16_t attr)
{
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    r->d[3] = set_word(r->d[3], (uint16_t)((uint16_t)r->d[3] << 7));
    uint16_t offset = (uint16_t)((uint16_t)r->d[2] * 2u + (uint16_t)r->d[3]);
    r->d[2] = 0x61C000u + (uint32_t)(int32_t)(int16_t)offset;
    draw_text(m, r, ret, text, r->d[2], attr);
    r->d[2] = pop_long(m, r);
    r->d[3] = pop_long(m, r);
}

/* Resume at 0x0B7524. Return 1 at the next yield and 0 after the RTS. */
int TaskYesNoPrompt_Frame(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[0]);                                 /* 0x0B7524 */
    push_long(m, r, r->a[0]);                                 /* 0x0B7528 */
    push_word(m, r, 1u);                                      /* 0x0B752C step */
    push_word(m, r, 1u);                                      /* 0x0B7530 upper */
    push_word(m, r, 0u);                                      /* 0x0B7534 lower */
    push_word(m, r, (uint16_t)r->d[1]);                        /* 0x0B7538 selection */
    r->d[0] = set_word(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, r->a[5] - 0x5DDCu));
    push_word(m, r, (uint16_t)r->d[0]);                        /* 0x0B7540 */
    push_long(m, r, 0x0B7548u);                               /* 0x0B7542 JSR */
    (void)Fn_19FFF0(m, r);
    r->a[7] += 10u;                                           /* 0x0B7548 */
    r->a[0] = pop_long(m, r);                                 /* 0x0B754C */
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);            /* 0x0B7550 */
    r->d[0] = pop_long(m, r);                                 /* 0x0B7552 */

    r->d[2] = set_word(r->d[2], (uint16_t)(rd16(m, r->a[0]) + 1u));
    r->d[3] = set_word(r->d[3], (uint16_t)(rd16(m, r->a[0] + 2u) + 1u));
    draw_at_cursor(m, r, 0x0B7584u, 0x0B7600u, 7u);            /* 0x0B7556..0x0B7588 */

    r->d[0] = (uint32_t)((int32_t)(int16_t)r->d[1] * 5);      /* 0x0B758C..0x0B7590 */
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] + 1u));    /* 0x0B7592 */
    r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] + r->d[0])); /* 0x0B7594 */
    draw_at_cursor(m, r, 0x0B75BAu, 0x0B7500u, 10u);          /* 0x0B7596..0x0B75BE */

    if ((rd8(m, r->a[5] - 0x5DDBu) & 0x20u) == 0u) {        /* 0x0B75C2 */
        r->d[1] = set_word(r->d[1], 0xFFFFu);                 /* 0x0B75CC cancel */
    } else if (rd8(m, r->a[5] - 0x5DDBu) & 0x10u) {           /* 0x0B75D4 */
        return 1;                                             /* 0x0B75DA -> 0x0B7522 */
    }

    push_long(m, r, r->d[1]);                                 /* 0x0B75DE */
    push_long(m, r, r->a[0]);                                 /* 0x0B75E2 */
    push_word(m, r, 3u);                                      /* 0x0B75E4 */
    push_word(m, r, 12u);                                     /* 0x0B75E8 */
    push_long(m, r, 0x0B75F0u);                               /* 0x0B75EC JSR */
    clear_box_0B73E2(m, r);
    r->a[7] += 8u;                                            /* 0x0B75F0 */
    r->d[1] = pop_long(m, r);                                 /* 0x0B75F4 */
    r->d[0] = set_word(r->d[0], (uint16_t)r->d[1]);            /* 0x0B75F8 */
    r->d[1] = pop_long(m, r);                                 /* 0x0B75FA MOVEM */
    r->d[2] = pop_long(m, r);
    r->d[3] = pop_long(m, r);
    r->a[0] = pop_long(m, r);
    r->a[7] += 4u;                                            /* 0x0B75FE RTS */
    return 0;
}
