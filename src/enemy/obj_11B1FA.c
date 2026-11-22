#include "obj_11B1FA.h"

/* sine table: (0x80,A1,D0w*2) = sin, (0x0,A1,D0w*2) = cos, A1 = 0x27E40 */
#define TRIG_TBL 0x00027E40u

/* ROM 0x11B1F2: timer still positive: stay on this entry */
static void sub_11B1F2(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011B1FAu);
}

/* ROM 0x11B1FA: drift by (sin,cos)(+0x20) * speed(+0x6) and scroll the global
 * (+0x30D2/+0x30D6) by the same deltas; when the timer runs out reload it to
 * 0x40 and jump to 0x11B23A */
int Obj_11B1FA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t d0w = (int16_t)rd16(m, obj + 0x20u);          /* move.w (0x20,A0),D0w */
    int16_t d1w = (int16_t)rd16(m, obj + 0x06u);          /* move.w (0x6,A0),D1w */
    uint32_t a1 = TRIG_TBL;
    uint16_t idx = (uint16_t)(d0w * 2);                   /* D0w*0x2 (scale, no ext) */

    /* 0x11B20A: muls.w (0x80,A1,D0w*2),D1 ; 0x11B210/14: add.l +0x14, A5+0x30D2 */
    int32_t dx = (int32_t)(int16_t)rd16(m, a1 + 0x80u + (uint32_t)(int32_t)(int16_t)idx) * d1w;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + (uint32_t)dx);

    /* 0x11B218: muls.w (0x0,A1,D0w*2),D1 (D1 keeps the speed) ; +0x18, A5+0x30D6 */
    int32_t dy = (int32_t)(int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)idx) * d1w;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + (uint32_t)dy);

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_11B1F2(m, obj);
        return 0;
    }
    /* 0x11B22C: move.w #0x40,(0x6,A0) then fall into LAB_0011b232 */
    wr16(m, obj + 0x06u, 0x0040u);
    wr32(m, obj + 0x00u, 0x0011B23Au);
    return 0;
}
