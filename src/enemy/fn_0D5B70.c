#include "fn_0D5B70.h"

/* ROM 0x0D5B70: 0x0D5B70 move.l (0x58,A0),D0 ; asr.l #2 ; sub.l D0,(0x3A,A0);
 * 0x0D5B7A move.l (0x3A,A0),D0 ; subi.l #0x18000,D0 ; asr.l #4 ; sub.l D0,(0x3A,A0);
 * 0x0D5B8A/0x0D5B9A cmp.w on the HIGH words of +0x3A, clamped to 0..3, then
 * 0x0D5BAA move.l (0x0D5964,D0w*4),(0x10,A0). All arithmetic is 32-bit on the
 * long at +0x3A with an arithmetic shift. */
void Obj_0D5B70_Step(const f3mem_t *m, uint32_t obj)
{
    int32_t v = (int32_t)rd32(m, obj + 0x3Au);
    int32_t d0 = (int32_t)rd32(m, obj + 0x58u);
    d0 = d0 >> 2;                                   /* asr.l #2 */
    v = (int32_t)((uint32_t)v - (uint32_t)d0);      /* sub.l d0,(a0)+0x3A */
    d0 = (int32_t)((uint32_t)v - 0x18000u);         /* subi.l #0x18000,d0 */
    d0 = d0 >> 4;                                   /* asr.l #4 */
    v = (int32_t)((uint32_t)v - (uint32_t)d0);
    if ((int16_t)(uint16_t)((uint32_t)v >> 16) < 0)  /* cmpi.w #0,(0x3A,A0) ; bge */
        v = (v & 0x0000FFFF) | 0x00000000;           /* move.w #0,(0x3A,A0) */
    if ((int16_t)(uint16_t)((uint32_t)v >> 16) > 3)  /* cmpi.w #3 ; ble */
        v = (v & 0x0000FFFF) | 0x00030000;           /* move.w #3,(0x3A,A0) */
    wr32(m, obj + 0x3Au, (uint32_t)v);
    uint16_t idx = (uint16_t)((uint32_t)v >> 16);   /* move.w (0x3A,A0),d0 */
    wr32(m, obj + 0x10u, rd32(m, 0x0000D5964u + (uint32_t)idx * 4u));
}
