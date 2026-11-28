#include "fn_auto.h"
/* ROM 0x0B80A4: (capture fnt_: injected call up to its first yield, F3_INJECT_ABORT): edit the test menu's MIN-MAX setting. */
/* All 30 supplied entry-to-return pairs span VBlank. The setup stops at
 * 0x0B80FC and TaskMinMax_Frame resumes at 0x0B80FE with the saved frame. */
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"
#include "../video/sprite.h"

#define TEXT_BASE 0x0061C000u

static uint32_t pop_long(const f3mem_t *m, f3regs_t *r)
{
    uint32_t value = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return value;
}

/* MOVEM.L of D0 through D(count-1), A0 and A1. */
static void save_text_regs(const f3mem_t *m, f3regs_t *r, unsigned count)
{
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    while (count != 0u)
        push_long(m, r, r->d[--count]);
}

static void restore_text_regs(const f3mem_t *m, f3regs_t *r, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        r->d[i] = pop_long(m, r);
    r->a[0] = pop_long(m, r);
    r->a[1] = pop_long(m, r);
}

/* ROM 0x005666: model LINK and MOVEM around the shared string renderer. */
static void draw_string(const f3mem_t *m, f3regs_t *r, uint32_t src,
                        uint32_t dst, uint16_t attr, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_long(m, r, src);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, src, dst, attr);
    r->a[7] = sp;
}

