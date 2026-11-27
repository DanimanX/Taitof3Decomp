#include "fn_auto.h"
/* ROM 0x19F684: Draw the ON or OFF label for a masked test-menu setting. */
#include "../kernel/fn_010352.h"

int Fn_19F684(const f3mem_t *m, f3regs_t *r)
{
    uint32_t frame = r->a[7] - 4u;
    wr32(m, frame, r->a[6]); /* 0x19F684 LINK.W */
    r->a[0] = rd32(m, frame + 8u);
    r->d[1] = (uint32_t)(int32_t)(int16_t)rd16(m, frame + 0x10u);
    r->d[0] = rd8(m, r->a[0]) & r->d[1];

    /* 0x19F698 chooses the ROM string and text colour. */
    uint16_t colour = r->d[0] ? 4u : 5u;
    uint32_t src = r->d[0] ? 0x1A1F1Cu : 0x1A1F18u;
    wr16(m, frame - 2u, colour);
    uint32_t dst = rd32(m, frame + 0xCu);
    wr32(m, frame - 6u, dst);
    wr32(m, frame - 10u, src);
    wr32(m, frame - 14u, 0x19F6BEu);

    /* 0x005666 LINK.W and MOVEM preserve A6, A1, A0 and D7. */
    wr32(m, frame - 18u, frame);
    wr32(m, frame - 22u, r->a[1]);
    wr32(m, frame - 26u, r->a[0]);
    wr32(m, frame - 30u, r->d[7]);
    r->d[0] = Kernel_005666(m, src, dst, colour);
    r->d[7] = rd32(m, frame - 30u);
    r->a[0] = rd32(m, frame - 26u);
    r->a[1] = rd32(m, frame - 22u);
    r->a[6] = rd32(m, frame);
    r->a[7] = frame + 8u;
    return 0;
}
