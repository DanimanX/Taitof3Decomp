#include "obj_BD630.h"
#include "obj_BD47E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0BD630: one-shot screen flash / fade ramp. Seeds two work-RAM records
 * (A5+0x421C via 0x16AB2C with D0=0x80000,D1=0; A5+0x423C via 0x16AB26 with
 * 0,0), rebuilds the zoom and scroll ramps of two line-RAM areas
 * (0x176832 / 0x1768CA at 0x62A600/0x628600 and 0x624600, both offset by
 * A5-0x5D56), then re-arms at 0xBD700.
 *
 * Translation is incomplete: the ramps are not emitted because their targets
 * are not image-backed here -- the callee rebases to 0x6226AA/0x6228AA and
 * 0x621CAA, which f3_ptr() rejects (outside WRAM/ROM/dpram/spr/gfx), so the
 * tick stops with -1. The two record stores of 0xBD63C/0xBD652 are the only
 * image-backed side effects and are reproduced; the -1 is returned before
 * re-arming at 0xBD6F8. */
int Obj_BD630_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0xBD63C: move.l #0,-(SP) ; move.l #0x80000,-(SP) ; D1=(SP)+ ; D0=(SP)+ ;
     * jsr 0x16AB2C -> write 0x80000,0 to A0=A5+0x421C and A1=A5-0x5D90 */
    Mem_WritePair64(m, F3_A5 + 0x421Cu, F3_A5 - 0x5D90u, 0u, 0x00080000u);  /* D1 pops the last push */

    /* 0xBD652: A0=A5+0x423C ; push 0,0 ; D1,D0 = 0 ; jsr 0x16AB26 */
    Obj_16AB26(m, F3_A5 + 0x423Cu, 0u, 0u);

    /* 0xBD66C: zoom ramp: A0=0x62A600, A1=0x628600, D7=0, D6=0xE7,
     * A5+0x4298=0, D5=0, D3=0, A5+0x42A0=3 */
    sub_176832(m, 0x0062A600u, 0x00628600u, 0x0000u, 0x00E7u, 0u, 0u, 0u, 0x0003u);
    /* 0xBD6A0 / 0xBD6CA: scroll ramps 0x74..0 and 0x74..0xE7, all-zero rates */
    sub_1768CA(m, 0x00624600u, 0x0074u, 0x0000u, 0u, 0u, 0u);
    sub_1768CA(m, 0x00624600u, 0x0074u, 0x00E7u, 0u, 0u, 0u);

    /* 0xBD6F4: movem.l (SP)+,{A0} ; move.l #0xBD700,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x000BD700u);
    return 0;
}
