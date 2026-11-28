/* ROM 0x17C662: the dithering fade entry of the 0x17C0xx family, register
 * level for its capture (work/ram/fnk_17C662).
 *
 *   17C662  bfextu (0x1F,A0){0:8},D0 ; cmpi.w #0xF0,D0 ; bge 0x17C78E
 *   17C670  bfextu (0x1E,A0){0:8},D0 ; cmpi.w #0xF0,D0 ; bge 0x17C78E
 *   17C67E  jsr 0x178982 (script) ; 0x17C684/0x17C68A position = +0x50/+0x54
 *   17C690  +0x1F += 0x10, +0x14 += (fade * size2*16) >> 9
 *   17C6B0  +0x1E += 0x10, +0x18 += (fade * size1*16) >> 9
 *   17C6CE  jsr (0x179A5C)[+0x40] (frame tick), centre on the frame size
 *   17C6F2  jsr 0x001282 (draw) ; move.l #0x17C700,(A0) ; rts
 *   17C78E  move.l A0,-(A7) ; jsr 0x0105EC (free) ; rts
 *
 * The body is Obj_17C700_Tick (src/enemy/obj_17C700.c), which is also the
 * 0x17C700 entry's translation; the registers of the 0x001282 draw come from
 * batch_sprite_outputs (r->d[0] = (x & 0xFFF) << 16 | attr, r->d[1] = flip
 * low byte, r->a[1] = the sprite cursor at A5-0xCA0, r->a[2] = the frame
 * record), which is what the capture's post state shows (D0 = 0x00080000,
 * D1 = 0, A1 = 0x608320, A2 = 0x41E300). On the free path the last D0 write
 * is the 0x17C662/0x17C670 bfextu, whose word Obj_Free (0x0105EC) preserves. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../enemy/obj_17C700.h"

int Fn_17C662(const f3mem_t *m, f3regs_t *r)
{
    uint8_t f1f = rd8(m, r->a[0] + 0x1Fu);
    uint8_t f1e = rd8(m, r->a[0] + 0x1Eu);

    int res = Obj_17C700_Tick(m, r->a[0]);
    if (res < 0)
        return -1;                                  /* undecompiled script/frame path */
    if (res > 0) {
        batch_sprite_outputs(m, r);                 /* 0x17C6F2 jsr 0x001282 */
    } else {
        uint16_t d0w = (uint16_t)(f1f >= 0xF0u ? f1f : f1e);  /* bfextu */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    }
    return 0;                                       /* 0x17C6FC / 0x17C79A rts */
}