/* ROM 0x0B8190-0x0B81A5: print D1 as three hex digits via 0x005690. */
static void draw_setting_hex(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    push_word(m, r, 7u);
    push_long(m, r, r->a[1]);
    push_word(m, r, 3u);
    push_long(m, r, r->d[1]);
    push_long(m, r, 0x0B81A2u);
    push_long(m, r, r->a[6]);   /* 0x005690 LINK */
    push_long(m, r, r->a[0]);   /* 0x005694 MOVEM */
    push_long(m, r, r->d[7]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = Gfx_DrawHex(m, r->d[1], 3u, r->a[1], 7u);
    r->a[7] = sp;
}

/* ROM 0x0B73E2: blank the text window at the stack argument {x.w, y.w}. */
static void sub_0B73E2(const f3mem_t *m, f3regs_t *r)
{
    save_text_regs(m, r, 4u);
    uint32_t sp = r->a[7];
    uint16_t width = rd16(m, sp + 0x1Cu);
    uint16_t height = rd16(m, sp + 0x1Eu);
    uint32_t rect = rd32(m, sp + 0x20u);
    uint16_t x = rd16(m, rect);
    uint16_t y = rd16(m, rect + 2u);
    uint32_t row = TEXT_BASE + (int16_t)(uint16_t)(x * 2u)
                             + (int16_t)(uint16_t)(y << 7);

    /* 0x0B7404-0x0B7416: subtract one from each word before its DBF loop. */
    uint16_t rows = (uint16_t)(height - 1u);
    do {
        uint32_t cell = row;
        uint16_t columns = (uint16_t)(width - 1u);
        do {
            wr16(m, cell, 0u);
            cell += 2u;
        } while (columns-- != 0u);
        row += 0x80u;
    } while (rows-- != 0u);

    restore_text_regs(m, r, 4u);  /* 0x0B741A */
    r->a[7] += 4u;                /* 0x0B741E RTS */
}

int Fn_0B80A4(const f3mem_t *m, f3regs_t *r)
{
    save_text_regs(m, r, 2u);                                   /* 0x0B80A4 */
    r->a[0] = rd32(m, r->a[5] - 0x0C18u);
    push_long(m, r, r->a[0]);
    push_word(m, r, 6u);
    push_word(m, r, 4u);
    push_word(m, r, 12u);
    push_long(m, r, 0x0B80BEu);
    uint32_t call_sp = r->a[7];
    save_text_regs(m, r, 6u);                                  /* 0x0B7332 MOVEM */
    r->a[7] = call_sp;                                          /* Fn takes entry SP. */
    Fn_0B7332(m, r);
    r->a[7] = call_sp - 32u;
    restore_text_regs(m, r, 6u);
    r->a[7] += 4u + 10u;                                       /* RTS, then 0x0B80BE */

    uint16_t x = (uint16_t)(rd16(m, r->a[0]) + 1u);               /* 0x0B80C2 */
    r->d[0] = set_word(r->d[0], x);
    uint16_t y = (uint16_t)(rd16(m, r->a[0] + 2u) + 1u);
    r->d[1] = set_word(r->d[1], y);
    push_long(m, r, r->d[1]);                                    /* 0x0B80CC */
    push_long(m, r, r->d[0]);
    uint16_t offset = (uint16_t)(x * 2u + (y << 7));
    r->d[1] = set_word(r->d[1], (uint16_t)(y << 7));
    r->d[0] = TEXT_BASE + (int16_t)offset;                       /* 0x0B80DA EXT.L */
    draw_string(m, r, 0x0B81DAu, r->d[0], 7u, 0x0B80F0u);       /* "MIN-MAX" */
    r->d[0] = pop_long(m, r);
    r->d[1] = pop_long(m, r);
    r->d[1] = set_word(r->d[1], rd16(m, r->a[5] + 0x1D40u));
    return 0;                                                  /* 0x0B80FC TRAP #5 */
}

/* ROM 0x0B80FE: return 1 at the next yield, or 0 after the final RTS. */
int TaskMinMax_Frame(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    push_long(m, r, r->d[0]);
    push_long(m, r, r->a[0]);
    push_word(m, r, 4u);                                        /* step */
    push_word(m, r, 0x0100u);                                   /* upper bound */
    push_word(m, r, 0u);                                        /* lower bound */
    push_word(m, r, (uint16_t)r->d[1]);
    r->d[0] = set_word(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, a5 - 0x5DDCu));
    push_word(m, r, (uint16_t)r->d[0]);                         /* 0x0B811A */
    push_long(m, r, 0x0B8122u);
    Fn_19FFF0(m, r);
    r->a[7] += 10u;
    r->a[0] = pop_long(m, r);
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
    r->d[0] = pop_long(m, r);

    /* 0x0B813A brief extension 0x0200 scales the signed x word by two.
     * The y displacement wraps as a word before the sign-extending ADDA.W. */
    uint16_t x = (uint16_t)(rd16(m, r->a[0]) + 2u);
    r->a[1] = TEXT_BASE + (int32_t)(int16_t)x * 2;
    uint16_t y = (uint16_t)(rd16(m, r->a[0] + 2u) + 2u);
    r->a[1] += (int16_t)(uint16_t)(y << 7);
    r->d[0] = (uint16_t)r->d[1];                                 /* 0x0B8148 */
    if (r->d[0] == 0u) {
        /* 0x0B8156 PEA uses 0x0B8158 + 0x8A. */
        draw_string(m, r, 0x0B81E2u, r->a[1], 5u, 0x0B8162u);   /* "AUTO OFF" */
    } else {
        r->d[0] >>= 3;                                          /* 0x0B816A */
        save_text_regs(m, r, 3u);
        push_word(m, r, 0x0Au);
        push_long(m, r, 4u);
        push_long(m, r, r->d[0]);
        push_long(m, r, r->a[1]);
        push_long(m, r, 0x0B8184u);
        Fn_1A0086(m, r);
        r->a[7] += 14u;
        restore_text_regs(m, r, 3u);
        r->a[1] += 10u;                                         /* 0x0B818C */
        draw_setting_hex(m, r);
    }

    if (rd8(m, a5 - 0x5DDBu) & 0x20u) {                        /* 0x0B81A6 */
        if (rd8(m, a5 - 0x5DDBu) & 0x10u)
            return 1;                                          /* 0x0B81BA -> TRAP #5 */
        wr16(m, a5 + 0x1D40u, (uint16_t)r->d[1]);               /* 0x0B81BE */
    }

    push_long(m, r, r->a[0]);                                   /* 0x0B81C2 */
    push_word(m, r, 4u);
    push_word(m, r, 12u);
    push_long(m, r, 0x0B81D0u);
    sub_0B73E2(m, r);
    r->a[7] += 8u;
    restore_text_regs(m, r, 2u);
    r->a[7] += 4u;                                              /* 0x0B81D8 RTS */
    return 0;
}
