#include "fn_auto.h"
/* ROM 0x19F6C6: Draw the HI/LOW label of a masked test-menu setting
 * (strings 0x1A1F20 "HI " / 0x1A1F24 "LOW", text colours 8 / 0x0A). */
#include "../kernel/fn_010352.h"

int Fn_19F6C6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t frame = r->a[7] - 4u;
    wr32(m, frame, r->a[6]); /* 0x19F6C6 LINK.W A6,#0 */
    r->a[0] = rd32(m, frame + 8u); /* 0x19F6CA movea.l (8,A6),A0 */
    r->d[1] = (uint32_t)(int32_t)(int16_t)rd16(m, frame + 0x10u); /* 0x19F6D2 */
    /* 0x19F6CE moveq #0,D0 ; 0x19F6D0 move.b (A0),D0 ; 0x19F6D8 and.l D1,D0 */
    r->d[0] = rd8(m, r->a[0]) & r->d[1];

    /* 0x19F6DA beq 0x19F6EC picks the ROM string and text colour. */
    uint16_t colour = r->d[0] ? 8u : 0x0Au;
    uint32_t src = r->d[0] ? 0x1A1F20u : 0x1A1F24u;
    wr16(m, frame - 2u, colour);             /* 0x19F6DC / 0x19F6EC */
    uint32_t dst = rd32(m, frame + 0xCu);    /* 0x19F6E0 / 0x19F6F0 */
    wr32(m, frame - 6u, dst);
    wr32(m, frame - 10u, src);               /* 0x19F6E4 / 0x19F6F4 */
    wr32(m, frame - 14u, 0x19F700u);         /* jsr 0x005666 return address */

    /* 0x005666 LINK.W and MOVEM preserve A6, A1, A0 and D7. */
    wr32(m, frame - 18u, frame);
    wr32(m, frame - 22u, r->a[1]);
    wr32(m, frame - 26u, r->a[0]);
    wr32(m, frame - 30u, r->d[7]);
    r->d[0] = Kernel_005666(m, src, dst, colour); /* 0x19F6FA jsr 0x005666 */
    r->d[7] = rd32(m, frame - 30u);
    r->a[0] = rd32(m, frame - 26u);
    r->a[1] = rd32(m, frame - 22u);
    r->a[6] = rd32(m, frame); /* 0x19F704 unlk A6 */
    r->a[7] = frame + 8u;
    return 0;
}
