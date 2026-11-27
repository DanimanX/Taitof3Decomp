#include "fn_auto.h"
/* ROM 0x145654: verified: the capture discriminates (an empty body fails it): Dispatch the stage's playfield reset. */
#include "../video/sprite.h"

/* ROM 0x0058DA / 0x005918: save D0-D1/A0-A1 before clearing the playfield. */
static void save_clear_frame(const f3mem_t *m, const f3regs_t *r, uint32_t sp)
{
    wr32(m, sp - 4u, r->a[1]);
    wr32(m, sp - 8u, r->a[0]);
    wr32(m, sp - 12u, r->d[1]);
    wr32(m, sp - 16u, r->d[0]);
}

int Fn_145654(const f3mem_t *m, f3regs_t *r)
{
    /* 0x145654 uses 0x145656 as the PC base. Its 28 pointers end
     * at the table referenced by 0x14544E, which starts at 0x1456EE.
     * 0x14565E extension 0x0400 selects signed D0.w scaled by four. */
    uint16_t stage = rd16(m, r->a[5] + 0x32DCu);
    if (stage >= 28u)
        return -1;
    uint32_t handler = rd32(m, 0x14567Eu + 4u * stage);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | stage;
    r->a[0] = handler;

    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, 0x145664u); /* 0x145662 JSR (A0) */
    if (handler != 0x145666u) {
        /* Both 0x145668 and 0x145670 call 0x0058DA. */
        wr32(m, sp - 8u, handler + 6u);
        save_clear_frame(m, r, sp - 8u);
        Gfx_ClearPf0(m);
        if (handler == 0x145670u) {
            wr32(m, sp - 8u, 0x14567Cu); /* 0x145676 JSR 0x005918 */
            save_clear_frame(m, r, sp - 8u);
            Gfx_ClearPf0b(m);
        }
    }
    return 0;
}
