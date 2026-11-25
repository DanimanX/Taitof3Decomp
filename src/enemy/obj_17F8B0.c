#include "obj_17F8B0.h"
#include "../enemy/obj_179550.h"
#include "../enemy/obj_179588.h"

/* ROM 0x184888: seed the object's stats from the record at A1. The record is
 * {1C.w, 1E.w, 24.w, 28.w, 2C.l, 30.l, 62.w, 66.l}; the first stat word is
 * divided by ten (divul.l) before it is stored back. */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x17F8B0: the phase-2 start. Takes the parent's velocity/frame, seeds the
 * stats from 0x17F9B6, starts animation 0 (record 0x17F9CC, {dur.w, spr.l})
 * and hands the object to 0x17F8FC with a 0x3C-frame budget. */
int Obj_17F8B0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);

    wr16(m, obj + 0x16u, rd16(m, obj + 0x0Cu));     /* 0x17F8B4 */
    wr16(m, obj + 0x1Au, rd16(m, a4 + 0x1Au));      /* 0x17F8BA */
    wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));      /* 0x17F8C0 */

    sub_184888(m, obj, 0x0017F9B6u);                /* 0x17F8C6 */

    {
        uint32_t a1 = 0x0017F9CCu;                  /* movea.l #0x17F9CC,A1 */
        uint16_t d0w = rd16(m, a1); a1 += 2u;       /* move.w (A1)+,D0w */
        wr16(m, obj + 0x40u, d0w);
        wr32(m, obj + 0x3Au, a1);
        /* 0x17F8EC: jsr ((0x0,0x179A5C,D0w*8)) -- the index's INIT half */
        if (rd32(m, 0x00179A5Cu + (uint32_t)d0w * 8u) == 0x00179550u) {
            Obj_179550_Tick(m, obj);                /* index 0 loader */
        } else if (rd32(m, 0x00179A5Cu + (uint32_t)d0w * 8u) == 0x00179588u) {
            Obj_179588_Tick(m, obj);                /* index 11 loader */
        } else {
            return -1;                              /* loader not translated */
        }
    }

    wr16(m, obj + 0x64u, 0x003Cu);                  /* 0x17F8EE */
    wr32(m, obj + 0x00u, 0x0017F8FCu);              /* 0x17F8F4 */
    return 0;
}
